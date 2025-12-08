// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcboxgradient.h"
#include "qcgradient_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCBoxGradient
    \brief QCBoxGradient is a brush for box gradient painting.
    \inmodule QtCanvasPainter

    Box gradient interpolate colors between start and end points, creating
    a rounded rectangle shaped gradient. With the \l setRadius() and
    \l setFeather() the roundness and softness of the gradient can be
    controlled.

    Here is a simple example:
    \code
    QRectF rect(50, 50, 200, 100);
    QCBoxGradient bg(rect, 20, 50);
    bg.setColorAt(0.0, Qt::darkBlue);
    bg.setColorAt(0.2, Qt::darkYellow);
    bg.setColorAt(1.0, Qt::transparent);
    painter.setFillStyle(bg);
    painter.fillRect(rect);
    \endcode

    \image boxgradient_example_1.png

    \note When QCBoxGradient is used to paint round rectangle, it is usually
    better to use \l QCPainter::rect() than \l QCPainter::roundRect() as the
    rounding comes from the brush rather than the path. This avoids creating
    extra vertices and thus performs better.
*/

/*!
    Constructs a default box gradient.
    Position of gradient is (0, 0) and size (100, 100)
    Gradient feather is 10.0.
    Gradient radius is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCBoxGradient::QCBoxGradient()
    : QCGradient(BrushType::BoxGradient)
{
    d->data.box.x = 0.0f;
    d->data.box.y = 0.0f;
    d->data.box.width = 100.0f;
    d->data.box.height = 100.0f;
    d->data.box.feather = 10.0f;
    d->data.box.radius = 0.0f;
}

/*!
    Constructs a box gradient.
    Position of gradient is ( \a x, \a y) and size ( \a width, \a height)
    Gradient feather is \a feather.
    Gradient radius is \a radius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCBoxGradient::QCBoxGradient(float x, float y, float width, float height, float feather, float radius)
    : QCGradient(BrushType::BoxGradient)
{
    d->data.box.x = x;
    d->data.box.y = y;
    d->data.box.width = width;
    d->data.box.height = height;
    d->data.box.feather = feather;
    d->data.box.radius = radius;
}

/*!
    Constructs a box gradient.
    Position and size of gradient is \a rect.
    Gradient feather is \a feather.
    Gradient radius is \a radius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCBoxGradient::QCBoxGradient(const QRectF &rect, float feather, float radius)
    : QCGradient(BrushType::BoxGradient)
{
    d->data.box.x = float(rect.x());
    d->data.box.y = float(rect.y());
    d->data.box.width = float(rect.width());
    d->data.box.height = float(rect.height());
    d->data.box.feather = feather;
    d->data.box.radius = radius;
}

QCBoxGradient::~QCBoxGradient()
{
}

/*!
    Returns the rectangle area of the box gradient.
    \sa setRect()
*/

QRectF QCBoxGradient::rect() const
{
    return QRectF(d->data.box.x,
                  d->data.box.y,
                  d->data.box.width,
                  d->data.box.height);
}
/*!
    Sets the rectangle of box gradient to position ( \a x, \a y) and size ( \a width, \a height).
*/

void QCBoxGradient::setRect(float x, float y, float width, float height)
{
    detach();
    d->data.box.x = x;
    d->data.box.y = y;
    d->data.box.width = width;
    d->data.box.height = height;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Sets the rectangle of box gradient to \a rect.
*/

void QCBoxGradient::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()),
            float(rect.width()), float(rect.height()));
}

/*!
    Returns the feather of the box gradient.
    \sa setFeather()
*/

float QCBoxGradient::feather() const
{
    return d->data.box.feather;
}

/*!
    Sets the feather of box gradient to \a feather.
*/

void QCBoxGradient::setFeather(float feather)
{
    detach();
    d->data.box.feather = feather;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Returns the radius of the box gradient.
    \sa setRadius()
*/

float QCBoxGradient::radius() const
{
    return d->data.box.radius;
}

/*!
    Sets the radius of box gradient to \a radius.
    The maximum radius is half of width or height of rect(),
    depending on which one is smaller.
*/

void QCBoxGradient::setRadius(float radius)
{
    detach();
    d->data.box.radius = radius;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCBoxGradient::createPaint(QCPainter *painter) const
{
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createBoxGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().second;
            createBoxGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().second;
            QColor oc = d->gradientStops.last().second;
            createBoxGradient(ic, oc, 0);
        } else {
            d->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createBoxGradient(col, col, d->imageId);
        }
        d->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCBoxGradient::createBoxGradient(const QColor &iColor, const QColor &oColor,
                                      int imageId) const
{
    const auto dd = d->data.box;
    QCPaint &p = d->paint;
    p.brushType = BrushBoxGradient;
    p.transform = QTransform::fromTranslate(dd.x + (dd.width * 0.5f), dd.y + (dd.height * 0.5f));

    p.extent[0] = dd.width * 0.5f;
    p.extent[1] = dd.height * 0.5f;

    p.radius = qMin(dd.radius, qMin(dd.width, dd.height) * 0.5f);
    constexpr float small = 0.0001f;
    p.feather = qMax(small, dd.feather);

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

QT_END_NAMESPACE
