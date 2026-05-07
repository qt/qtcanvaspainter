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

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcboxgradient-example.webp
    \li
    \code
    QRectF rect(20, 20, 160, 160);
    QCanvasBoxGradient bg(rect, 20, 50);
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
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasBoxGradient::QCanvasBoxGradient()
    : QCanvasGradient(QCanvasBrush::BrushType::BoxGradient)
{
    m_data.box.x = 0.0f;
    m_data.box.y = 0.0f;
    m_data.box.width = 100.0f;
    m_data.box.height = 100.0f;
    m_data.box.feather = 10.0f;
    m_data.box.radius = 0.0f;
}

/*!
    Constructs a box gradient.
    Position of gradient is ( \a x, \a y) and size ( \a width, \a height)
    Gradient feather is \a feather.
    Gradient radius is \a radius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasBoxGradient::QCanvasBoxGradient(float x, float y, float width, float height, float feather, float radius)
    : QCanvasGradient(QCanvasBrush::BrushType::BoxGradient)
{
    m_data.box.x = x;
    m_data.box.y = y;
    m_data.box.width = width;
    m_data.box.height = height;
    m_data.box.feather = feather;
    m_data.box.radius = radius;
}

/*!
    Constructs a box gradient.
    Position and size of gradient is \a rect.
    Gradient feather is \a feather.
    Gradient radius is \a radius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasBoxGradient::QCanvasBoxGradient(const QRectF &rect, float feather, float radius)
    : QCanvasGradient(QCanvasBrush::BrushType::BoxGradient)
{
    m_data.box.x = float(rect.x());
    m_data.box.y = float(rect.y());
    m_data.box.width = float(rect.width());
    m_data.box.height = float(rect.height());
    m_data.box.feather = feather;
    m_data.box.radius = radius;
}

QCanvasBoxGradient::~QCanvasBoxGradient()
{
}

/*!
    Returns the rectangle area of the box gradient.
    \sa setRect()
*/

QRectF QCanvasBoxGradient::rect() const
{
    return QRectF(m_data.box.x, m_data.box.y, m_data.box.width, m_data.box.height);
}

/*!
    Sets the rectangle of box gradient to position ( \a x, \a y) and size ( \a width, \a height).
*/

void QCanvasBoxGradient::setRect(float x, float y, float width, float height)
{
    m_data.box.x = x;
    m_data.box.y = y;
    m_data.box.width = width;
    m_data.box.height = height;
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

float QCanvasBoxGradient::feather() const
{
    return m_data.box.feather;
}

/*!
    Sets the feather of box gradient to \a feather.
*/

void QCanvasBoxGradient::setFeather(float feather)
{
    m_data.box.feather = feather;
}

/*!
    Returns the radius of the box gradient.
    \sa setRadius()
*/

float QCanvasBoxGradient::radius() const
{
    return m_data.box.radius;
}

/*!
    Sets the radius of box gradient to \a radius.
    The maximum radius is half of width or height of rect(),
    depending on which one is smaller.
*/

void QCanvasBoxGradient::setRadius(float radius)
{
    m_data.box.radius = radius;
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
            QColor icol = { 255, 255, 255, 255 };
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
            createBoxGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->imageId > 0) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasBoxGradientBrushPrivate::createBoxGradient(const QColor &iColor, const QColor &oColor,
                                      int imageId) const
{
    auto *d = this;
    const auto dd = d->data.box;
    QCPaint &p = DECONST(d)->paint;
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
        p.innerColor.a = d->imageY;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef DECONST

template<> QCanvasBoxGradient QCanvasBrush::as<QCanvasBoxGradient>() const
{
    Q_ASSERT(type() == BrushType::BoxGradient);
    const auto *gd = static_cast<const QCanvasGradientBrushPrivate *>(QCanvasBrushPrivate::get(*this));
    QCanvasBoxGradient g(gd->data.box.x, gd->data.box.y,
                         gd->data.box.width, gd->data.box.height,
                         gd->data.box.feather, gd->data.box.radius);
    g.setStops(gd->gradientStops);
    return g;
}

QT_END_NAMESPACE
