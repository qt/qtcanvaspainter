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
    : QCanvasGradient(QCanvasBrush::BrushType::RadialGradient)
{
    m_data.radial.icx = 0.0f;
    m_data.radial.icy = 0.0f;
    m_data.radial.iRadius = 0.0f;
    m_data.radial.ocx = 0.0f;
    m_data.radial.ocy = 0.0f;
    m_data.radial.oRadius = 100.0f;
}

/*!
    Constructs a radial gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(float centerX, float centerY,
                                             float innerRadius, float outerRadius)
    : QCanvasGradient(QCanvasBrush::BrushType::RadialGradient)
{
    m_data.radial.icx = centerX;
    m_data.radial.icy = centerY;
    m_data.radial.iRadius = innerRadius;
    m_data.radial.ocx = centerX;
    m_data.radial.ocy = centerY;
    m_data.radial.oRadius = outerRadius;
}

/*!
    Constructs a radial gradient.
    Gradient center position is \a centerPosition.
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF centerPosition,
                                             float innerRadius, float outerRadius)
    : QCanvasGradient(QCanvasBrush::BrushType::RadialGradient)
{
    m_data.radial.icx = float(centerPosition.x());
    m_data.radial.icy = float(centerPosition.y());
    m_data.radial.iRadius = innerRadius;
    m_data.radial.ocx = float(centerPosition.x());
    m_data.radial.ocy = float(centerPosition.y());
    m_data.radial.oRadius = outerRadius;
}

/*!
    \since 6.12
    Constructs a radial gradient.
    Gradient inner center position is (\a innerCenterX, \a innerCenterY).
    Gradient outer center position is ( \a outerCenterX, \a outerCenterY).
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(float innerCenterX, float innerCenterY, float innerRadius,
                                             float outerCenterX, float outerCenterY, float outerRadius)
    : QCanvasGradient(QCanvasBrush::BrushType::RadialGradient)
{
    m_data.radial.icx = innerCenterX;
    m_data.radial.icy = innerCenterY;
    m_data.radial.iRadius = innerRadius;
    m_data.radial.ocx = outerCenterX;
    m_data.radial.ocy = outerCenterY;
    m_data.radial.oRadius = outerRadius;
}

/*!
    \since 6.12
    Constructs a radial gradient.
    Gradient inner center position is \a innerCenterPosition.
    Gradient outer center position is \a outerCenterPosition.
    Gradient inner radius is \a innerRadius and outer radius is \a outerRadius.
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF innerCenterPosition, float innerRadius,
                                             QPointF outerCenterPosition, float outerRadius)
    : QCanvasGradient(QCanvasBrush::BrushType::RadialGradient)
{
    m_data.radial.icx = float(innerCenterPosition.x());
    m_data.radial.icy = float(innerCenterPosition.y());
    m_data.radial.iRadius = innerRadius;
    m_data.radial.ocx = float(outerCenterPosition.x());
    m_data.radial.ocy = float(outerCenterPosition.y());
    m_data.radial.oRadius = outerRadius;
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
    m_data.radial.icx = x;
    m_data.radial.icy = y;
    m_data.radial.ocx = x;
    m_data.radial.ocy = y;
    m_cachedBrush = {};
}

/*!
    \since 6.12
    Returns the inner center point of radial gradient.

    \sa setInnerCenterPosition()
*/

QPointF QCanvasRadialGradient::innerCenterPosition() const
{
    return QPointF(m_data.radial.icx, m_data.radial.icy);
}

/*!
    \since 6.12
    Sets the inner center point of radial gradient to ( \a x, \a y).

    \sa innerCenterPosition()
*/

void QCanvasRadialGradient::setInnerCenterPosition(float x, float y)
{
    m_data.radial.icx = x;
    m_data.radial.icy = y;
    m_cachedBrush = {};
}

/*!
    \since 6.12
    Returns the outer center point of radial gradient.

    \sa setOuterCenterPosition()
*/

QPointF QCanvasRadialGradient::outerCenterPosition() const
{
    return QPointF(m_data.radial.ocx, m_data.radial.ocy);
}

/*!
    \since 6.12
    Sets the outer center point of radial gradient to ( \a x, \a y).

    \sa outerCenterPosition()
*/

void QCanvasRadialGradient::setOuterCenterPosition(float x, float y)
{
    m_data.radial.ocx = x;
    m_data.radial.ocy = y;
    m_cachedBrush = {};
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
    return m_data.radial.oRadius;
}

/*!
    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from outer center position.
*/

void QCanvasRadialGradient::setOuterRadius(float radius)
{
    m_data.radial.oRadius = radius;
    m_cachedBrush = {};
}

/*!
    Returns the inner radius of radial gradient.
    \sa setInnerRadius()
*/

float QCanvasRadialGradient::innerRadius() const
{
    return m_data.radial.iRadius;
}

/*!
    Sets the inner radius of radial gradient to \a radius.
    Start color will be drawn at this radius from inner center position.
    The default inner radius is \c 0.0 meaning that gradient starts
    directly from inner center position.
*/

void QCanvasRadialGradient::setInnerRadius(float radius)
{
    m_data.radial.iRadius = radius;
    m_cachedBrush = {};
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
            createRadialGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->imageId > 0) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
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
        p.innerColor.a = d->imageY;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef DECONST

template<> QCanvasRadialGradient QCanvasBrush::as<QCanvasRadialGradient>() const
{
    Q_ASSERT(type() == BrushType::RadialGradient);
    const auto *gd = static_cast<const QCanvasGradientBrushPrivate *>(QCanvasBrushPrivate::get(*this));
    QCanvasRadialGradient g(gd->data.radial.icx, gd->data.radial.icy, gd->data.radial.iRadius,
                             gd->data.radial.ocx, gd->data.radial.ocy, gd->data.radial.oRadius);
    g.setStops(gd->gradientStops);
    return g;
}

QT_END_NAMESPACE
