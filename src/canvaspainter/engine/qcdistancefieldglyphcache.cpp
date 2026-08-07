// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "engine/qctextlayout_p.h"
#include "qcdistancefieldglyphcache_p.h"
#include "qcanvaspainter_p.h"
#include <private/qrawfont_p.h>
#include <cmath>

QT_BEGIN_NAMESPACE

QCDistanceFieldGlyphCache::QCDistanceFieldGlyphCache(QRhi *rhi /*, QRawFont& font*/)
    : m_rhi(rhi)
{
    m_layout.setCacheEnabled(true);
}

QCDistanceFieldGlyphCache::~QCDistanceFieldGlyphCache()
{
    for (auto &cache : std::as_const(m_glyphCaches))
        delete cache;
    delete m_colorCache;
}

QList<QGlyphRun> QCDistanceFieldGlyphCache::generateGlyphRuns(
    const QString &text, const QRectF &rect, const QFont &font,
    const QFontMetricsF &metrics, QCState *state, QCanvasPainter::TextAlign alignment)
{
    QTextOption option;
    // Emoji parsing is left enabled so that color-emoji sequences are grouped
    // into their own QGlyphRun(s) backed by a color (Format_ARGB) font engine.
    // generate() detects those runs and routes them to the color glyph atlas
    // instead of the SDF cache, so they render with their native colors.
    option.setWrapMode(QCTextLayout::convertToQtWrapMode(state->textWrapMode));
    option.setAlignment(QCTextLayout::convertToQtAlignment(alignment));
    option.setTextDirection(QCTextLayout::convertToQtDirection(state->textDirection));
    m_layout.clearLayout();
    m_layout.setTextOption(option);
    m_layout.setFont(font);
    m_layout.setText(text);
    const int leading = metrics.leading();
    float layoutHeight = 0;
    m_layout.beginLayout();
    while (true) {
        QTextLine line = m_layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(rect.width());
        layoutHeight += leading;
        line.setPosition(QPointF(0, layoutHeight));
        layoutHeight += line.height() + state->textLineHeight;
    }
    m_layout.endLayout();
    return m_layout.glyphRuns();
}

QCDistanceFieldGlyphCache::FontKeyData *QCDistanceFieldGlyphCache::fontKeyDataFor(const QRawFont &rFont)
{
    QFontEngine *fe = QRawFontPrivate::get(rFont)->fontEngine;
    if (fe == m_lastFontEngine && m_lastFontKeyData)
        return m_lastFontKeyData;

    FontKey key(rFont);
    auto it = m_glyphCaches.find(key);
    if (it == m_glyphCaches.end())
        it = m_glyphCaches.insert(key, new FontKeyData{ nullptr, QCRhiDistanceFieldGlyphCache(m_rhi)});

    m_lastFontEngine = fe;
    m_lastFontKeyData = it.value();
    return m_lastFontKeyData;
}

QCDistanceFieldGlyphCache::FontKeyData *QCDistanceFieldGlyphCache::generate(const QString &text, const QRectF &rect, const QFont &font, QCState *state, QCanvasPainter::TextAlign alignment,
                                         float devicePixelRatio,
                                         QCRhiDistanceFieldGlyphCache::VertexList *verts, QCRhiDistanceFieldGlyphCache::IndexList *indices)
{
    // Remove raw fonts
    auto rFont = QRawFont::fromFont(font);
    const auto metrics = QFontMetricsF(font);

    // Get glyph cache
    FontKeyData *data = fontKeyDataFor(rFont);
    QCRhiDistanceFieldGlyphCache *cache = &data->nativeGlyphCache;

    QList<QGlyphRun> glyphRuns;
    float textY = 0;
#ifdef QCPAINTER_CACHE_GLYPH_RUNS
    const GlyphCacheKey hashKey {
        FontKey(rFont),
        text,
        state->textAlignment,
        state->textWrapMode,
        state->textDirection,
        float(rFont.pixelSize()),
        float(rect.width()),
        float(state->textLineHeight),
        float(font.letterSpacing()),
        float(font.wordSpacing())
    };

    if (m_glyphRunCache.contains(hashKey)) {
        // Matching QGlyphRuns exists
        auto &c = m_glyphRunCache[hashKey];
        glyphRuns = c.runs;
        textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, c.rect);
        c.usageCounter++;
    } else {
        // Not in cache yet, generate
        glyphRuns = generateGlyphRuns(text, rect,
                                      font, metrics,
                                      state, alignment);
        textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, m_layout.boundingRect());
        if (m_layout.cacheEnabled())
            m_glyphRunCache.insert(hashKey, { glyphRuns, m_layout.boundingRect(), 0 });
    }
