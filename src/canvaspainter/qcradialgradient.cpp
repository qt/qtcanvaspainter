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
    \since 6.11
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

#define G_D() auto *d = QCGradientPrivate::get(this)
#define DECONST(d) const_cast<QCRadialGradientPrivate *>(d)

class QCRadialGradientPrivate : public QCGradientPrivate
{
public:
    QCRadialGradientPrivate() : QCGradientPrivate(QCBrush::BrushType::RadialGradient) {}
    QCRadialGradientPrivate(const QCRadialGradientPrivate &) = default;
    QCPaint createPaint(QCPainter *painter) const override;
    void createRadialGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;
    QCBrushPrivate *clone() override
    {
        return new QCRadialGradientPrivate(*this);
    }
};

/*!
    Constructs a default radial gradient.
    Gradient center position is (0, 0).
    Gradient outer radius is 100.0 and inner radius is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCRadialGradient::QCRadialGradient()
    : QCGradient(new QCRadialGradientPrivate)
{
    G_D();
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
    : QCGradient(new QCRadialGradientPrivate)
{
    G_D();
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

QCRadialGradient::QCRadialGradient(QPointF center, float outerRadius, float innerRadius)
    : QCGradient(new QCRadialGradientPrivate)
{
    G_D();
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
    G_D();
    return QPointF(d->data.radial.cx,
                   d->data.radial.cy);
}

/*!
    Sets the center point of radial gradient to ( \a x, \a y).
*/

void QCRadialGradient::setCenterPosition(float x, float y)
{
    G_D();
    detach();
    d->data.radial.cx = x;
    d->data.radial.cy = y;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

/*!
    Sets the center point of radial gradient to \a center.
*/

void QCRadialGradient::setCenterPosition(QPointF center)
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
    G_D();
    return d->data.radial.oRadius;
}

/*!
    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from center position.
*/

void QCRadialGradient::setOuterRadius(float radius)
{
    G_D();
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
    G_D();
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
    G_D();
    detach();
    d->data.radial.iRadius = radius;
    d->dirty |= QCGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCRadialGradientPrivate::createPaint(QCPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createRadialGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().second;
            createRadialGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().second;
            QColor oc = d->gradientStops.last().second;
            createRadialGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createRadialGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCRadialGradientPrivate::createRadialGradient(const QColor &iColor, const QColor &oColor,
                                               int imageId) const
{
    auto *d = this;
    const auto dd = d->data.radial;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushRadialGradient;
    const float r = (dd.iRadius + dd.oRadius) * 0.5f;
    const float f = (dd.oRadius - dd.iRadius);
    p.transform = QTransform::fromTranslate(dd.cx, dd.cy);

    // Note: extent not used.

    constexpr float small = 0.0001f;
    p.radius = r;
    p.feather = qMax(small, f);

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
