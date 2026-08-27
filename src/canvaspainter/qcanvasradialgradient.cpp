// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasradialgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasRadialGradient
    \since 6.11
    \brief QCanvasRadialGradient is a brush for radial gradient painting.
    \inmodule QtCanvasPainter

    Radial gradient interpolate colors between start and end points, creating
    a circle shaped gradient. The gradient is controlled with \l centerPosition(),
    \l outerRadius() and \l innerRadius().

    \image qcradialgradient-gallery.webp
           {Four squares with radial gradients radiating from a center point,
           varying in color and center offset}

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcradialgradient-example.webp
        {Square with a radial gradient from a yellow center through red to a
        blue edge}
    \li
    \code
    QRectF rect1(0, 0, 200, 200);
    QCanvasRadialGradient rg(rect1.center(), 0, 100);
    rg.setColorAt(0.0, "#fdbb2d");
    rg.setColorAt(0.6, "#b21f1f");
    rg.setColorAt(1.0, "#1a2a6c");
    p->setFillStyle(rg);
    p->fillRect(rect1);
    \endcode
    \endtable
*/

#define DECONST(d) const_cast<QCanvasRadialGradientBrushPrivate *>(d)

/*!
    Constructs a default radial gradient.
    Gradient center position is (0, 0).
    Gradient outer radius is 100.0 and inner radius is 0.0.
*/

QCanvasRadialGradient::QCanvasRadialGradient()
    : QCanvasRadialGradient(0.0, 0.0, 0.0, 0.0, 0.0, 100.0)
{
}

/*!
    Constructs a radial gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(qreal centerX, qreal centerY,
                                             qreal innerRadius, qreal outerRadius)
    : QCanvasRadialGradient(centerX, centerY, innerRadius, centerX, centerY, outerRadius)
{
}

/*!
    Constructs a radial gradient.
    Gradient center position is \a centerPosition.
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF centerPosition,
                                             qreal innerRadius, qreal outerRadius)
    : QCanvasRadialGradient(centerPosition.x(), centerPosition.y(),
                            innerRadius, outerRadius)
{
}

/*!
    \since 6.12
    Constructs a radial gradient.
    Gradient inner center position is (\a innerCenterX, \a innerCenterY).
    Gradient outer center position is ( \a outerCenterX, \a outerCenterY).
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(qreal innerCenterX, qreal innerCenterY, qreal innerRadius,
                                             qreal outerCenterX, qreal outerCenterY, qreal outerRadius)
    : QCanvasGradient(QCanvasBrush::BrushType::RadialGradient)
{
    auto &radial = QCanvasGradientBrushPrivate::get(*this)->data.radial;
    radial.icx = innerCenterX;
    radial.icy = innerCenterY;
    radial.iRadius = innerRadius;
    radial.ocx = outerCenterX;
    radial.ocy = outerCenterY;
    radial.oRadius = outerRadius;
}

/*!
    \since 6.12
    Constructs a radial gradient.
    Gradient inner center position is \a innerCenterPosition.
    Gradient outer center position is \a outerCenterPosition.
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF innerCenterPosition, qreal innerRadius,
                                             QPointF outerCenterPosition, qreal outerRadius)
    : QCanvasRadialGradient(innerCenterPosition.x(), innerCenterPosition.y(), innerRadius,
                            outerCenterPosition.x(), outerCenterPosition.y(), outerRadius)
{
}

/*!
    Returns the center point of radial gradient.
    This is the same as \l outerCenterPosition().

    \sa setCenterPosition()
*/

QPointF QCanvasRadialGradient::centerPosition() const
{
    return outerCenterPosition();
}

/*!
    Sets the both center points of radial gradient to ( \a x, \a y).
    So after calling this, the gradient is symmetric (inner and outer
    positions are the same).

    \sa centerPosition()
*/

void QCanvasRadialGradient::setCenterPosition(qreal x, qreal y)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.radial.icx = x;
    d->data.radial.icy = y;
    d->data.radial.ocx = x;
    d->data.radial.ocy = y;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    \since 6.12
    Returns the inner center point of radial gradient.

    \sa setInnerCenterPosition()
*/

