// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcgradient.h"
#include "qcgradient_p.h"
#include "qclineargradient.h"
#include "qcradialgradient.h"
#include "qcconicalgradient.h"
#include "qcboxgradient.h"
#include "qcpainter.h"
#include "qcpainter_p.h"
#include "qdatastream.h"
#include "qvariant.h"
#include <qdebug.h>
#include <limits>
#include <QtGui/private/qpixellayout_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCGradient
    \since 6.11
    \brief QCGradient is the base class for all QCPainter gradient brushes.
    \inmodule QtCanvasPainter

    QCGradient is the base class for all QCPainter gradient brushes.
    Currently there are four type of gradients: \l QCLinearGradient,
    \l QCRadialGradient, \l QCConicalGradient and \l QCBoxGradient.

    If no stops are provided with \l setStartColor, \l setEndColor, \l setColorAt or \l setStops,
    the gragient is rendered as start color white (255,255,255) and end color transparent (0,0,0,0).
    If only a single stop is provided, the gradient is filled with this color.

    QCPainter uses two different approaches for painting gradients.
    \list
    \li With maximum of 2 stops, the colors are passed into shader as 2 vec4 uniforms.
        This makes animating 2 stop gradients very performant, with no extra steps in between.
    \li With more than 2 stops, the colors are passed into shader as a one dimensional texture.
        By default the texture size is 1x256 pixels, optimized for quality and performance.
        The textures are cached, so when gradient stops or colors don't change, previous texture
        can be reused even when other gradient properties (like position, angle etc.) change.
        Although animating multi-stop gradients is fast, it will cause texture uploads, so
        consider if it is worth it.
    \endlist

    There are few environment variables to control the gradients texture usage:
    \list
    \li QCPAINTER_DISABLE_TEXTURE_USAGE_TRACKING - By default, gradient texture usage is
        tracked and kept under the max amount. Set this environment variable to disable
        the tracking and keep all gradient textures in memory.
    \li QCPAINTER_MAX_TEXTURES - By default, the maximum amount of textures is \c 1024.
        Set this environt variable to contain number of desired maximum texture amount.
        The currently unused temporary gradient textures are automatically removed when
        the maximum amount is reached. \note This does not have an effect when the
        texture usage tracking has been disabled.
    \endlist

*/

// Pixel amount of textured gradients
#ifndef QCPAINTER_GRADIENT_SIZE
#define QCPAINTER_GRADIENT_SIZE 256
#endif

#ifndef QCPAINTER_GRADIENT_MAX_STOPS
#define QCPAINTER_GRADIENT_MAX_STOPS 16
#endif


QCGradient::QCGradient(QCGradientPrivate *priv)
    : QCBrush(priv)
{
}


/*!
   Returns the gradient as a \l QVariant.
*/

QCGradient::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCGradient::operator!=(const QCGradient &gradient) const

    Returns \c true if the gradient is different from the given \a gradient;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCGradient::operator==(const QCGradient &gradient) const

    Returns \c true if the gradient is equal to the given \a gradient; otherwise
    false.

    \sa operator!=()
*/

#define G_D() auto *d = QCGradientPrivate::get(this)

