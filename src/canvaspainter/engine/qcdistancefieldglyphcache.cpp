// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "engine/qctextlayout_p.h"
#include "qcdistancefieldglyphcache_p.h"
#include "qctext.h"
#include <private/qrawfont_p.h>

QT_BEGIN_NAMESPACE

QCDistanceFieldGlyphCache::QCDistanceFieldGlyphCache(QRhi *rhi /*, QRawFont& font*/)
    : m_rhi(rhi)
{
    m_layout.setCacheEnabled(true);
}

QCDistanceFieldGlyphCache::~QCDistanceFieldGlyphCache()
{
    for (auto &cache : std::as_const(m_glyphCaches)) {
        delete cache.nativeGlyphCache;
        /*delete cache.layout;*/
    }
}

QList<QGlyphRun> QCDistanceFieldGlyphCache::generateGlyphRuns(
    const QString &text, const QRectF &rect, const QFont &font,
    const QFontMetrics &metrics, QCState *state, QCPainter::TextAlign alignment)
{
    QTextOption option;
    option.setWrapMode(QCTextLayout::convertToQtWrapMode(state->textWrapMode));
    option.setAlignment(QCTextLayout::convertToQtAlignment(alignment));
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

std::tuple<std::vector<QCRhiDistanceFieldGlyphCache::TexturedPoint2D>, std::vector<uint32_t>>
QCDistanceFieldGlyphCache::generate(const QString &text, const QRectF &rect, const QFont &font, QCState *state, QCPainter::TextAlign alignment)
{
    std::vector<QCRhiDistanceFieldGlyphCache::TexturedPoint2D> textVerts{};
    std::vector<uint32_t> textIndices{};

    // Remove raw fonts
    auto rFont = QRawFont::fromFont(font);
    FontKeyData *data;
    const auto metrics = QFontMetrics(font);

    // Get glyph cache
    QCRhiDistanceFieldGlyphCache *cache;
    FontKey key = {rFont};
    if (m_glyphCaches.contains(key)) {
        data = &m_glyphCaches[key];
        cache = data->nativeGlyphCache;
    } else {
        cache = new QCRhiDistanceFieldGlyphCache(m_rhi);
        FontKeyData f{ nullptr, cache, {} };
        m_glyphCaches.insert(key, std::move(f));
        cache->setRawFont(rFont);
        data = &m_glyphCaches[key];
    }

    QList<QGlyphRun> glyphRuns;
    float textY = 0;
#ifdef QCPAINTER_CACHE_GLYPH_RUNS
    const GlyphCacheKey hashKey {
        key,
        text,
        state->textAlignment,
        state->textWrapMode,
        float(rFont.pixelSize()),
        float(rect.width()),
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
        m_glyphRunCache.insert(hashKey, { glyphRuns, m_layout.boundingRect(), 0 });
    }
#else
    // QGlyphRuns caching disabled, generate
    glyphRuns = generateGlyphRuns(text, rect,
                                  font, metrics,
                                  state, alignment);
    textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, m_layout.boundingRect());
#endif

    qsizetype index = 0;
    const QPointF glyphPos(rect.x(), rect.y() + textY);
    for (const auto &run : std::as_const(glyphRuns)) {
        cache->addGlyphs(glyphPos, run);
        cache->update();

        QVarLengthArray<QCRhiDistanceFieldGlyphCache::TexturedPoint2D, 256> verts{};
        QVarLengthArray<ushort, 384> indices{};
        // TODO: Add proper bounding box
        QRectF box{};
        cache->generateVertices(&verts, &indices, state->transform, &box);

        for (const auto t : verts)
            textVerts.push_back(t);

        for (const auto t : indices)
            textIndices.push_back(t + index);

        index += verts.size();
    }
    return {textVerts, textIndices};
}

std::tuple<std::vector<QCRhiDistanceFieldGlyphCache::TexturedPoint2D>, std::vector<uint32_t>>
QCDistanceFieldGlyphCache::generate(QCText &text, const QFont &font, QCState *state, QCContext *ctx)
{
    // Remove raw fonts
    auto rFont = QRawFont::fromFont(font);
    FontKeyData *data;

    QCRhiDistanceFieldGlyphCache *cache;
    FontKey key = {rFont};
    if (m_glyphCaches.contains(key)) {
        data = &m_glyphCaches[key];
        cache = data->nativeGlyphCache;
    } else {
        cache = new QCRhiDistanceFieldGlyphCache(m_rhi);
        FontKeyData f{ nullptr, cache, {} };
        m_glyphCaches.insert(key, std::move(f));
        cache->setRawFont(rFont);
        data = &m_glyphCaches[key];
    }

    QCTextCache &ct = ctx->cachedTexts[text.getId()];
    if (!ct.layout)
        ct.layout = new QCTextLayout();
    auto *layout = ct.layout;
    layout->setOptimized(text.optimized());
    if (layout->isOptimized()) {
        layout->createOptimizedLayout(m_rhi, font, cache);
        return layout->constructOptimizedBufferForString(text.text());
    } else {
        layout->createDefaultLayout(m_rhi, text, font, state, cache, data);
        return layout->getDefaultBuffer();
    }
}

void QCDistanceFieldGlyphCache::commitResourceUpdates(QRhiResourceUpdateBatch *batch)
{
    for (auto it = m_glyphCaches.begin(); it != m_glyphCaches.end(); ++it)
        it.value().nativeGlyphCache->commitResourceUpdate(batch);
}

QRhiTexture *QCDistanceFieldGlyphCache::getCurrentTextures(const FontKey &key) const
{
    if (!m_glyphCaches.contains(key))
        return nullptr;

    const auto l = m_glyphCaches.value(key).nativeGlyphCache->getTextures();

    if (l.size() == 0)
        return nullptr;

    return l.constFirst().texture;
}

QRhiTexture *QCDistanceFieldGlyphCache::getOldTextures(const FontKey &key) const
{
    if (!m_glyphCaches.contains(key))
        return nullptr;

    return m_glyphCaches.value(key).prevTextureState;
}

void QCDistanceFieldGlyphCache::setOldTexture(FontKey key, QRhiTexture *tex)
{
    if (!m_glyphCaches.contains(key))
        return;

    m_glyphCaches[key].prevTextureState = tex;
}

void QCDistanceFieldGlyphCache::optimizeCache()
{
#ifdef QCPAINTER_CACHE_GLYPH_RUNS
#ifdef QCPAINTER_GLYPH_RUNS_CACHE_SIZE
    const int maxGlyphRuns = QCPAINTER_GLYPH_RUNS_CACHE_SIZE;
#else
    const int maxGlyphRuns = 1024;
#endif
    if (m_glyphRunCache.size() > maxGlyphRuns) {
        qDebug() << "Text glyph cache full, cleaning.";
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
        qDebug() << "Removed: " << eraseCount;
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
