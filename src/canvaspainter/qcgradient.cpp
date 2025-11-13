// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

QT_BEGIN_NAMESPACE

/*!
    \class QCGradient
    \brief QCGradient is the base class for all QCPainter gradient brushes.
    \inmodule QtCanvasPainter

    QCGradient is the base class for all QCPainter gradient brushes.
    Currently there are four type of gradients: \l QCLinearGradient,
    \l QCRadialGradient, \l QCConicalGradient and \l QCBoxGradient.

    If no stops are provided with \l setStartColor, \l setEndColor, \l setColorAt or \l setStops,
    the gragient is rendered as start color white (255,255,255) and end color transparent (0,0,0,0).
    If only a single stop is provided, the gradient is filled with this color.
*/

// Pixel amount of textured gradients
#ifndef QCPAINTER_GRADIENT_SIZE
#define QCPAINTER_GRADIENT_SIZE 256
#endif

#ifndef QCPAINTER_GRADIENT_MAX_STOPS
#define QCPAINTER_GRADIENT_MAX_STOPS 16
#endif

/*!
    Constructs a default gradient of type \a type.
*/

QCGradient::QCGradient(QCBrush::BrushType type)
    : d(new QCGradientPrivate(type))
{
}

/*!
    Constructs a gradient that is a copy of the given \a gradient.
*/

QCGradient::QCGradient(const QCGradient &gradient) noexcept
    : d(gradient.d)
{
}

/*!
    Destroys the gradient.
*/

QCGradient::~QCGradient() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCGradientPrivate)

/*!
    Assigns the given \a gradient to this gradient and returns a reference to
    this gradient.
*/

QCGradient &QCGradient::operator=(const QCGradient &gradient) noexcept
{
    QCGradient(gradient).swap(*this);
    return *this;
}

/*!
    \fn QCGradient::QCGradient(QCGradient &&other) noexcept

    Move-constructs a new QCGradient from \a other.
*/

/*!
    \fn QCGradient &QCGradient::operator=(QCGradient &&other)

    Move-assigns \a other to this QCGradient instance.
*/

/*!
    \fn void QCGradient::swap(QCGradient &other)
    \memberswap{gradient}
*/

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