bool QCGradient::operator==(const QCGradient &g) const
{
    G_D();
    auto *gd = QCGradientPrivate::get(&g);
    if (gd == d)
        return true;
    if (gd->type != d->type)
        return false;
    if (d->type == QCBrush::BrushType::LinearGradient) {
        if (d->data.linear.sx != gd->data.linear.sx
            || d->data.linear.sy != gd->data.linear.sy
            || d->data.linear.ex != gd->data.linear.ex
            || d->data.linear.ey != gd->data.linear.ey)
            return false;
    } else if (d->type == QCBrush::BrushType::RadialGradient) {
        if (d->data.radial.cx != gd->data.radial.cx
            || d->data.radial.cy != gd->data.radial.cy
            || d->data.radial.oRadius != gd->data.radial.oRadius
            || d->data.radial.iRadius != gd->data.radial.iRadius)
            return false;
    } else if (d->type == QCBrush::BrushType::ConicalGradient) {
        if (d->data.conical.cx != gd->data.conical.cx
            || d->data.conical.cy != gd->data.conical.cy
            || d->data.conical.angle != gd->data.conical.angle)
            return false;
    } else if (d->type == QCBrush::BrushType::BoxGradient) {
        if (d->data.box.x != gd->data.box.x
            || d->data.box.y != gd->data.box.y
            || d->data.box.width != gd->data.box.width
            || d->data.box.height != gd->data.box.height
            || d->data.box.feather != gd->data.box.feather
            || d->data.box.radius != gd->data.box.radius)
            return false;
    }
    return gd->gradientStops == d->gradientStops;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCGradient &g)
{
    QDebugStateSaver saver(dbg);
    const auto t = g.type();
    if (t == QCBrush::BrushType::LinearGradient)
        dbg.nospace() << "QCLinearGradient(" << g.stops() << ')';
    else if (t == QCBrush::BrushType::RadialGradient)
        dbg.nospace() << "QCRadialGradient(" << g.stops() << ')';
    else if (t == QCBrush::BrushType::ConicalGradient)
        dbg.nospace() << "QCConicalGradient(" << g.stops() << ')';
    else if (t == QCBrush::BrushType::BoxGradient)
        dbg.nospace() << "QCBoxGradient(" << g.stops() << ')';
    else // QCGradient
        dbg.nospace() << "QCGradient(" << g.stops() << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCGradient stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCGradient &gradient)
    \relates QCGradient

    Writes the given \a gradient to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCGradient &g)
{
    s << g.type();
    s << g.stops();
    if (g.type() == QCBrush::BrushType::LinearGradient) {
        const auto lg = static_cast<const QCLinearGradient *>(&g);
        const auto &sp = lg->startPosition();
        const auto &ep = lg->endPosition();
        s << sp.x() << sp.y() << ep.x() << ep.y();
    } else if (g.type() == QCBrush::BrushType::RadialGradient) {
        const auto rg = static_cast<const QCRadialGradient *>(&g);
        const auto &cp = rg->centerPosition();
        s << cp.x() << cp.y();
        s << rg->outerRadius();
        s << rg->innerRadius();
    } else if (g.type() == QCBrush::BrushType::ConicalGradient) {
        const auto cg = static_cast<const QCConicalGradient *>(&g);
        const auto &cp = cg->centerPosition();
        s << cp.x() << cp.y();
        s << cg->angle();
    } else if (g.type() == QCBrush::BrushType::BoxGradient) {
        const auto bg = static_cast<const QCBoxGradient *>(&g);
        const auto &r = bg->rect();
        s << r.x() << r.y() << r.width() << r.height();
        s << bg->feather();
        s << bg->radius();
    }
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCGradient &gradient)
    \relates QCGradient

    Reads the given \a gradient from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCGradient &g)
{
    int type_as_int;
    s >> type_as_int;
    QCBrush::BrushType type = QCBrush::BrushType(type_as_int);
    // Stops
    QCGradientStops stops;
    s >> stops;
    // Gradient specifics
    if (type == QCBrush::BrushType::LinearGradient) {
        float startX, startY, endX, endY;
        s >> startX >> startY >> endX >> endY;
        QCLinearGradient lg(startX, startY, endX, endY);
        g = lg;
    } else if (type == QCBrush::BrushType::RadialGradient) {
        float cX, cY, oRad, iRad;
        s >> cX >> cY >> oRad >> iRad;
        QCRadialGradient rg(cX, cY, oRad, iRad);
        g = rg;
    } else if (type == QCBrush::BrushType::ConicalGradient) {
        float cX, cY, angle;
        s >> cX >> cY >> angle;
        QCConicalGradient cg(cX, cY, angle);
        g = cg;
    } else if (type == QCBrush::BrushType::BoxGradient) {
        float x, y, w, h, feather, radius;
        s >> x >> y >> w >> h >> feather >> radius;
        QCBoxGradient bg(x, y, w, h, feather, radius);
        g = bg;
    }
    g.setStops(stops);
    return s;
}

