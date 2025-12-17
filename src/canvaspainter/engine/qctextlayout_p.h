// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCTEXTLAYOUT_P_H
#define QCTEXTLAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "engine/qcdistancefieldglyphcache_p.h"
#include "engine/qcpainterengineutils_p.h"
#include "engine/qcrhidistancefieldglyphcache_p.h"
#include <vector>
#include <QString>
#include <QTextLayout>
#include <qglyphrun.h>

QT_BEGIN_NAMESPACE

using TextPoints
    = std::tuple<std::vector<QCRhiDistanceFieldGlyphCache::TexturedPoint2D>, std::vector<uint32_t>>;

class QCTextLayout
{
public:
    QCTextLayout();

    const QRectF bounds() const;

    static QTextOption::WrapMode convertToQtWrapMode(QCPainter::WrapMode mode);
    static Qt::Alignment convertToQtAlignment(QCPainter::TextAlign alignment);
    static float calculateVerticalAlignment(QCPainter::TextBaseline baseline, const QRectF &rect,
                                            const QFontMetrics &metrics, const QRectF &layoutRect);

private:
    void generateVertexData(QCRhiDistanceFieldGlyphCache *cache);

    QTextLayout m_textLayout;
    QList<QGlyphRun> m_runs;
    bool m_initialized = false;

    QVarLengthArray<QCRhiDistanceFieldGlyphCache::TexturedPoint2D, 256> verts;
    QVarLengthArray<ushort, 384> indices;

    QRectF m_bounds;
};

QT_END_NAMESPACE

#endif
