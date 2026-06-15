// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCRHIDISTANCEFIELDGLYPHCACHE_H
#define QCRHIDISTANCEFIELDGLYPHCACHE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/QPainterPath>
#include <QtGui/private/qdistancefield_p.h>
#include <QtGui/private/qdatabuffer_p.h>
#include <rhi/qrhi.h>
#include "qcareaallocator_p.h"
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCRhiDistanceFieldGlyphCache
{
public:
    struct TextureInfo
    {
        QRhiTexture *texture = nullptr;
        QSize size;
        QRect allocatedArea;
        QDistanceField image;
        int padding = -1;
        QVarLengthArray<QRhiTextureUploadEntry, 16> uploads;

        bool operator==(const TextureInfo &other) const { return texture == other.texture; }

        TextureInfo(const QRect &preallocRect = QRect())
            : allocatedArea(preallocRect)
        {}
    };

    struct TexCoord
    {
        qreal x = 0;
        qreal y = 0;
        qreal width = -1;
        qreal height = -1;
        qreal xMargin = 0;
        qreal yMargin = 0;

        bool isNull() const { return width <= 0 || height <= 0; }
        bool isValid() const { return width >= 0 && height >= 0; }
    };

    struct Metrics
    {
        qreal width;
        qreal height;
        qreal baselineX;
        qreal baselineY;

        bool isNull() const { return width == 0 || height == 0; }
    };

    struct TexturedPoint2D
    {
        float x, y;
        float tx, ty;
        void set(float nx, float ny, float ntx, float nty)
        {
            x = nx;
            y = ny;
            tx = ntx;
            ty = nty;
        }
    };

    struct GlyphData
    {
        TextureInfo *texture = nullptr;
        TexCoord texCoord;
        QRectF boundingRect;
        QPainterPath path;
        quint32 ref = 0;
    };

    struct GlyphPosition
    {
        glyph_t glyph;
        QPointF position;
    };

    using VertexList = QVarLengthArray<TexturedPoint2D, 256>;
    using IndexList = QVarLengthArray<uint32_t, 384>;

public:
    QCRhiDistanceFieldGlyphCache(QRhi *rhi);
    ~QCRhiDistanceFieldGlyphCache();

    bool populate(const QList<glyph_t> &glyphs);
    void update();

    void setRawFont(const QRawFont &font);
    bool addGlyphs(QPointF position, const QGlyphRun &glyphs);
    void createTexture(TextureInfo *texInfo, int width, int height);
    void createTexture(TextureInfo *texInfo, int width, int height, const void *pixels);
    void resizeTexture(TextureInfo *texInfo, int width, int height);

    void requestGlyphs(const QSet<glyph_t> &glyphs);
    void storeGlyphs(const QList<QDistanceField> &glyphs);
    void setGlyphsPosition(const QList<GlyphPosition> &glyphs);
    void referenceGlyphs(const QSet<glyph_t> &glyphs);

    void optimizeAfterRendering();

    bool setGlyphs(QPointF position, const QGlyphRun &glyphs);
    void setGlyphTexture(const QList<glyph_t> &glyphs, const TextureInfo &tex);

    void generateVertices(
        VertexList *verts,
        IndexList *indices,
        const QTransform &transform,
        QRectF *boundingRect);

    void commitResourceUpdate(QRhiResourceUpdateBatch *batch);
    QList<TextureInfo> &getTextures() { return m_textures; }

    GlyphData &glyphData(glyph_t glyph);

    // A small region of the atlas filled with the maximum SDF value (0xFF) so
    // that sdfFontAlpha() returns ~1.0 when sampled from any point within it.
    // The intent is to let callers draw non-glyph geometry through the same
    // shader pipeline as the glyphs — today's sole consumer is the underline /
    // overline / strikeout quads emitted alongside glyph quads in
    // QCDistanceFieldGlyphCache::generate(); add other "needs a flat-coverage
    // SDF sample" callers (selection backgrounds, cursors, …) here when they
    // appear. Returns an invalid TexCoord if the tile could not be allocated.
    TexCoord solidTileTexCoord() const { return m_solidTileTexCoord; }

    // Guarantees the solid tile exists AND that its atlas texture is created and
    // uploaded, even when no glyph was ever added to this cache. Needed when a
    // run consists solely of color (emoji) glyphs but still requires text
    // decorations (underline/overline/strikeout), which are drawn through the
    // solid tile.
    void ensureSolidTileTexture();

private:

    struct ReferenceFont {
        QRawFont mutatedFont;
        qreal pixelSize;
        bool doubleGlyphResolution;
        int glyphCount;
    };

    static TextureInfo s_emptyTexture;

    QRhi *m_rhi;
    QCAreaAllocator *m_areaAllocator = nullptr;
    QRhiResourceUpdateBatch *m_batch;

    QPointF m_position;
    QGlyphRun m_glyphs;
    QRawFont m_referenceFont;
    mutable int m_maxTextureSize = 1024;
    int m_maxTextureCount = 1;

    QHash<glyph_t, GlyphData> m_glyphsData;
    QSet<glyph_t> m_populatingGlyphs;
    QSet<glyph_t> m_unusedGlyphs;
    QSet<glyph_t> m_referencedThisFrame;
    QSet<glyph_t> m_referencedPrevFrame;
    QHash<glyph_t, TextureInfo *> m_glyphsTexture;
    QList<TextureInfo> m_textures;
    QDataBuffer<glyph_t> m_pendingGlyphs;
    QHash<QRawFont, ReferenceFont> m_rawFontCache;

    TexCoord m_solidTileTexCoord;
    TextureInfo *m_solidTileTexture = nullptr;
    bool m_solidTileUploadPending = false;

    void ensureSolidTile();
    void uploadSolidTileIfNeeded();

    GlyphData &emptyData(glyph_t glyph);
    int maxTextureSize() const;
    void markGlyphsToRender(const QList<glyph_t> &glyphs);
    void releaseGlyphs(const QSet<glyph_t> &glyphs);
    bool useTextureResizeWorkaround() const;
    void updateRhiTexture(QRhiTexture *oldTex, QRhiTexture *newTex, QSize newTexSize);
    Metrics glyphMetrics(glyph_t glyph, qreal pixelSize);

    qreal fontScale(qreal pixelSize) const;

    qreal distanceFieldRadius() const
    {
        auto doubleRes = m_rawFontCache[m_referenceFont].doubleGlyphResolution;

        return QT_DISTANCEFIELD_RADIUS(doubleRes)
               / qreal(QT_DISTANCEFIELD_SCALE(doubleRes));
    }

    TextureInfo *textureInfo(int index)
    {
        // Make sure there are enough m_textures for index.
        for (int i = m_textures.size(); i <= index; ++i)
            m_textures.append(TextureInfo());

        return &m_textures[index];
    }

    TexCoord glyphTexCoord(glyph_t glyph) { return glyphData(glyph).texCoord; };
};

QT_END_NAMESPACE

#endif // QCRHIDISTANCEFIELDGLYPHCACHE_H