#else
    // QGlyphRuns caching disabled, generate
    glyphRuns = generateGlyphRuns(text, rect,
                                  font, metrics,
                                  state, alignment);
    textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, m_layout.boundingRect());
#endif

    // use clear(), so that the containers' allocations are potentially kept
    verts->clear();
    indices->clear();
    m_colorVertices.clear();
    m_colorIndices.clear();

    const QPointF glyphPos(rect.x(), rect.y() + textY);
    for (const auto &run : std::as_const(glyphRuns)) {
        // Color (emoji) runs cannot be represented in the single-channel SDF
        // atlas; rasterize their RGBA bitmaps into a separate color atlas and
        // collect their quads for a dedicated draw pass.
        QFontEngine *fe = QRawFontPrivate::get(run.rawFont())->fontEngine;
        if (fe && fe->isColorFont()) {
            if (!m_colorCache)
                m_colorCache = new QCRhiColorGlyphCache(m_rhi);
            QColor glyphColor = Qt::black;
            if (state->fill.brushType == BrushColor) {
                glyphColor = QColor::fromRgbF(state->fill.innerColor.r,
                                              state->fill.innerColor.g,
                                              state->fill.innerColor.b);
            }
            m_colorCache->addGlyphRun(glyphPos, run, glyphColor, state->transform,
                                      devicePixelRatio, &m_colorVertices, &m_colorIndices);
            continue;
        }

        cache->setRawFont(run.rawFont());
        cache->addGlyphs(glyphPos, run);
        cache->update();

        // TODO: Add proper bounding box
        QRectF box{};
        cache->generateVertices(verts, indices, state->transform, &box);
    }

    const bool fontUnderline = font.underline();
    const bool fontOverline = font.overline();
    const bool fontStrikeOut = font.strikeOut();
    if (!(fontUnderline || fontOverline || fontStrikeOut))
        return data;

    // Decorations are drawn through the SDF solid tile. For text made up solely
    // of color (emoji) glyphs, no monochrome glyph reserved/created it, so force
    // it to exist now.
    cache->ensureSolidTileTexture();
    const QCRhiDistanceFieldGlyphCache::TexCoord solidTC = cache->solidTileTexCoord();
    if (solidTC.isNull())
        return data;
    // Sample from the center of the reserved 0xFF tile so linear filtering
    // never picks up neighbouring atlas content. All four corners of every
    // decoration quad share the same texCoord — the rect is a flat lookup.
    const float solidTx = float(solidTC.x + solidTC.width * 0.5);
    const float solidTy = float(solidTC.y + solidTC.height * 0.5);

    auto appendDecorationQuad = [&](float x, float y, float w, float h) {
        QPointF p1(x,     y);
        QPointF p2(x + w, y);
        QPointF p3(x,     y + h);
        QPointF p4(x + w, y + h);
        if (!state->transform.isIdentity()) {
            p1 = state->transform.map(p1);
            p2 = state->transform.map(p2);
            p3 = state->transform.map(p3);
            p4 = state->transform.map(p4);
        }
        const uint32_t baseIndex = uint32_t(verts->size());
        QCRhiDistanceFieldGlyphCache::TexturedPoint2D v;
        v.set(float(p1.x()), float(p1.y()), solidTx, solidTy);
        verts->append(v);
        v.set(float(p2.x()), float(p2.y()), solidTx, solidTy);
        verts->append(v);
        v.set(float(p3.x()), float(p3.y()), solidTx, solidTy);
        verts->append(v);
        v.set(float(p4.x()), float(p4.y()), solidTx, solidTy);
        verts->append(v);
        indices->append(baseIndex + 0);
        indices->append(baseIndex + 2);
        indices->append(baseIndex + 3);
        indices->append(baseIndex + 3);
        indices->append(baseIndex + 1);
        indices->append(baseIndex + 0);
    };

    for (const QGlyphRun &run : std::as_const(glyphRuns)) {
        const QList<QPointF> &positions = run.positions();
        const QList<quint32> &indexes = run.glyphIndexes();
        if (positions.isEmpty() || indexes.isEmpty())
            continue;

        QRawFont rawFont = run.rawFont();
        // Sub-pixel-thick rects at fractional y can fall between pixel centers
        // when rendered without AA; clamp thickness to >= 1 px and snap y to
        // the pixel grid below. With the SDF-pipeline path the underlying draw
        // does keep AA enabled, but the glyph path already turns it off at the
        // call level (the text fill marks itself non-AA), so the same snapping
        // remains the safe choice here.
        const qreal lineThickness = qMax<qreal>(1.0, std::round(rawFont.lineThickness()));
        const QList<QPointF> advances = rawFont.advancesForGlyphIndexes(indexes);

        // For wrapped text, QTextLayout::glyphRuns() may return a single
        // QGlyphRun whose positions span every line — same x ranges, but
        // different baseline y per line. Glyphs are emitted in reading order,
        // so positions on a line are contiguous: scan sequentially and flush
        // the running (xMin, xMax) whenever y changes. Uses glyph advances so
        // spaces and RTL runs are covered.
        auto emitLine = [&](qreal baselineY, qreal xMin, qreal xMax) {
            if (xMin >= xMax)
                return;
            // Glyph positions use the baseline as the y-origin (standard Qt convention).
            const qreal worldX = glyphPos.x() + xMin;
            const qreal runWidth = xMax - xMin;
            const qreal worldBaseline = glyphPos.y() + baselineY;
            if (fontUnderline) {
                appendDecorationQuad(float(worldX),
                                     float(std::floor(worldBaseline + rawFont.underlinePosition())),
                                     float(runWidth), float(lineThickness));
            }
            if (fontOverline) {
                appendDecorationQuad(float(worldX),
                                     float(std::floor(worldBaseline - rawFont.ascent())),
                                     float(runWidth), float(lineThickness));
            }
            if (fontStrikeOut) {
                appendDecorationQuad(float(worldX),
                                     float(std::floor(worldBaseline - rawFont.ascent() / 3.0)),
                                     float(runWidth), float(lineThickness));
            }
        };

        const int count = qMin(positions.size(), advances.size());
        qreal lineY = positions.first().y();
        qreal xMin = positions.first().x();
        qreal xMax = xMin + advances.first().x();
        for (int i = 1; i < count; ++i) {
            const qreal y = positions[i].y();
            if (y != lineY) {
                emitLine(lineY, xMin, xMax);
                lineY = y;
                xMin = positions[i].x();
                xMax = xMin + advances[i].x();
            } else {
                xMin = qMin(xMin, positions[i].x());
                xMax = qMax(xMax, positions[i].x() + advances[i].x());
            }
        }
        emitLine(lineY, xMin, xMax);
    }
    return data;
}

