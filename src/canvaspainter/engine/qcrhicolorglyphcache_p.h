// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCRHICOLORGLYPHCACHE_P_H
#define QCRHICOLORGLYPHCACHE_P_H

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

#include "qcrhidistancefieldglyphcache_p.h"
#include <QtGui/QImage>
#include <QtGui/QRawFont>
#include <QtGui/QGlyphRun>
#include <QtGui/private/qfontengine_p.h>
#include <rhi/qrhi.h>
#include "qcareaallocator_p.h"

QT_BEGIN_NAMESPACE

class QCRhiColorGlyphCache
{
public:
    QCRhiColorGlyphCache(QRhi *rhi);
    ~QCRhiColorGlyphCache();

    void addGlyphRun(const QPointF &glyphPos,
                     const QGlyphRun &run,
                     const QColor &color,
                     const QTransform &transform,
                     float devicePixelRatio,
                     QCRhiDistanceFieldGlyphCache::VertexList *verts,
                     QCRhiDistanceFieldGlyphCache::IndexList *indices);

    void commitResourceUpdate(QRhiResourceUpdateBatch *batch);

    QRhiTexture *texture() const { return m_atlas; }
    bool isEmpty() const { return m_atlas == nullptr; }

private:
    struct GlyphKey
    {
        QFontEngine::FaceId faceId;
        quint32 glyph;
        int pixelSize;
        float scale; // rasterization scale (device pixel ratio)
        QRgb color;  // text color, used by COLR "foreground" layers
        bool operator==(const GlyphKey &o) const
        {
            return glyph == o.glyph && pixelSize == o.pixelSize && scale == o.scale
                   && color == o.color && faceId == o.faceId;
        }
    };
    friend size_t qHash(const GlyphKey &k, size_t seed)
    {
        return qHashMulti(seed, k.faceId, k.glyph, k.pixelSize, k.scale, k.color);
    }

    struct GlyphData
    {
        QRect atlasRect;
        QPointF bearing;
        bool valid = false;
    };

    void ensureAtlas();
    void growAtlas(int requiredSize);
    const GlyphData &ensureGlyph(const GlyphKey &key, QFontEngine *fe,
                                 const QColor &color, const QTransform &rasterTransform);
    bool useTextureResizeWorkaround() const;

    QRhi *m_rhi = nullptr;
    QRhiTexture *m_atlas = nullptr;
    QRhiResourceUpdateBatch *m_batch = nullptr;
    QCAreaAllocator *m_allocator = nullptr;
    QHash<GlyphKey, GlyphData> m_glyphs;
    int m_atlasSize = 0;    // current physical atlas texture size (square, px)
    int m_maxAtlasSize = 0; // allocator coordinate space and growth ceiling (px)
    QImage m_atlasImage;    // CPU shadow of the atlas (resize workaround only)
};

QT_END_NAMESPACE

#endif // QCRHICOLORGLYPHCACHE_P_H
