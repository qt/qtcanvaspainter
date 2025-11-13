// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcconicalgradient.h"
#include "qcgradient_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCConicalGradient
    \brief QCConicalGradient is a brush for conical gradient painting.
    \inmodule QtCanvasPainter

    Conical gradient interpolate colors between start and end points
    around a point with given coordinates. The angle starts from a line
    going horizontally right from the center, and proceeds clockwise.

    Here is a simple example:
    \code
    QCConicalGradient cg(200, 200, 0);
    cg.setStartColor(Qt::blue);
    cg.setEndColor(Qt::darkYellow);
    painter.setFillStyle(cg);
    painter.fillRect(100, 100, 200, 200);
    \endcode

    \image conicalgradient_example_1.png

*/

/*!
    Constructs a default conical gradient.
    Gradient center position is (0, 0).
    Gradient angle is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCConicalGradient::QCConicalGradient()
    : QCGradient(BrushType::ConicalGradient)
{
    d->data.conical.cx = 0.0f;
    d->data.conical.cy = 0.0f;
    d->data.conical.angle = 0.0f;
}

/*!
    Constructs a conical gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient start angle is \a startAngle.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCConicalGradient::QCConicalGradient(float centerX, float centerY, float startAngle)
    : QCGradient(BrushType::ConicalGradient)
{
    d->data.conical.cx = centerX;
    d->data.conical.cy = centerY;
    d->data.conical.angle = startAngle;
}

/*!
    Constructs a conical gradient.
    Gradient center position is \a center.
    Gradient start angle is \a startAngle.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCConicalGradient::QCConicalGradient(const QPointF &center, float startAngle)
    : QCGradient(BrushType::ConicalGradient)
{
    d->data.conical.cx = float(center.x());
    d->data.conical.cy = float(center.y());
    d->data.conical.angle = startAngle;
}

QCConicalGradient::~QCConicalGradient()
{
}

/*!
    Returns the center point of conical gradient.
    \sa setCenterPosition()
*/

QPointF QCConicalGradient::centerPosition() const
{
    return QPointF(d->data.conical.cx,
                   d->data.conical.cy);
}

/*!
    Sets the center point of conical gradient to ( \a x, \a y).
*/
void QCConicalGradient::setCenterPosition(float x, float y)
{
    detach();
    d->data.conical.cx = x;
    d->data.conical.cy = y;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Sets the center point of conical gradient to \a center.
*/

void QCConicalGradient::setCenterPosition(const QPointF &center)
{
    setCenterPosition(float(center.x()),
                      float(center.y()));
}

/*!
    Returns the start angle of conical gradient in radians.
*/

float QCConicalGradient::angle() const
{
    return d->data.conical.angle;
}

/*!
    Sets the start angle of conical gradient to \a angle in radians.
    The angle starts from a line going horizontally right from the
    center, and proceeds clockwise.
*/

void QCConicalGradient::setAngle(float angle)
{
    detach();
    d->data.conical.angle = angle;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCConicalGradient::createPaint(QCPainter *painter) const
{
    auto *painterPriv = QCPainterPrivate::get(painter);
    if (d->dirty) {
        const float cx = d->data.conical.cx;
        const float cy = d->data.conical.cy;
        auto *e = painterPriv->engine();
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            d->paint = e->createConicalGradient(cx, cy, d->data.conical.angle, icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().second;
            d->paint = e->createConicalGradient(cx, cy, d->data.conical.angle, c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().second;
            QColor oc = d->gradientStops.last().second;
            d->paint = e->createConicalGradient(cx, cy, d->data.conical.angle, ic, oc, 0);
        } else {
            d->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            d->paint = e->createConicalGradient(cx, cy, d->data.conical.angle, col, col, d->imageId);
        }
        d->dirty = {};
    }
    if (d->gradientStops.size() > 2)
        painterPriv->markTextureIdUsed(d->imageId);
    return d->paint;
}

QT_END_NAMESPACE
