// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qclineargradient.h"
#include "qcgradient_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCLinearGradient
    \brief QCLinearGradient is a brush for linear gradient painting.
    \inmodule QtCanvasPainter

    Liner gradient interpolate colors between start and end points
    along a linear pattern, so horizontally, vertically or diagonally.
    The gradient starts from \a startPosition() and ends to \a endPosition().
    Outside this area, the colors are the starting and the ending color.

    Here is a simple example:
    \code
    QRectF rect1(50, 50, 200, 200);
    QCLinearGradient lg(rect1.topLeft(),
                        rect1.bottomRight());
    lg.setColorAt(0.0, "#1a2a6c");
    lg.setColorAt(0.5, "#b21f1f");
    lg.setColorAt(1.0, "#fdbb2d");
    painter.setFillStyle(lg);
    painter.fillRect(rect1);
    \endcode

    \image lineargradient_example_1.png
*/

#define G_D() auto *d = QCGradientPrivate::get(this)
#define DECONST(d) const_cast<QCLinearGradientPrivate *>(d)

class QCLinearGradientPrivate : public QCGradientPrivate
{
public:
    QCLinearGradientPrivate() : QCGradientPrivate(QCBrush::BrushType::LinearGradient) {}
    QCLinearGradientPrivate(const QCLinearGradientPrivate &) = default;
    QCPaint createPaint(QCPainter *painter) const override;
    void createLinearGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;

    QCBrushPrivate *clone() override
    {
        return new QCLinearGradientPrivate(*this);
    }
};


/*!
    Constructs a default linear gradient.
    Gradient start color position is (0, 0) and end color position (0, 100).
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCLinearGradient::QCLinearGradient()
    : QCGradient(new QCLinearGradientPrivate)
{
    G_D();
    d->data.linear.sx = 0.0f;
    d->data.linear.sy = 0.0f;
    d->data.linear.ex = 0.0f;
    d->data.linear.ey = 100.0f;
}

/*!
    Constructs a linear gradient.
    Gradient start color position is (\a startX, \a startY) and
    end color position (\a endX, \a endY).
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCLinearGradient::QCLinearGradient(float startX, float startY, float endX, float endY)
    : QCGradient(new QCLinearGradientPrivate)
{
    G_D();
    d->data.linear.sx = startX;
    d->data.linear.sy = startY;
    d->data.linear.ex = endX;
    d->data.linear.ey = endY;
}

/*!
    Constructs a linear gradient.
    Gradient start color position is \a start and end color position \a end.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCLinearGradient::QCLinearGradient(QPointF start, QPointF end)
    : QCGradient(new QCLinearGradientPrivate)
{
    G_D();
    d->data.linear.sx = float(start.x());
    d->data.linear.sy = float(start.y());
    d->data.linear.ex = float(end.x());
    d->data.linear.ey = float(end.y());
}

QCLinearGradient::~QCLinearGradient()
{
}

/*!
    Returns the start point of linear gradient.
*/

QPointF QCLinearGradient::startPosition() const
{
    G_D();
    return QPointF(d->data.linear.sx,
                   d->data.linear.sy);
}

/*!
    Sets the start point of linear gradient to ( \a x, \a y).
*/

void QCLinearGradient::setStartPosition(float x, float y)
{
    G_D();
    detach();
    d->data.linear.sx = x;
    d->data.linear.sy = y;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Sets the start point of linear gradient to \a start.
*/

void QCLinearGradient::setStartPosition(QPointF start)
{
    setStartPosition(float(start.x()),
                     float(start.y()));
}

/*!
    Returns the end point of linear gradient.
*/

QPointF QCLinearGradient::endPosition() const
{
    G_D();
    return QPointF(d->data.linear.ex,
                   d->data.linear.ey);
}

/*!
    Sets the end point of linear gradient to ( \a x, \a y).
*/

void QCLinearGradient::setEndPosition(float x, float y)
{
    G_D();
    detach();
    d->data.linear.ex = x;
    d->data.linear.ey = y;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Sets the end point of linear gradient to \a end.
*/

void QCLinearGradient::setEndPosition(QPointF end)
{
    setEndPosition(float(end.x()),
                   float(end.y()));
}

// ***** Private *****

/*!
   \internal
*/


QCPaint QCLinearGradientPrivate::createPaint(QCPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createLinearGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().second;
            createLinearGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().second;
            QColor oc = d->gradientStops.last().second;
            createLinearGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createLinearGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCLinearGradientPrivate::createLinearGradient(const QColor &iColor, const QColor &oColor,
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
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef G_D
#undef DECONST

QT_END_NAMESPACE
