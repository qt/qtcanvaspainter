// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcrhicolorglyphcache_p.h"
#include <QtGui/private/qrawfont_p.h>

QT_BEGIN_NAMESPACE

// Padding between glyphs so linear filtering never samples a neighbour.
static constexpr int QC_COLOR_GLYPH_PADDING = 1;

QCRhiColorGlyphCache::QCRhiColorGlyphCache(QRhi *rhi)
    : m_rhi(rhi)
{
    m_batch = rhi->nextResourceUpdateBatch();
}

QCRhiColorGlyphCache::~QCRhiColorGlyphCache()
{
    if (m_batch)
        m_batch->release();
    if (m_atlas)
        m_atlas->deleteLater();
    delete m_allocator;
}

bool QCRhiColorGlyphCache::useTextureResizeWorkaround() const
{
    return m_rhi->backend() == QRhi::OpenGLES2;
}

void QCRhiColorGlyphCache::ensureAtlas()
{
    if (m_atlas)
        return;

    m_maxAtlasSize = qMin(2048, m_rhi->resourceLimit(QRhi::TextureSizeMax));
    m_atlasSize = qMin(256, m_maxAtlasSize);
    m_atlas = m_rhi->newTexture(QRhiTexture::RGBA8, QSize(m_atlasSize, m_atlasSize), 1,
                                QRhiTexture::UsedAsTransferSource);
    m_atlas->setName("QCColorGlyphAtlas");
    if (!m_atlas->create()) {
        qWarning("Failed to create color glyph atlas");
        delete m_atlas;
        m_atlas = nullptr;
        return;
    }

    const QByteArray zeroBuf(m_atlasSize * m_atlasSize * 4, 0);
    QRhiTextureSubresourceUploadDescription zeroSubres(zeroBuf.constData(), zeroBuf.size());
    zeroSubres.setSourceSize(QSize(m_atlasSize, m_atlasSize));
    m_batch->uploadTexture(m_atlas, QRhiTextureUploadEntry(0, 0, zeroSubres));

    m_allocator = new QCAreaAllocator(QSize(m_maxAtlasSize, m_maxAtlasSize));

    if (useTextureResizeWorkaround()) {
        m_atlasImage = QImage(m_atlasSize, m_atlasSize, QImage::Format_RGBA8888_Premultiplied);
        m_atlasImage.fill(Qt::transparent);
    }
}

void QCRhiColorGlyphCache::growAtlas(int requiredSize)
{
    int newSize = m_atlasSize;
    while (newSize < requiredSize)
        newSize *= 2;
    newSize = qMin(newSize, m_maxAtlasSize);
    if (newSize == m_atlasSize)
        return;

    QRhiTexture *newAtlas = m_rhi->newTexture(QRhiTexture::RGBA8, QSize(newSize, newSize), 1,
                                              QRhiTexture::UsedAsTransferSource);
    newAtlas->setName("QCColorGlyphAtlas");
    if (!newAtlas->create()) {
        qWarning("Failed to grow color glyph atlas");
        delete newAtlas;
        return;
    }

    if (useTextureResizeWorkaround()) {
        // Enlarge the CPU shadow first (the area outside the old image is
        // filled with transparent black) and upload it in full, so the region
        // beyond the old size does not stay undefined in the new texture.
        m_atlasImage = m_atlasImage.copy(0, 0, newSize, newSize);
        QRhiTextureSubresourceUploadDescription subres(m_atlasImage.constBits(),
                                                       m_atlasImage.sizeInBytes());
        subres.setSourceSize(QSize(newSize, newSize));
        m_batch->uploadTexture(newAtlas, QRhiTextureUploadEntry(0, 0, subres));
    } else {
        const QByteArray zeroBuf(newSize * newSize * 4, 0);
        QRhiTextureSubresourceUploadDescription zeroSubres(zeroBuf.constData(), zeroBuf.size());
        zeroSubres.setSourceSize(QSize(newSize, newSize));
        m_batch->uploadTexture(newAtlas, QRhiTextureUploadEntry(0, 0, zeroSubres));
        m_batch->copyTexture(newAtlas, m_atlas);
    }

    m_atlas->deleteLater();
    m_atlas = newAtlas;
    m_atlasSize = newSize;
}

static inline QRect allocationRect(const QRect &atlasRect)
{
    return QRect(atlasRect.x() - QC_COLOR_GLYPH_PADDING,
                 atlasRect.y() - QC_COLOR_GLYPH_PADDING,
                 atlasRect.width() + QC_COLOR_GLYPH_PADDING * 2,
                 atlasRect.height() + QC_COLOR_GLYPH_PADDING * 2);
}