QCDistanceFieldGlyphCache::FontKeyData *QCDistanceFieldGlyphCache::generateFromShapedText(
    QFontEngine *fontEngine,
    const quint32 *glyphIndexes,
    const QFixedPoint *glyphPositions,
    int glyphCount,
    const QCState &state,
    float devicePixelRatio,
    QCRhiDistanceFieldGlyphCache::VertexList *verts,
    QCRhiDistanceFieldGlyphCache::IndexList *indices)
{
    if (glyphCount == 0)
        return nullptr;

    // Build a QRawFont directly from the font engine, matching how Qt's own
    // QTextEngine::createGlyphRun() does it (qtextlayout.cpp). This is the only
    // correct source of font identity here: a QStaticTextItem's font can be a
    // meaningless painter-ambient font whenever usesRawFont is set.
    QRawFont rFont;
    QRawFontPrivate::get(rFont)->setFontEngine(fontEngine);

    FontKeyData *data = fontKeyDataFor(rFont);
    QCRhiDistanceFieldGlyphCache *cache = &data->nativeGlyphCache;

    verts->clear();
    indices->clear();
    m_colorVertices.clear();
    m_colorIndices.clear();

    // One unavoidable conversion pass: QFixedPoint (26.6 fixed) -> QPointF. Everything
    // downstream (generateVertices(), addGlyphRun()) works in qreal.
    QVarLengthArray<QPointF, 128> positions(glyphCount);
    for (int i = 0; i < glyphCount; ++i)
        positions[i] = glyphPositions[i].toPointF();

    // setRawData() does not copy the index array — it aliases the caller-owned
    // buffer for the lifetime of this call, so glyphIndexes/positions must stay
    // valid until run is no longer used below.
    QGlyphRun run;
    run.setRawFont(rFont);
    run.setRawData(glyphIndexes, positions.constData(), glyphCount);

    if (fontEngine && fontEngine->isColorFont()) {
        if (!m_colorCache)
            m_colorCache = new QCRhiColorGlyphCache(m_rhi);
        QColor glyphColor = Qt::black;
        if (state.fill.brushType == BrushColor) {
            glyphColor = QColor::fromRgbF(state.fill.innerColor.r,
                                          state.fill.innerColor.g,
                                          state.fill.innerColor.b);
        }
        m_colorCache->addGlyphRun(QPointF(0, 0), run, glyphColor, state.transform,
                                  devicePixelRatio, &m_colorVertices, &m_colorIndices);
    } else {
        cache->setRawFont(rFont);
        // addGlyphs() re-adds the font ascent internally before handing off to
        // setGlyphs(), which subtracts it again — the two cancel out, so passing
        // (0, 0) here is what actually yields m_position=(0,0), keeping the
        // absolute positions in the run as-is in world space.
        cache->addGlyphs(QPointF(0.0, 0.0), run);
        cache->update();
        QRectF box{};
        cache->generateVertices(verts, indices, state.transform, &box);
    }

    // Underline/overline/strikeout are deliberately not drawn here: QPainter always
    // draws text decorations itself via qt_draw_decoration_for_glyphs() (through
    // ordinary drawLine()/fillRect() calls on the same engine), never through
    // drawStaticTextItem(). Drawing them here too would double-render them for any
    // caller reached through QPainter::drawStaticText()/drawGlyphRun().
    return data;
}