#endif // QT_NO_DATASTREAM

/*!
    Returns the type of gradient.
*/

QCBrush::BrushType QCGradient::type() const
{
    G_D();
    return d->type;
}

/*!
    Returns the gradient start color or the color at the smallest position.
    If the start color has not been set, returns the default
    start color white (255, 255, 255).
    \sa setStartColor()
*/

QColor QCGradient::startColor() const
{
    G_D();
    if (d->gradientStops.isEmpty())
        return QColor(255, 255, 255);
    return d->gradientStops.constFirst().second;
}

/*!
    Sets the start color of gradient to \a color.
    This is equal to calling setColorAt() with position \c 0.
*/

void QCGradient::setStartColor(const QColor &color)
{
    setColorAt(0.0f, color);
}

/*!
    Returns the gradient end color or the color at the largest position.
    If the end color has not been set, returns the default
    end color transparent black (0, 0, 0, 0).
    \sa setEndColor()
*/

QColor QCGradient::endColor() const
{
    G_D();

    if (d->gradientStops.isEmpty())
        return QColor(0, 0, 0, 0);
    return d->gradientStops.constLast().second;
}

/*!
    Sets the end color of gradient to \a color.
    This is equal to calling setColorAt() with position \c 1.
*/

void QCGradient::setEndColor(const QColor &color)
{
    setColorAt(1.0f, color);
}

/*!
    Creates a stop point at the given \a position with the given \a
    color. The given \a position must be in the range 0 to 1.

    \sa setStops(), stops()
*/

void QCGradient::setColorAt(float position, const QColor &color)
{
    G_D();

    if (Q_UNLIKELY(d->gradientStops.size() >= QCPAINTER_GRADIENT_MAX_STOPS)) {
        qWarning("QCGradient::setColorAt: The maximum amount of color stops is: %d",
                 QCPAINTER_GRADIENT_MAX_STOPS);
        return;
    }

    position = qBound(0.0f, position, 1.0f);
    detach();
    auto &stops = d->gradientStops;
    // Add or replace stop in the correct index so that stops remains sorted.
    qsizetype index = 0;
    while (index < stops.size() && stops.at(index).first < position) ++index;

    if (index < stops.size() && qFuzzyCompare(stops.at(index).first, position))
        stops[index].second = color;
    else
        stops.insert(index, QCGradientStop(position, color));

    d->dirty |= QCGradientPrivate::DirtyFlag::Stops;

}

/*!
    Replaces the current set of stop points with the given \a
    stops.

    The rules for the stops list are following:
    \list
    \li The list should contain at least 2 stops.
    \li The positions of the stops must be in the range 0 to
    1, and must be sorted with the lowest stops first.
    \li The first position must be at 0.0 and the last position at 1.0.
    \endlist

    \sa setColorAt(), stops()
*/

void QCGradient::setStops(const QCGradientStops &stops)
{
    G_D();
    detach();
    d->gradientStops = stops;
    d->dirty |= QCGradientPrivate::DirtyFlag::Stops;
}

/*!
    Returns the stop points for this gradient.

    \sa setStops(), setColorAt()
*/
QCGradientStops QCGradient::stops() const
{
    G_D();
    return d->gradientStops;
}


/*!
    \typedef QCGradientStop
    \relates QCGradient

    Typedef for std::pair<\c float, QColor>.
*/

/*!
    \typedef QCGradientStops
    \relates QCGradient

    Typedef for QList<QCGradientStop>.
*/

// ***** Private *****

QCGradientPrivate::QCGradientPrivate(QCBrush::BrushType type)
    : QCBrushPrivate(type)
    , dirty(DirtyFlag::All)
    , imageId(0)
{
}