void QCRhiColorGlyphCache::referenceGlyph(const GlyphKey &key, GlyphData &gd)
{
    // Count one use per frame, so a glyph drawn many times in a frame is not
    // weighted more heavily than one drawn once (matches the SDF cache).
    if (!m_referencedThisFrame.contains(key)) {
        ++gd.ref;
        m_referencedThisFrame.insert(key);
    }
    m_unusedGlyphs.remove(key);
}

// Free atlas space held by unused glyphs until allocSize fits (or none are
// left to evict). Returns the resulting allocation, or a null rect.
QRect QCRhiColorGlyphCache::evictUntilAllocated(const QSize &allocSize)
{
    QRect alloc;
    while (alloc.isNull() && !m_unusedGlyphs.isEmpty()) {
        const GlyphKey unused = *m_unusedGlyphs.constBegin();
        auto it = m_glyphs.constFind(unused);
        if (it != m_glyphs.constEnd() && it->valid)
            m_allocator->deallocate(allocationRect(it->atlasRect));
        m_unusedGlyphs.remove(unused);
        m_glyphs.remove(unused);
        alloc = m_allocator->allocate(allocSize);
    }
    return alloc;
}

const QCRhiColorGlyphCache::GlyphData &
QCRhiColorGlyphCache::ensureGlyph(const GlyphKey &key, QFontEngine *fe,
                                  QColor color, const QTransform &rasterTransform)
{
    auto it = m_glyphs.find(key);
    if (it != m_glyphs.end()) {
        if (it->valid)
            referenceGlyph(key, it.value());
        return it.value();
    }

    GlyphData gd;

    ensureAtlas();
    if (!m_atlas)
        return m_invalidGlyph;

    // The bitmap is already in Format_ARGB32_Premultiplied for color fonts.
    QImage bitmap = fe->bitmapForGlyph(key.glyph, QFixedPoint(), rasterTransform, color);
    if (bitmap.isNull())
        return *m_glyphs.insert(key, gd); // permanently invalid: no color bitmap for this glyph

    if (bitmap.format() != QImage::Format_RGBA8888_Premultiplied)
        bitmap = bitmap.convertToFormat(QImage::Format_RGBA8888_Premultiplied);

    const QSize glyphSize = bitmap.size();
    const QSize allocSize(glyphSize.width() + QC_COLOR_GLYPH_PADDING * 2,
                          glyphSize.height() + QC_COLOR_GLYPH_PADDING * 2);
    QRect alloc = m_allocator->allocate(allocSize);
    if (alloc.isNull())
        alloc = evictUntilAllocated(allocSize);
    if (alloc.isNull()) {
        qWarning("Color glyph atlas full; emoji glyph dropped");
        return m_invalidGlyph;
    }

    // The allocator hands out positions across the full (max) coordinate space;
    // grow the physical texture to cover this allocation before uploading into
    // it. The allocation is bounded by m_maxAtlasSize, so the grow always
    // succeeds in reaching it.
    const int required = qMax(alloc.x() + alloc.width(), alloc.y() + alloc.height());
    if (required > m_atlasSize)
        growAtlas(required);

    const QPoint dst = alloc.topLeft() + QPoint(QC_COLOR_GLYPH_PADDING, QC_COLOR_GLYPH_PADDING);

    QRhiTextureSubresourceUploadDescription subres(bitmap.constBits(), bitmap.sizeInBytes());
    subres.setSourceSize(glyphSize);
    subres.setDestinationTopLeft(dst);
    m_batch->uploadTexture(m_atlas, QRhiTextureUploadEntry(0, 0, subres));

    if (useTextureResizeWorkaround()) {
        const int bytesPerRow = glyphSize.width() * 4;
        for (int row = 0; row < glyphSize.height(); ++row) {
            memcpy(m_atlasImage.scanLine(dst.y() + row) + dst.x() * 4,
                   bitmap.constScanLine(row),
                   bytesPerRow);
        }
    }

    // Bearing: where the bitmap's top-left sits relative to the glyph pen
    // position on the baseline (alphaMapBoundingBox returns this offset).
    const glyph_metrics_t gm =
        fe->alphaMapBoundingBox(key.glyph, QFixedPoint(), rasterTransform,
                                QFontEngine::Format_ARGB);

    gd.atlasRect = QRect(dst, glyphSize);
    gd.bearing = QPointF(gm.x.toReal(), gm.y.toReal());
    gd.valid = true;
    GlyphData &stored = *m_glyphs.insert(key, gd);
    referenceGlyph(key, stored);
    return stored;
}

