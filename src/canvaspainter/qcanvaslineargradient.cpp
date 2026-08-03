// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvaslineargradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasLinearGradient
    \since 6.11
    \brief QCanvasLinearGradient is a brush for linear gradient painting.
    \inmodule QtCanvasPainter

    Liner gradient interpolate colors between start and end points
    along a linear pattern, so horizontally, vertically or diagonally.
    The gradient starts from \a startPosition() and ends to \a endPosition().
    Outside this area, the colors are the starting and the ending color.

    \image qclineargradient-gallery.webp
           {Four squares with linear gradients running vertically,
           horizontally, diagonally, and as horizontal color bands}

    Here is a simple example:
    \table
    \row
    \li \inlineimage qclineargradient-example.webp
        {Square with a diagonal linear gradient from blue to red to yellow}
    \li
    \code
    QRectF rect1(0, 0, 200, 200);
    QCanvasLinearGradient lg(rect1.topLeft(),
                        rect1.bottomRight());
    lg.setColorAt(0.0, "#1a2a6c");
    lg.setColorAt(0.5, "#b21f1f");
    lg.setColorAt(1.0, "#fdbb2d");
    p->setFillStyle(lg);
    p->fillRect(rect1);
    \endcode
    \endtable
*/

#define DECONST(d) const_cast<QCanvasLinearGradientBrushPrivate *>(d)

/*!
    Constructs a default linear gradient.
    Gradient start color position is (0, 0) and end color position (0, 100).
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasLinearGradient::QCanvasLinearGradient()
    : QCanvasGradient(QCanvasBrush::BrushType::LinearGradient)
{
    m_data.linear.sx = 0.0f;
    m_data.linear.sy = 0.0f;
    m_data.linear.ex = 0.0f;
    m_data.linear.ey = 100.0f;
}

/*!
    Constructs a linear gradient.
    Gradient start color position is (\a startX, \a startY) and
    end color position (\a endX, \a endY).
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasLinearGradient::QCanvasLinearGradient(float startX, float startY, float endX, float endY)
    : QCanvasGradient(QCanvasBrush::BrushType::LinearGradient)
{
    m_data.linear.sx = startX;
    m_data.linear.sy = startY;
    m_data.linear.ex = endX;
    m_data.linear.ey = endY;
}

/*!
    Constructs a linear gradient.
    Gradient start color position is \a start and end color position \a end.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasLinearGradient::QCanvasLinearGradient(QPointF start, QPointF end)
    : QCanvasGradient(QCanvasBrush::BrushType::LinearGradient)
{
    m_data.linear.sx = float(start.x());
    m_data.linear.sy = float(start.y());
    m_data.linear.ex = float(end.x());
    m_data.linear.ey = float(end.y());
}

/*!
    Returns the start point of linear gradient.
*/

QPointF QCanvasLinearGradient::startPosition() const
{
    return QPointF(m_data.linear.sx, m_data.linear.sy);
}

/*!
    Sets the start point of linear gradient to ( \a x, \a y).
*/

void QCanvasLinearGradient::setStartPosition(float x, float y)
{
    m_data.linear.sx = x;
    m_data.linear.sy = y;
    m_cachedBrush = {};
}

/*!
    \fn void QCanvasLinearGradient::setStartPosition(QPointF start)
    \overload

    Sets the start point of linear gradient to \a start.
*/

/*!
    Returns the end point of linear gradient.
*/

QPointF QCanvasLinearGradient::endPosition() const
{
    return QPointF(m_data.linear.ex, m_data.linear.ey);
}

/*!
    Sets the end point of linear gradient to ( \a x, \a y).
*/

void QCanvasLinearGradient::setEndPosition(float x, float y)
{
    m_data.linear.ex = x;
    m_data.linear.ey = y;
    m_cachedBrush = {};
}

/*!
    \fn void QCanvasLinearGradient::setEndPosition(QPointF end)
    \overload

    Sets the end point of linear gradient to \a end.
*/

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasLinearGradientBrushPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            // Note: Without stops, custom image might be used.
            createLinearGradient(icol, ocol, d->imageId);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createLinearGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createLinearGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createLinearGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->imageId > 0) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasLinearGradientBrushPrivate::createLinearGradient(const QColor &iColor, const QColor &oColor,
                                            int imageId) const
{
    auto *d = this;

    const auto dd = d->data.linear;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushLinearGradient;

    float dx = dd.ex - dd.sx;
    float dy = dd.ey - dd.sy;
    float dist = std::sqrt(dx*dx + dy*dy);
    constexpr float small = 0.0001f;
    if (dist > small) {
        dx /= dist;
        dy /= dist;
    } else {
        dx = 0;
        dy = 1;
    }
    p.transform.setMatrix(dy, -dx, 0,
                          dx, dy, 0,
                          dd.sx, dd.sy, 1);
    p.feather = qMax(small, dist);

    // Note: extent and radius not used.

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
        p.innerColor.a = d->imageY;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef DECONST

template<> QCanvasLinearGradient QCanvasBrush::as<QCanvasLinearGradient>() const
{
    Q_ASSERT(type() == BrushType::LinearGradient);
    const auto *gd = static_cast<const QCanvasGradientBrushPrivate *>(QCanvasBrushPrivate::get(*this));
    QCanvasLinearGradient g(gd->data.linear.sx, gd->data.linear.sy,
                             gd->data.linear.ex, gd->data.linear.ey);
    g.setStops(gd->gradientStops);
    return g;
}

QT_END_NAMESPACE
