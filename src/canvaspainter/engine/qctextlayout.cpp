// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "engine/qcdistancefieldglyphcache_p.h"
#include "qctextlayout_p.h"
#include "engine/qcrhidistancefieldglyphcache_p.h"
#include <qtransform.h>
#include <qvarlengtharray.h>

QT_BEGIN_NAMESPACE

QCTextLayout::QCTextLayout() {}

QTextOption::WrapMode QCTextLayout::convertToQtWrapMode(QCPainter::WrapMode mode)
{
    switch (mode) {
    case QCPainter::WrapMode::NoWrap:
        return QTextOption::NoWrap;
        break;
    case QCPainter::WrapMode::Wrap:
        return QTextOption::WrapAtWordBoundaryOrAnywhere;
        break;
    case QCPainter::WrapMode::WordWrap:
        return QTextOption::WordWrap;
        break;
    case QCPainter::WrapMode::WrapAnywhere:
        return QTextOption::WrapAnywhere;
        break;
    }
    return QTextOption::NoWrap;
}

Qt::Alignment QCTextLayout::convertToQtAlignment(QCPainter::TextAlign alignment)
{
    switch (alignment) {
    case QCPainter::TextAlign::Left:
        return Qt::AlignLeft;
        break;
    case QCPainter::TextAlign::Center:
        return Qt::AlignCenter;
        break;
    case QCPainter::TextAlign::Right:
        return Qt::AlignRight;
        break;
    default:
        break;
    }
    return Qt::AlignLeft;
}

const QRectF QCTextLayout::bounds() const
{
    return m_bounds;
}

void QCTextLayout::generateVertexData(QCRhiDistanceFieldGlyphCache *cache)
{
    m_initialized = true;

    for (const auto &run : m_runs) {
        cache->addGlyphs({0, 0}, run);
        cache->update();
        cache->generateVertices(&verts, &indices, QTransform(), &m_bounds);
    }
}

float QCTextLayout::calculateVerticalAlignment(QCPainter::TextBaseline baseline, const QRectF &rect,
                                               const QFontMetrics &metrics, const QRectF &layoutRect)
{
    float offset = 0;
    switch (baseline) {
    case QCPainter::TextBaseline::Top:
        break;
    case QCPainter::TextBaseline::Hanging:
        offset = -metrics.height() + metrics.ascent();
        break;
    case QCPainter::TextBaseline::Middle:
        offset = rect.height() * 0.5 - layoutRect.height() * 0.5;
        break;
    case QCPainter::TextBaseline::Alphabetic:
        offset = rect.height() - layoutRect.height() + metrics.descent();
        break;
    case QCPainter::TextBaseline::Bottom:
        offset = rect.height() - layoutRect.height();
        break;
    }

    return offset;
}

QT_END_NAMESPACE
