// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasboxgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasBoxGradient
    \since 6.11
    \brief QCanvasBoxGradient is a brush for box gradient painting.
    \inmodule QtCanvasPainter

    Box gradient interpolate colors between start and end points, creating
    a rounded rectangle shaped gradient. With the \l setRadius() and
    \l setFeather() the roundness and softness of the gradient can be
    controlled.

    \image qcboxgradient-gallery.webp
           {Four squares with rounded-rectangle box gradients of varying
           color, corner radius, and edge softness}

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcboxgradient-example.webp
        {Dark blue rounded square with a soft yellow glow along its edges}
    \li
    \code
    QRectF rect(20, 20, 160, 160);
    QCanvasBoxGradient bg(rect);
    bg.setFeather(20);
    bg.setRadius(50);
    bg.setColorAt(0.0, "#1a2a6c");
    bg.setColorAt(0.2, "#fdbb2d");
    bg.setColorAt(1.0, QColorConstants::Transparent);
    p->setFillStyle(bg);
    p->fillRect(rect);
    \endcode
    \endtable

    \note When QCanvasBoxGradient is used to paint round rectangle, it is usually
    better to use \l QCanvasPainter::rect() than \l QCanvasPainter::roundRect() as the
    rounding comes from the brush rather than the path. This avoids creating
    extra vertices and thus performs better.
*/

#define DECONST(d) const_cast<QCanvasBoxGradientBrushPrivate *>(d)

/*!
    Constructs a default box gradient.
    Position of gradient is (0, 0) and size (100, 100)
    Gradient feather is 10.0.
    Gradient radius is 0.0.
*/

QCanvasBoxGradient::QCanvasBoxGradient()
    : QCanvasBoxGradient(0.0, 0.0, 100.0, 100.0)
{
}

/*!
    Constructs a box gradient.
    Position of gradient is ( \a x, \a y) and size ( \a width, \a height)
    Gradient feather is 10.0 and radius is 0.0.

    \sa setFeather(), setRadius()
*/

QCanvasBoxGradient::QCanvasBoxGradient(qreal x, qreal y, qreal width, qreal height)
    : QCanvasGradient(QCanvasBrush::BrushType::BoxGradient)
{
    auto &box = QCanvasGradientBrushPrivate::get(*this)->data.box;
    box.x = x;
    box.y = y;
    box.width = width;
    box.height = height;
    box.feather = 10.0;
    box.radius = 0.0;
}

/*!
    \fn QCanvasBoxGradient::QCanvasBoxGradient(const QRectF &rect)

    Constructs a box gradient.
    Position and size of gradient is \a rect.
    Gradient feather is 10.0 and radius is 0.0.

    \sa setFeather(), setRadius()
*/

/*!
    Returns the rectangle area of the box gradient.
    \sa setRect()
*/

QRectF QCanvasBoxGradient::rect() const
{
    const auto &box = QCanvasGradientBrushPrivate::get(*this)->data.box;
    return QRectF(box.x, box.y, box.width, box.height);
}

/*!
    Sets the rectangle of box gradient to position ( \a x, \a y) and size ( \a width, \a height).
*/

void QCanvasBoxGradient::setRect(qreal x, qreal y, qreal width, qreal height)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.box.x = x;
    d->data.box.y = y;
    d->data.box.width = width;
    d->data.box.height = height;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasBoxGradient::setRect(const QRectF &rect)
    \overload

    Sets the rectangle of box gradient to \a rect.
*/

/*!
    Returns the feather of the box gradient.
    \sa setFeather()
*/

qreal QCanvasBoxGradient::feather() const
{
    return QCanvasGradientBrushPrivate::get(*this)->data.box.feather;
}

/*!
    Sets the feather of box gradient to \a feather.
*/

void QCanvasBoxGradient::setFeather(qreal feather)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.box.feather = feather;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    Returns the radius of the box gradient.
    \sa setRadius()
*/

qreal QCanvasBoxGradient::radius() const
{
    return QCanvasGradientBrushPrivate::get(*this)->data.box.radius;
}

/*!
    Sets the radius of box gradient to \a radius.
    The maximum radius is half of width or height of rect(),
    depending on which one is smaller.
*/

void QCanvasBoxGradient::setRadius(qreal radius)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.box.radius = radius;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasBoxGradientBrushPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 0, 0, 0, 0 };
            QColor ocol = { 0, 0, 0, 0 };
            // Note: Without stops, custom image might be used.
            createBoxGradient(icol, ocol, d->imageId);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createBoxGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createBoxGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createBoxGradient(col, col, d->textureId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->paint.imageId > 0) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->paint.imageId);
    }
    return d->paint;
}

void QCanvasBoxGradientBrushPrivate::createBoxGradient(QColor iColor, QColor oColor,
                                      int imageId) const
{
    auto *d = this;
    const auto dd = d->data.box;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushBoxGradient;
    p.transform = QTransform::fromTranslate(dd.x + (dd.width * 0.5), dd.y + (dd.height * 0.5));

    p.extent[0] = float(dd.width * 0.5);
    p.extent[1] = float(dd.height * 0.5);

    p.radius = float(qMin(dd.radius, qMin(dd.width, dd.height) * 0.5));
    constexpr qreal small = 0.0001;
    p.feather = float(qMax(small, dd.feather));
    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
        p.innerColor.a = float(d->imageY);
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef DECONST

QT_END_NAMESPACE