void QCDistanceFieldGlyphCache::commitResourceUpdates(QRhiResourceUpdateBatch *batch)
{
    for (auto it = m_glyphCaches.begin(); it != m_glyphCaches.end(); ++it)
        it.value()->nativeGlyphCache.commitResourceUpdate(batch);
    if (m_colorCache && !m_colorCache->isEmpty())
        m_colorCache->commitResourceUpdate(batch);
}

QRhiTexture *QCDistanceFieldGlyphCache::getCurrentTextures(const FontKeyData *data) const
{
    if (!data)
        return nullptr;

    const auto l = data->nativeGlyphCache.getTextures();

    if (l.size() == 0)
        return nullptr;

    return l.constFirst().texture;
}

QRhiTexture *QCDistanceFieldGlyphCache::getOldTextures(const FontKeyData *data) const
{
    return data ? data->prevTextureState : nullptr;
}

void QCDistanceFieldGlyphCache::setOldTexture(FontKeyData *data, QRhiTexture *tex)
{
    if (data)
        data->prevTextureState = tex;
}

void QCDistanceFieldGlyphCache::optimizeCacheAfterRendering()
{
    for (auto it = m_glyphCaches.begin(); it != m_glyphCaches.end(); ++it)
        it.value()->nativeGlyphCache.optimizeAfterRendering();

    if (m_colorCache)
        m_colorCache->optimizeAfterRendering();

#ifdef QCPAINTER_CACHE_GLYPH_RUNS
#ifdef QCPAINTER_GLYPH_RUNS_CACHE_SIZE
    const int maxGlyphRuns = QCPAINTER_GLYPH_RUNS_CACHE_SIZE;
#else
    const int maxGlyphRuns = 1024;
#endif
    if (m_glyphRunCache.size() > maxGlyphRuns) {
        int eraseCount = 0;
        // First remove the ones which have been used only few times.
        // These can come when e.g. animating letterSpacing.
        erase_if(m_glyphRunCache, [&eraseCount](const QHash<GlyphCacheKey, GlyphCacheValue>::iterator it) {
            if (it->usageCounter < 5) {
                eraseCount++;
                return true;
            }
            return false;
        });
        // In the rare case where this isn't enough, clear the whole cache.
        const int cleanedMaxGlyphRuns = 0.9 * maxGlyphRuns;
        if (m_glyphRunCache.size() > cleanedMaxGlyphRuns) {
            eraseCount += m_glyphRunCache.size();
            m_glyphRunCache.clear();
        }
        qCDebug(QC_INFO) << "Max text glyph cache size" << maxGlyphRuns
                         << "reached. Removing" << eraseCount << "elements.";
    }
#endif
}

QCDistanceFieldGlyphCache::FontKey::FontKey(const QRawFont &font)
{
    QFontEngine *fe = QRawFontPrivate::get(font)->fontEngine;
    if (fe != nullptr)
        faceId = fe->faceId();
    style = font.style();
    weight = font.weight();
    if (faceId.filename.isEmpty()) {
        familyName = font.familyName();
        styleName = font.styleName();
    }
}

QT_END_NAMESPACE
