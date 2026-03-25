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

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcradialgradient-example.webp
    \li
    \code
    QRectF rect1(0, 0, 200, 200);
    QCanvasRadialGradient rg(rect1.center(), 100, 0);
    rg.setColorAt(0.0, "#fdbb2d");
    rg.setColorAt(0.6, "#b21f1f");
    rg.setColorAt(1.0, "#1a2a6c");
    p->setFillStyle(rg);
    p->fillRect(rect1);
    \endcode
    \endtable
*/

#define G_D() auto *d = QCanvasGradientPrivate::get(this)
#define DECONST(d) const_cast<QCanvasRadialGradientPrivate *>(d)

class QCanvasRadialGradientPrivate : public QCanvasGradientPrivate
{
public:
    QCanvasRadialGradientPrivate() : QCanvasGradientPrivate(QCanvasBrush::BrushType::RadialGradient) {}
    QCanvasRadialGradientPrivate(const QCanvasRadialGradientPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createRadialGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;
    QCanvasBrushPrivate *clone() override
    {
        return new QCanvasRadialGradientPrivate(*this);
    }
};

/*!
    Constructs a default radial gradient.
    Gradient center position is (0, 0).
    Gradient outer radius is 100.0 and inner radius is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient()
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.icx = 0.0f;
    d->data.radial.icy = 0.0f;
    d->data.radial.iRadius = 0.0f;
    d->data.radial.ocx = 0.0f;
    d->data.radial.ocy = 0.0f;
    d->data.radial.oRadius = 100.0f;
}

/*!
    Constructs a radial gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient(float centerX, float centerY, float outerRadius, float innerRadius)
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.icx = centerX;
    d->data.radial.icy = centerY;
    d->data.radial.iRadius = innerRadius;
    d->data.radial.ocx = centerX;
    d->data.radial.ocy = centerY;
    d->data.radial.oRadius = outerRadius;
}

/*!
    Constructs a radial gradient.
    Gradient center position is \a center.
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255,255,255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF center, float outerRadius, float innerRadius)
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.icx = float(center.x());
    d->data.radial.icy = float(center.y());
    d->data.radial.iRadius = innerRadius;
    d->data.radial.ocx = float(center.x());
    d->data.radial.ocy = float(center.y());
    d->data.radial.oRadius = outerRadius;
}

/*!
    \since 6.12
    Constructs a radial gradient.
    Gradient inner center position is (\a innerCenterX, \a innerCenterY).
    Gradient outer center position is ( \a outerCenterX, \a outerCenterY).
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient(float innerCenterX, float innerCenterY, float innerRadius, float outerCenterX, float outerCenterY, float outerRadius)
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.icx = innerCenterX;
    d->data.radial.icy = innerCenterY;
    d->data.radial.iRadius = innerRadius;
    d->data.radial.ocx = outerCenterX;
    d->data.radial.ocy = outerCenterY;
    d->data.radial.oRadius = outerRadius;
}

/*!
    \since 6.12
    Constructs a radial gradient.
    Gradient inner center position is \a innerCenter.
    Gradient outer center position is \a outerCenter.
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF innerCenter, float innerRadius, QPointF outerCenter, float outerRadius)
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.icx = float(innerCenter.x());
    d->data.radial.icy = float(innerCenter.y());
    d->data.radial.iRadius = innerRadius;
    d->data.radial.ocx = float(outerCenter.x());
    d->data.radial.ocy = float(outerCenter.y());
    d->data.radial.oRadius = outerRadius;
}

QCanvasRadialGradient::~QCanvasRadialGradient()
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

void QCanvasRadialGradient::setCenterPosition(float x, float y)
{
    G_D();
    detach();
    d->data.radial.icx = x;
    d->data.radial.icy = y;
    d->data.radial.ocx = x;
    d->data.radial.ocy = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \since 6.12
    Returns the inner center point of radial gradient.

    \sa setInnerCenterPosition()
*/

QPointF QCanvasRadialGradient::innerCenterPosition() const
{
    G_D();
    return QPointF(d->data.radial.icx,
                   d->data.radial.icy);
}

/*!
    \since 6.12
    Sets the inner center point of radial gradient to ( \a x, \a y).

    \sa innerCenterPosition()
*/

void QCanvasRadialGradient::setInnerCenterPosition(float x, float y)
{
    G_D();
    detach();
    d->data.radial.icx = x;
    d->data.radial.icy = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \since 6.12
    Returns the outer center point of radial gradient.

    \sa setOuterCenterPosition()
*/

QPointF QCanvasRadialGradient::outerCenterPosition() const
{
    G_D();
    return QPointF(d->data.radial.ocx,
                   d->data.radial.ocy);
}

/*!
    \since 6.12
    Sets the outer center point of radial gradient to ( \a x, \a y).

    \sa outerCenterPosition()
*/

void QCanvasRadialGradient::setOuterCenterPosition(float x, float y)
{
    G_D();
    detach();
    d->data.radial.ocx = x;
    d->data.radial.ocy = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
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

float QCanvasRadialGradient::outerRadius() const
{
    G_D();
    return d->data.radial.oRadius;
}

/*!
    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from outer center position.
*/

void QCanvasRadialGradient::setOuterRadius(float radius)
{
    G_D();
    detach();
    d->data.radial.oRadius = radius;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    Returns the inner radius of radial gradient.
    \sa setInnerRadius()
*/

float QCanvasRadialGradient::innerRadius() const
{
    G_D();
    return d->data.radial.iRadius;
}

/*!
    Sets the inner radius of radial gradient to \a radius.
    Start color will be drawn at this radius from inner center position.
    The default inner radius is \c 0.0 meaning that gradient starts
    directly from inner center position.
*/

void QCanvasRadialGradient::setInnerRadius(float radius)
{
    G_D();
    detach();
    d->data.radial.iRadius = radius;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasRadialGradientPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createRadialGradient(icol, ocol, 0);
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
            createRadialGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasRadialGradientPrivate::createRadialGradient(const QColor &iColor, const QColor &oColor,
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
        p.extent[0] = dd.icx;
        p.extent[1] = dd.icy;
        p.radius = dd.iRadius;
        // feather used for outer radius.
        p.feather = dd.oRadius;
    } else {
        p.brushType = BrushRadialGradient;
        constexpr float small = 0.0001f;
        const float r = (dd.iRadius + dd.oRadius) * 0.5f;
        const float f = (dd.oRadius - dd.iRadius);
        p.radius = r;
        p.feather = qMax(small, f);
    }

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
