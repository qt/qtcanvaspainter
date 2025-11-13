// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcradialgradient.h"
#include "qcgradient_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCRadialGradient
    \brief QCRadialGradient is a brush for radial gradient painting.
    \inmodule QtCanvasPainter

    Radial gradient interpolate colors between start and end points, creating
    a circle shaped gradient. The gradient is controlled with \l centerPosition(),
    \l outerRadius() and \l innerRadius().

    Here is a simple example:
    \code
    QRectF rect1(50, 50, 200, 200);
    QCRadialGradient rg(rect1.center(), 100, 50);
    rg.setStartColor("#fdbb2d");
    rg.setEndColor("#b21f1f");
    painter.setFillStyle(rg);
    painter.beginPath();
    painter.ellipse(rect1);
    painter.fill();
    \endcode

*/

// TODO:     \image radialgradient_example_1.png

/*!
    Constructs a default radial gradient.
    Gradient center position is (0, 0).
    Gradient outer radius is 100.0 and inner radius is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCRadialGradient::QCRadialGradient()
    : QCGradient(BrushType::RadialGradient)
{
    d->data.radial.cx = 0.0f;
    d->data.radial.cy = 0.0f;
    d->data.radial.oRadius = 100.0f;
    d->data.radial.iRadius = 0.0f;
}

/*!
    Constructs a radial gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCRadialGradient::QCRadialGradient(float centerX, float centerY, float outerRadius, float innerRadius)
    : QCGradient(BrushType::RadialGradient)
{
    d->data.radial.cx = centerX;
    d->data.radial.cy = centerY;
    d->data.radial.oRadius = outerRadius;
    d->data.radial.iRadius = innerRadius;
}

/*!
    Constructs a radial gradient.
    Gradient center position is \a center.
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255,255,255) and end color
    transparent black (0, 0, 0, 0).
*/

QCRadialGradient::QCRadialGradient(const QPointF &center, float outerRadius, float innerRadius)
    : QCGradient(BrushType::RadialGradient)
{
    d->data.radial.cx = float(center.x());
    d->data.radial.cy = float(center.y());
    d->data.radial.oRadius = outerRadius;
    d->data.radial.iRadius = innerRadius;
}

QCRadialGradient::~QCRadialGradient()
{
}

/*!
    Returns the center point of radial gradient.
*/

QPointF QCRadialGradient::centerPosition() const
{
    return QPointF(d->data.radial.cx,
                   d->data.radial.cy);
}

/*!
    Sets the center point of radial gradient to ( \a x, \a y).
*/

void QCRadialGradient::setCenterPosition(float x, float y)
{
    detach();
    d->data.radial.cx = x;
    d->data.radial.cy = y;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Sets the center point of radial gradient to \a center.
*/

void QCRadialGradient::setCenterPosition(const QPointF &center)
{
    setCenterPosition(float(center.x()),
                      float(center.y()));
}

/*!
    Returns the outer radius of radial gradient.
    \sa setOuterRadius()
*/

float QCRadialGradient::outerRadius() const
{
    return d->data.radial.oRadius;
}

/*!
    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from center position.
*/

void QCRadialGradient::setOuterRadius(float radius)
{
    detach();
    d->data.radial.oRadius = radius;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Returns the inner radius of radial gradient.
    \sa setInnerRadius()
*/

float QCRadialGradient::innerRadius() const
{
    return d->data.radial.iRadius;
}

/*!
    Sets the inner radius of radial gradient to \a radius.
    Start color will be drawn at this radius from center position.
    The default inner radius is \c 0.0 meaning that gradient starts
    directly from center position.
*/

void QCRadialGradient::setInnerRadius(float radius)
{
    detach();
    d->data.radial.iRadius = radius;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCRadialGradient::createPaint(QCPainter *painter) const
{
    if (d->dirty) {
        const float cx = d->data.radial.cx;
        const float cy = d->data.radial.cy;
        auto *e = QCPainterPrivate::get(painter)->engine();
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            d->paint = e->createRadialGradient(cx, cy, d->data.radial.iRadius, d->data.radial.oRadius, icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().second;
            d->paint = e->createRadialGradient(cx, cy, d->data.radial.iRadius, d->data.radial.oRadius, c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().second;
            QColor oc = d->gradientStops.last().second;
            d->paint = e->createRadialGradient(cx, cy, d->data.radial.iRadius, d->data.radial.oRadius, ic, oc, 0);
        } else {
            d->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            d->paint = e->createRadialGradient(cx, cy, d->data.radial.iRadius, d->data.radial.oRadius, col, col, d->imageId);
        }
        d->dirty = {};
    }
    if (d->gradientStops.size() > 2)
        QCPainterPrivate::get(painter)->markTextureIdUsed(d->imageId);
    return d->paint;
}

QT_END_NAMESPACE