bool QCGradient::operator==(const QCGradient &g) const
{
    if (g.d == d)
        return true;
    if (g.d->type != d->type)
        return false;
    if (d->type == BrushType::LinearGradient) {
        if (d->data.linear.sx != g.d->data.linear.sx
            || d->data.linear.sy != g.d->data.linear.sy
            || d->data.linear.ex != g.d->data.linear.ex
            || d->data.linear.ey != g.d->data.linear.ey)
            return false;
    } else if (d->type == BrushType::RadialGradient) {
        if (d->data.radial.cx != g.d->data.radial.cx
            || d->data.radial.cy != g.d->data.radial.cy
            || d->data.radial.oRadius != g.d->data.radial.oRadius
            || d->data.radial.iRadius != g.d->data.radial.iRadius)
            return false;
    } else if (d->type == BrushType::ConicalGradient) {
        if (d->data.conical.cx != g.d->data.conical.cx
            || d->data.conical.cy != g.d->data.conical.cy
            || d->data.conical.angle != g.d->data.conical.angle)
            return false;
    } else if (d->type == BrushType::BoxGradient) {
        if (d->data.box.x != g.d->data.box.x
            || d->data.box.y != g.d->data.box.y
            || d->data.box.width != g.d->data.box.width
            || d->data.box.height != g.d->data.box.height
            || d->data.box.feather != g.d->data.box.feather
            || d->data.box.radius != g.d->data.box.radius)
            return false;
    }
    return g.d->gradientStops == d->gradientStops;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCGradient &g)
{
    QDebugStateSaver saver(dbg);
    const auto t = g.type();
    if (t == QCGradient::BrushType::LinearGradient)
        dbg.nospace() << "QCLinearGradient(" << g.stops() << ')';
    else if (t == QCGradient::BrushType::RadialGradient)
        dbg.nospace() << "QCRadialGradient(" << g.stops() << ')';
    else if (t == QCGradient::BrushType::ConicalGradient)
        dbg.nospace() << "QCConicalGradient(" << g.stops() << ')';
    else if (t == QCGradient::BrushType::BoxGradient)
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
    if (g.type() == QCGradient::BrushType::LinearGradient) {
        const auto lg = static_cast<const QCLinearGradient *>(&g);
        const auto &sp = lg->startPosition();
        const auto &ep = lg->endPosition();
        s << sp.x() << sp.y() << ep.x() << ep.y();
    } else if (g.type() == QCGradient::BrushType::RadialGradient) {
        const auto rg = static_cast<const QCRadialGradient *>(&g);
        const auto &cp = rg->centerPosition();
        s << cp.x() << cp.y();
        s << rg->outerRadius();
        s << rg->innerRadius();
    } else if (g.type() == QCGradient::BrushType::ConicalGradient) {
        const auto cg = static_cast<const QCConicalGradient *>(&g);
        const auto &cp = cg->centerPosition();
        s << cp.x() << cp.y();
        s << cg->angle();
    } else if (g.type() == QCGradient::BrushType::BoxGradient) {
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
    QCGradient::BrushType type = QCGradient::BrushType(type_as_int);
    // Stops
    QCGradientStops stops;
    s >> stops;
    // Gradient specifics
    if (type == QCGradient::BrushType::LinearGradient) {
        float startX, startY, endX, endY;
        s >> startX >> startY >> endX >> endY;
        QCLinearGradient lg(startX, startY, endX, endY);
        g = lg;
    } else if (type == QCGradient::BrushType::RadialGradient) {
        float cX, cY, oRad, iRad;
        s >> cX >> cY >> oRad >> iRad;
        QCRadialGradient rg(cX, cY, oRad, iRad);
        g = rg;
    } else if (type == QCGradient::BrushType::ConicalGradient) {
        float cX, cY, angle;
        s >> cX >> cY >> angle;
        QCConicalGradient cg(cX, cY, angle);
        g = cg;
    } else if (type == QCGradient::BrushType::BoxGradient) {
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
    return d->gradientStops;
}

/*!
    \internal
*/
void QCGradient::detach()
{
    if (d)
        d.detach();
    else
        d = new QCGradientPrivate(type());
}

/*!
    \typedef QCGradientStop
    \relates QCGradient

    Typedef for std::pair<\l float, QColor>.
*/

/*!
    \typedef QCGradientStops
    \relates QCGradient

    Typedef for QList<QCGradientStop>.
*/

// ***** Private *****

QCGradientPrivate::QCGradientPrivate(QCBrush::BrushType type)
    : type(type)
    , dirty(DirtyFlag::All)
    , imageId(0)
{
}

// Create unique id hash for the gradient
// Required for caching the gradient textures
qint64 QCGradientPrivate::generateGradientId() const
{
    qint64 id = 0;
    for (const auto &v : std::as_const(gradientStops)) {
        id ^= qHash(int(v.first * QCPAINTER_GRADIENT_SIZE))
              ^ qHash(v.second.rgba());
    }
    return id;
}

void QCGradientPrivate::gradientColorSpan(quint32 *data, const QColor &color1, const QColor &color2, float offset1, float offset2)
{
    int s = offset1 * QCPAINTER_GRADIENT_SIZE;
    int e = offset2 * QCPAINTER_GRADIENT_SIZE;
    int d = e - s;
    float a = color1.alphaF();
    float r = color1.redF();
    float g = color1.greenF();
    float b = color1.blueF();
    float da = (color2.alphaF() - a) / d;
    float dr = (color2.redF() - r) / d;
    float dg = (color2.greenF() - g) / d;
    float db = (color2.blueF() - b) / d;
    for (int i = s; i < e; i++)
    {
        quint8 ua = quint8(a * 255);
        quint8 ur = quint8(r * 255);
        quint8 ug = quint8(g * 255);
        quint8 ub = quint8(b * 255);
        data[i] = (ua << 24) | (ub << 16) | (ug << 8) | ur;
        a += da;
        r += dr;
        g += dg;
        b += db;
    }
}

void QCGradientPrivate::updateGradientTexture(QCPainter *painter)
{
    // If stops haven't changed, texture doesn't need changes
    if (!(dirty & QCGradientPrivate::DirtyFlag::Stops))
        return;

    const qint64 key = generateGradientId();
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
            QColor c1 = grad1.second;
            QColor c2 = grad2.second;
            // Premultipled alpha
            c1 = QColor(c1.alphaF() * c1.red(),
                        c1.alphaF() * c1.green(),
                        c1.alphaF() * c1.blue(),
                        c1.alpha());
            c2 = QColor(c2.alphaF() * c2.red(),
                        c2.alphaF() * c2.green(),
                        c2.alphaF() * c2.blue(),
                        c2.alpha());
            float o1 = std::clamp(grad1.first, 0.0f, 1.0f);
            float o2 = std::clamp(grad2.first, 0.0f, 1.0f);
            gradientColorSpan(data, c1, c2, o1, o2);
        }
        QImage gradientTexture = QImage((uchar*)data, QCPAINTER_GRADIENT_SIZE, 1, QImage::Format_RGBA8888_Premultiplied);
        QCPainter::ImageFlags flags = {QCPainter::ImageFlag::Premultiplied};
        imageId = painterPriv->getQCImage(gradientTexture, flags, key).id();
    }
}

QT_END_NAMESPACE
