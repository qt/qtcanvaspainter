// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "engine/qcdistancefieldglyphcache_p.h"
#include "qctext.h"
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

bool QCTextLayout::isOptimized() const
{
    return m_optimized;
}

void QCTextLayout::setOptimized(bool o)
{
    m_optimized = o;
}

const QRectF QCTextLayout::bounds() const
{
    return m_bounds;
}

void QCTextLayout::createOptimizedLayout(QRhi *rhi, const QFont &font, QCRhiDistanceFieldGlyphCache *cache)
{
    Q_UNUSED(rhi);

    if (!m_optimized || m_initialized)
        return;

    m_textLayout.setText(m_commonText);
    m_textLayout.setFont(font);
    m_textLayout.beginLayout();
    QTextLine line = m_textLayout.createLine();
    line.setLineWidth(1080);
    line.setPosition(QPointF(0, 0));
    m_textLayout.endLayout();

    m_runs = m_textLayout.glyphRuns();

    generateVertexData(cache);
}

void QCTextLayout::createDefaultLayout(
    QRhi *rhi,
    QCText &text,
    const QFont &font,
    QCState *state,
    QCRhiDistanceFieldGlyphCache *cache,
    QCDistanceFieldGlyphCache::FontKeyData *data)
{
    Q_UNUSED(rhi);
    if (m_optimized)
        return;

    defaultVerts.clear();
    defaultIndices.clear();

    auto rFont = QRawFont::fromFont(font);
    //auto requiredGlyphs = rFont.glyphIndexesForString(text.text());

    // isDirty |= data->nativeGlyphCache->containsNewGlyphs(requiredGlyphs);

    // TODO:Move layout to each separate draw call (QCFont/QCText object) This is quite
    // intensive so should be updated sparingly
    QTextOption option;
    option.setWrapMode(convertToQtWrapMode(state->textWrapMode));
    option.setAlignment(convertToQtAlignment(state->textAlignment));
    m_textLayout.setTextOption(option);
    m_textLayout.setText(text.text());
    m_textLayout.setFont(font);
    const auto metrics = QFontMetrics(font);
    const int leading = metrics.leading();
    float layoutHeight = 0;
    m_textLayout.beginLayout();
    while (true) {
        QTextLine line = m_textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(text.width());
        layoutHeight += leading;
        // we render everything at 0, 0 to minimize layout changes
        line.setPosition(QPointF(0, layoutHeight));
        layoutHeight += line.height() + state->textLineHeight;
    }
    m_textLayout.endLayout();

    generateDefaultVertexData(text, font, state, cache, data);
}

TextPoints QCTextLayout::constructOptimizedBufferForString(const QString &txt)
{
    TextPoints points;

    if (!m_optimized || !m_initialized)
        return points;

    std::vector<QCRhiDistanceFieldGlyphCache::TexturedPoint2D> v;
    std::vector<uint32_t> i;

    for (auto &c : txt) {
        if (!m_commonText.contains(c)) {
            qWarning() << "Only digits and punctuation allowed";
            return points;
        }
    }

    auto xpos = 0.0f;

    for (auto &c : txt) {
        auto c1 = verts[m_commonText.indexOf(c) * 4 + 0];
        auto c2 = verts[m_commonText.indexOf(c) * 4 + 1];
        auto c3 = verts[m_commonText.indexOf(c) * 4 + 2];
        auto c4 = verts[m_commonText.indexOf(c) * 4 + 3];

        auto width = c4.x - c3.x;

        QCRhiDistanceFieldGlyphCache::TexturedPoint2D n1 = {xpos, c1.y, c1.tx, c1.ty};
        QCRhiDistanceFieldGlyphCache::TexturedPoint2D n2 = {xpos + width, c2.y, c2.tx, c2.ty};
        QCRhiDistanceFieldGlyphCache::TexturedPoint2D n3 = {xpos, c3.y, c3.tx, c3.ty};
        QCRhiDistanceFieldGlyphCache::TexturedPoint2D n4 = {xpos + width, c4.y, c4.tx, c4.ty};

        auto i1 = static_cast<uint32_t>(v.size() + 0);
        auto i2 = static_cast<uint32_t>(v.size() + 2);
        auto i3 = static_cast<uint32_t>(v.size() + 3);
        auto i4 = static_cast<uint32_t>(v.size() + 3);
        auto i5 = static_cast<uint32_t>(v.size() + 1);
        auto i6 = static_cast<uint32_t>(v.size() + 0);

        v.push_back(n1);
        v.push_back(n2);
        v.push_back(n3);
        v.push_back(n4);

        i.push_back(i1);
        i.push_back(i2);
        i.push_back(i3);
        i.push_back(i4);
        i.push_back(i5);
        i.push_back(i6);

        xpos += width;
    }

    return std::make_tuple(v, i);
}

TextPoints QCTextLayout::getDefaultBuffer() const
{
    return {defaultVerts, defaultIndices};
}

void QCTextLayout::generateDefaultVertexData(
    const QCText &text,
    const QFont &font,
    QCState *state,
    QCRhiDistanceFieldGlyphCache *cache,
    QCDistanceFieldGlyphCache::FontKeyData *data)
{
    Q_UNUSED(data);

    if (m_optimized)
        return;

    m_initialized = true;

    const auto runs = m_textLayout.glyphRuns();
    const auto metrics = QFontMetrics{font};

    qsizetype index = 0;

    const QRectF textRect = QRectF(text.x(), text.y(), text.width(), text.height());
    const float verticalAlignment = calculateVerticalAlignment(state->textBaseline, textRect, metrics, m_textLayout.boundingRect());
    const QPointF glyphPos(0, verticalAlignment);
    for (const auto &run : runs) {
        cache->addGlyphs(glyphPos, run);
        cache->update();

        QVarLengthArray<QCRhiDistanceFieldGlyphCache::TexturedPoint2D, 256> v{};
        QVarLengthArray<ushort, 384> i{};

        cache->generateVertices(&v, &i, QTransform(), &m_bounds);

        for (auto t : v)
            defaultVerts.push_back(t);

        for (auto t : i)
            defaultIndices.push_back(t + index);

        index += defaultVerts.size();
    }
}

void QCTextLayout::generateVertexData(QCRhiDistanceFieldGlyphCache *cache)
{
    m_initialized = true;

    if (!m_optimized)
        return;

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