// Convert quint64 into qint64
static constexpr qint64 toInt64(quint64 value) noexcept
{
    constexpr qint64 QINT64_MAX = std::numeric_limits<qint64>::max();
    if (value <= quint64(QINT64_MAX))
        return qint64(value);
    else
        return -qint64(~value) - 1;
}

// Create unique id hash for the gradient
// Required for caching the gradient textures
qint64 QCGradientPrivate::generateGradientKey() const
{
    quint64 id = 0;
    for (const auto &v : std::as_const(gradientStops)) {
        id += qHash(int(v.first * QCPAINTER_GRADIENT_SIZE))
              ^ qHash(v.second.rgba());
    }
    return toInt64(id);
}

// Fills gradient span from offset1 at color1 to offset2 at color2.
// The color2 is not fully reached, as it will be used as the starting color
// of the next span. So e.g. black -> red span from 0.0 to 0.02 will normally
// take 0.02 * 256 = 5 pixels and then last pixel will be 4/5 = 80% red. Next
// span (so pixel 6) will then start from 100% red.
static void gradientColorSpan(quint32 *data, QRgb color1, QRgb color2, float offset1, float offset2) noexcept
{
    int s = offset1 * QCPAINTER_GRADIENT_SIZE;
    int e = offset2 * QCPAINTER_GRADIENT_SIZE;
    int d = e - s;
    if (d < 1)
        return;
    constexpr float m = 1.0f / 256;
    float a = qAlpha(color1) * m;
    float r = qRed(color1) * m;
    float g = qGreen(color1) * m;
    float b = qBlue(color1) * m;
    float da = (qAlpha(color2) * m - a) / d;
    float dr = (qRed(color2) * m - r) / d;
    float dg = (qGreen(color2) * m - g) / d;
    float db = (qBlue(color2) * m - b) / d;
    for (int i = 0; i < d; i++) {
        QRgb rgba = qRgba((r + i * dr) * 256,
                          (g + i * dg) * 256,
                          (b + i * db) * 256,
                          (a + i * da) * 256);
        data[s + i] = ARGB2RGBA(rgba);
    }
}

void QCGradientPrivate::updateGradientTexture(QCPainter *painter)
{
    // If stops haven't changed, texture doesn't need changes
    if (!(dirty & QCGradientPrivate::DirtyFlag::Stops))
        return;

    const qint64 key = generateGradientKey();
    auto *painterPriv = QCPainterPrivate::get(painter);
    if (painterPriv->m_dataCache.contains(key)) {
        // Texture for the current stops is available in the cache
        imageId = painterPriv->m_dataCache.image(key).id();
    } else {
        const int gradStops = gradientStops.size();
        Q_ASSERT(gradStops >= 3); // Only gets called for gradients with more stops
        quint32 data[QCPAINTER_GRADIENT_SIZE];
        for (int i = 0; i < (gradStops - 1); i++)
        {
            const auto &grad1 = gradientStops[i];
            const auto &grad2 = gradientStops[i + 1];
            // Premultipled alpha
            QRgb c1 = qPremultiply(grad1.second.rgba());
            QRgb c2 = qPremultiply(grad2.second.rgba());
            float o1 = std::clamp(grad1.first, 0.0f, 1.0f);
            float o2 = std::clamp(grad2.first, 0.0f, 1.0f);
            gradientColorSpan(data, c1, c2, o1, o2);
        }
        // Make the first & last pixels to contain the colors
        // of the first & last stops
        data[0] = ARGB2RGBA(qPremultiply(gradientStops.constFirst().second.rgba()));
        data[QCPAINTER_GRADIENT_SIZE - 1] = ARGB2RGBA(qPremultiply(gradientStops.constLast().second.rgba()));

        // Create image texture
        QImage gradientTexture = QImage((uchar*)data, QCPAINTER_GRADIENT_SIZE, 1, QImage::Format_RGBA8888_Premultiplied);
        QCPainter::ImageFlags flags = {QCPainter::ImageFlag::Premultiplied};
        imageId = painterPriv->getQCImage(gradientTexture, flags, key).id();
    }
}

#undef G_D

QT_END_NAMESPACE