void QCRhiColorGlyphCache::releaseGlyphs(const QSet<GlyphKey> &glyphs)
{
    for (const GlyphKey &key : glyphs) {
        auto it = m_glyphs.find(key);
        if (it == m_glyphs.end())
            continue;
        if (it->ref > 0 && --it->ref == 0)
            m_unusedGlyphs.insert(key);
    }
}

void QCRhiColorGlyphCache::optimizeAfterRendering()
{
    releaseGlyphs(m_referencedPrevFrame);
    m_referencedPrevFrame.swap(m_referencedThisFrame);
    m_referencedThisFrame.clear();
}

void QCRhiColorGlyphCache::addGlyphRun(const QPointF &glyphPos,
                                       const QGlyphRun &run,
                                       QColor color,
                                       const QTransform &transform,
                                       float devicePixelRatio,
                                       QCRhiDistanceFieldGlyphCache::VertexList *verts,
                                       QCRhiDistanceFieldGlyphCache::IndexList *indices)
{
    const QRawFont rawFont = run.rawFont();
    QFontEngine *fe = QRawFontPrivate::get(rawFont)->fontEngine;
    if (!fe || !fe->isColorFont())
        return;

    const QList<quint32> glyphIndexes = run.glyphIndexes();
    const QList<QPointF> positions = run.positions();
    const int count = qMin(glyphIndexes.size(), positions.size());

    const QFontEngine::FaceId faceId = fe->faceId();
    const int pixelSize = qRound(rawFont.pixelSize());
    // The color participates in the key because COLR fonts can have layers
    // painted in the text ("foreground") color.
    const QRgb rgba = color.rgba();

    // Rasterize at the device pixel ratio so the bitmaps match the physical
    // resolution of the render target; the quads below stay in logical
    // coordinates and the projection scales them back up. The painter
    // transform is still only applied to the quad corners (consistent with
    // the SDF path), so a scaling transform does reduce sharpness.
    qreal scale = devicePixelRatio > 0.0f ? devicePixelRatio : 1.0;
    QTransform rasterTransform = QTransform::fromScale(scale, scale);
    if (!fe->supportsTransformation(rasterTransform)) {
        scale = 1.0;
        rasterTransform = QTransform();
    }

    for (int i = 0; i < count; ++i) {
        const GlyphKey key{ faceId, glyphIndexes.at(i), pixelSize, float(scale), rgba };
        const GlyphData &gd = ensureGlyph(key, fe, color, rasterTransform);
        if (!gd.valid)
            continue;

        // Pen position on the baseline for this glyph, then offset by the
        // bearing. The bitmap and its metrics are in device pixels; divide by
        // the rasterization scale to get back to logical coordinates.
        const QPointF pen = glyphPos + positions.at(i);
        const qreal x = pen.x() + gd.bearing.x() / scale;
        const qreal y = pen.y() + gd.bearing.y() / scale;
        const qreal w = gd.atlasRect.width() / scale;
        const qreal h = gd.atlasRect.height() / scale;

        QPointF p1(x,     y);
        QPointF p2(x + w, y);
        QPointF p3(x,     y + h);
        QPointF p4(x + w, y + h);
        if (!transform.isIdentity()) {
            p1 = transform.map(p1);
            p2 = transform.map(p2);
            p3 = transform.map(p3);
            p4 = transform.map(p4);
        }

        // Texture coords in atlas texels (shader divides by textureSize()).
        const float tx1 = gd.atlasRect.x();
        const float ty1 = gd.atlasRect.y();
        const float tx2 = tx1 + float(gd.atlasRect.width());
        const float ty2 = ty1 + float(gd.atlasRect.height());

        const uint32_t base = uint32_t(verts->size());
        QCRhiDistanceFieldGlyphCache::TexturedPoint2D v;
        v.set(float(p1.x()), float(p1.y()), tx1, ty1); verts->append(v);
        v.set(float(p2.x()), float(p2.y()), tx2, ty1); verts->append(v);
        v.set(float(p3.x()), float(p3.y()), tx1, ty2); verts->append(v);
        v.set(float(p4.x()), float(p4.y()), tx2, ty2); verts->append(v);

        indices->append(base + 0);
        indices->append(base + 2);
        indices->append(base + 3);
        indices->append(base + 3);
        indices->append(base + 1);
        indices->append(base + 0);
    }
}

void QCRhiColorGlyphCache::commitResourceUpdate(QRhiResourceUpdateBatch *batch)
{
    batch->merge(m_batch);
    m_batch->release();
    m_batch = m_rhi->nextResourceUpdateBatch();
}

QT_END_NAMESPACE