QPointF QCanvasRadialGradient::innerCenterPosition() const
{
    const auto &radial = QCanvasGradientBrushPrivate::get(*this)->data.radial;
    return QPointF(radial.icx, radial.icy);
}

/*!
    \since 6.12
    Sets the inner center point of radial gradient to ( \a x, \a y).

    \sa innerCenterPosition()
*/

void QCanvasRadialGradient::setInnerCenterPosition(qreal x, qreal y)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.radial.icx = x;
    d->data.radial.icy = y;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    \since 6.12
    Returns the outer center point of radial gradient.

    \sa setOuterCenterPosition()
*/

QPointF QCanvasRadialGradient::outerCenterPosition() const
{
    const auto &radial = QCanvasGradientBrushPrivate::get(*this)->data.radial;
    return QPointF(radial.ocx, radial.ocy);
}

/*!
    \since 6.12
    Sets the outer center point of radial gradient to ( \a x, \a y).

    \sa outerCenterPosition()
*/

void QCanvasRadialGradient::setOuterCenterPosition(qreal x, qreal y)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.radial.ocx = x;
    d->data.radial.ocy = y;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasRadialGradient::setCenterPosition(QPointF center)
    \overload

    Sets the both center points of radial gradient to \a center.
    So after calling this, the gradient is symmetric (inner and outer
    positions are the same).
*/

/*!
    \fn void QCanvasRadialGradient::setInnerCenterPosition(QPointF center)
    \overload
    \since 6.12

    Sets the inner center point of radial gradient to \a center.
*/

/*!
    \fn void QCanvasRadialGradient::setOuterCenterPosition(QPointF center)
    \overload
    \since 6.12

    Sets the outer center point of radial gradient to \a center.
*/

/*!
    Returns the outer radius of radial gradient.
    \sa setOuterRadius()
*/

qreal QCanvasRadialGradient::outerRadius() const
{
    return QCanvasGradientBrushPrivate::get(*this)->data.radial.oRadius;
}

/*!
    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from outer center position.
*/

void QCanvasRadialGradient::setOuterRadius(qreal radius)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.radial.oRadius = radius;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    Returns the inner radius of radial gradient.
    \sa setInnerRadius()
*/

qreal QCanvasRadialGradient::innerRadius() const
{
    return QCanvasGradientBrushPrivate::get(*this)->data.radial.iRadius;
}

/*!
    Sets the inner radius of radial gradient to \a radius.
    Start color will be drawn at this radius from inner center position.
    The default inner radius is \c 0.0 meaning that gradient starts
    directly from inner center position.
*/

void QCanvasRadialGradient::setInnerRadius(qreal radius)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.radial.iRadius = radius;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasRadialGradientBrushPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 0, 0, 0, 0 };
            QColor ocol = { 0, 0, 0, 0 };
            // Note: Without stops, custom image might be used.
            createRadialGradient(icol, ocol, d->imageId);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createRadialGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createRadialGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createRadialGradient(col, col, d->textureId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->paint.imageId > 0) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->paint.imageId);
    }
    return d->paint;
}

void QCanvasRadialGradientBrushPrivate::createRadialGradient(const QColor &iColor, const QColor &oColor,
                                               int imageId) const
{
    auto *d = this;
    const auto dd = d->data.radial;
    QCPaint &p = DECONST(d)->paint;
    p.transform = QTransform::fromTranslate(dd.ocx, dd.ocy);

    if (!qFuzzyCompare(dd.icx, dd.ocx) || !qFuzzyCompare(dd.icy, dd.ocy)) {
        // Inner and outer points are not the same, so use more complex
        // extended type.
        p.brushType = BrushRadialGradientExtended;
        // extent used for inner center position.
        p.extent[0] = float(dd.icx);
        p.extent[1] = float(dd.icy);
        p.radius = float(dd.iRadius);
        // feather used for outer radius.
        p.feather = float(dd.oRadius);
    } else {
        p.brushType = BrushRadialGradient;
        constexpr qreal small = 0.0001;
        const qreal r = (dd.iRadius + dd.oRadius) * 0.5;
        const qreal f = (dd.oRadius - dd.iRadius);
        p.radius = float(r);
        p.feather = float(qMax(small, f));
    }

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
