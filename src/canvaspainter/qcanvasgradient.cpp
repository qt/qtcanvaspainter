// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaslineargradient.h"
#include "qcanvasradialgradient.h"
#include "qcanvasconicalgradient.h"
#include "qcanvasboxgradient.h"
#include "qcanvasimage.h"
#include "qcanvaspainter.h"
#include "qcanvaspainter_p.h"
#include "qdatastream.h"
#include "qvariant.h"
#include <qdebug.h>
#include <limits>
#include <QtGui/private/qpixellayout_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasGradient
    \since 6.11
    \brief QCanvasGradient is the base class for all QCanvasPainter gradient brushes.
    \inmodule QtCanvasPainter

    QCanvasGradient is the base class for all QCanvasPainter gradient brushes.
    Currently there are four type of gradients: \l QCanvasLinearGradient,
    \l QCanvasRadialGradient, \l QCanvasConicalGradient and \l QCanvasBoxGradient.

    If no stops are provided with \l setStartColor, \l setEndColor, \l setColorAt or \l setStops,
    the gradient is rendered as transparent black (0,0,0,0).
    If only a single stop is provided, the gradient is filled with this color.

    QCanvasPainter uses two different approaches for painting gradients.
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

QCanvasGradient::QCanvasGradient(QCanvasBrush::BrushType type)
    : m_type(type)
{
}

/*!
    Returns the type of gradient.
*/

QCanvasBrush::BrushType QCanvasGradient::type() const
{
    return m_type;
}

/*!
    Returns the gradient start color or the color at the smallest position.
    If any stops have not been set, returns transparent black (0, 0, 0, 0).
    \sa setStartColor()
*/

QColor QCanvasGradient::startColor() const
{
    if (m_stops.isEmpty())
        return QColorConstants::Transparent;
    return m_stops.constFirst().color;
}

/*!
    Sets the start color of gradient to \a color.
    This is equal to calling setColorAt() with position \c 0.
*/

void QCanvasGradient::setStartColor(const QColor &color)
{
    setColorAt(0.0f, color);
}

/*!
    Returns the gradient end color or the color at the largest position.
    If any stops have not been set, returns transparent black (0, 0, 0, 0).
    \sa setEndColor()
*/

QColor QCanvasGradient::endColor() const
{
    if (m_stops.isEmpty())
        return QColorConstants::Transparent;
    return m_stops.constLast().color;
}

/*!
    Sets the end color of gradient to \a color.
    This is equal to calling setColorAt() with position \c 1.
*/

void QCanvasGradient::setEndColor(const QColor &color)
{
    setColorAt(1.0f, color);
}

/*!
    Creates a stop point at the given \a position with the given \a
    color. The given \a position must be in the range 0 to 1.

    \sa setStops(), stops()
*/

void QCanvasGradient::setColorAt(float position, const QColor &color)
{
    if (Q_UNLIKELY(m_stops.size() >= QCPAINTER_GRADIENT_MAX_STOPS)) {
        qWarning("QCanvasGradient::setColorAt: The maximum amount of color stops is: %d",
                 QCPAINTER_GRADIENT_MAX_STOPS);
        return;
    }

    position = qBound(0.0f, position, 1.0f);
    auto &stops = m_stops;
    // Add or replace stop in the correct index so that stops remains sorted.
    qsizetype index = 0;
    while (index < stops.size() && stops.at(index).position < position) ++index;

    if (index < stops.size() && qFuzzyCompare(stops.at(index).position, position))
        stops[index].color = color;
    else
        stops.insert(index, { position, color });

    m_cachedBrush = {};
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

void QCanvasGradient::setStops(const QCanvasGradientStops &stops)
{
    m_stops = stops;
    m_cachedBrush = {};
}

/*!
    Returns the stop points for this gradient.

    \sa setStops(), setColorAt()
*/
QCanvasGradientStops QCanvasGradient::stops() const
{
    return m_stops;
}

/*!
    \fn void QCanvasGradient::addColorStop(float position, const QColor &color)
    \since 6.12

    Creates a stop point at the given \a position with the given \a
    color. The given \a position must be in the range 0 to 1.

    This method is provided for HTML CanvasGradient compatibility.
    It is equivalent to \l setColorAt().

    \sa setStops(), stops()
*/

/*!
    \struct QCanvasGradientStop
    \since 6.11
    \brief A gradient stop.
    \inmodule QtCanvasPainter

    Describes a stop point in a \l{QCanvasGradient}{gradient}.
*/

/*!
    \variable QCanvasGradientStop::position

    The position for the stop point.
*/

/*!
    \variable QCanvasGradientStop::color

    The color for the stop point.
*/

/*!
    \typedef QCanvasGradientStops
    \relates QCanvasGradient

    Typedef for QList<QCanvasGradientStop>.
*/

/*!
    \since 6.12
    Uses the \a image as the gradient source at the y-coordinate \a index.
    This is an alternative for setting the gradient stops using e.g.
    setStops() or setColorAt(). The expected default width of the image
    is 256 pixels, but it can be adjusted by defining the
    QCPAINTER_GRADIENT_SIZE. Index parameter is optional and not needed
    when the height of the image is 1 pixel. The maximum value of index
    should be image height - 1.

    Possible reasons to use this method instead of setting stops:
    \list
    \li Slightly faster to create as stops don't need to be parsed. Also,
        multiple gradients can be included in a single image using \a index.
    \li The ability to use gradient from design / image directly, instead
        of converting it to set of stops.
    \li The ability to have non-linear gradients, like e.g. Gaussian curve.
    \endlist

    \note If both the stops and the image have been set, stops will be used.

    Here is a simple example of how to use a \c{256x1} gradient image:
    \table
    \row
    \li {2, 1}
    \inlineimage gradient1.png
        {Thin horizontal strip blending blue to red to orange}
    \row
    \li
    \inlineimage qcgradient-image.webp
        {Square with a radial gradient from a white center to a red and
        purple edge}
    \li
    \code
    static QImage image(":/gradient1.png");
    QCanvasImage gradImage = p->addImage(image);
    QRectF rect(0, 0, 200, 200);
    QCanvasRadialGradient rg(rect.center(), 20, 150);
    rg.setImage(gradImage);
    p->setFillStyle(rg);
    p->fillRect(rect);
    \endcode
    \endtable

    \sa setStops()
*/

void QCanvasGradient::setImage(const QCanvasImage &image, int index)
{
    m_imageId = image.id();
    // Y-coordinate of the texture is the middle of the pixel at index.
    m_imageY = (index + 0.5f) / image.height();
    m_cachedBrush = {};
}

/*!
   Returns the gradient as a \l QCanvasBrush.
*/

QCanvasGradient::operator QCanvasBrush() const
{
    if (m_cachedBrush.type() != QCanvasBrush::BrushType::Invalid)
        return m_cachedBrush;

    QCanvasGradientBrushPrivate *p = nullptr;
    switch (m_type) {
    case QCanvasBrush::BrushType::LinearGradient: {
        auto *lp = new QCanvasLinearGradientBrushPrivate;
        lp->data.linear.sx = m_data.linear.sx;
        lp->data.linear.sy = m_data.linear.sy;
        lp->data.linear.ex = m_data.linear.ex;
        lp->data.linear.ey = m_data.linear.ey;
        p = lp;
        break;
    }
    case QCanvasBrush::BrushType::RadialGradient: {
        auto *rp = new QCanvasRadialGradientBrushPrivate;
        rp->data.radial.icx = m_data.radial.icx;
        rp->data.radial.icy = m_data.radial.icy;
        rp->data.radial.iRadius = m_data.radial.iRadius;
        rp->data.radial.ocx = m_data.radial.ocx;
        rp->data.radial.ocy = m_data.radial.ocy;
        rp->data.radial.oRadius = m_data.radial.oRadius;
        p = rp;
        break;
    }
    case QCanvasBrush::BrushType::ConicalGradient: {
        auto *cp = new QCanvasConicalGradientBrushPrivate;
        cp->data.conical.cx = m_data.conical.cx;
        cp->data.conical.cy = m_data.conical.cy;
        cp->data.conical.angle = m_data.conical.angle;
        p = cp;
        break;
    }
    case QCanvasBrush::BrushType::BoxGradient: {
        auto *bp = new QCanvasBoxGradientBrushPrivate;
        bp->data.box.x = m_data.box.x;
        bp->data.box.y = m_data.box.y;
        bp->data.box.width = m_data.box.width;
        bp->data.box.height = m_data.box.height;
        bp->data.box.feather = m_data.box.feather;
        bp->data.box.radius = m_data.box.radius;
        p = bp;
        break;
    }
    default:
        Q_ASSERT_X(false, "QCanvasGradient::operator QCanvasBrush()", "Invalid gradient type");
        return QCanvasBrush();
    }
    p->gradientStops = m_stops;
    p->imageId = m_imageId;
    p->imageY = m_imageY;
    m_cachedBrush = QCanvasBrushPrivate::create(p);
    return m_cachedBrush;
}

/*!
   Returns the gradient as a \l QVariant.
*/

QCanvasGradient::operator QVariant() const
{
    switch (m_type) {
    case QCanvasBrush::BrushType::LinearGradient:
        return QVariant::fromValue(static_cast<const QCanvasLinearGradient &>(*this));
    case QCanvasBrush::BrushType::RadialGradient:
        return QVariant::fromValue(static_cast<const QCanvasRadialGradient &>(*this));
    case QCanvasBrush::BrushType::ConicalGradient:
        return QVariant::fromValue(static_cast<const QCanvasConicalGradient &>(*this));
    case QCanvasBrush::BrushType::BoxGradient:
        return QVariant::fromValue(static_cast<const QCanvasBoxGradient &>(*this));
    default:
        return QVariant();
    }
}

/*!
    \fn bool QCanvasGradient::operator!=(const QCanvasGradient &lhs, const QCanvasGradient &rhs)

    \return \c true if the gradient \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasGradient::operator==(const QCanvasGradient &lhs, const QCanvasGradient &rhs)

    \return \c true if the gradient \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasGradient &lhs, const QCanvasGradient &rhs) noexcept
{
    if (&lhs == &rhs)
        return true;
    if (lhs.m_type != rhs.m_type)
        return false;
    if (!qCanvasGradientDataEquals(lhs.m_type, lhs.m_data, rhs.m_data))
        return false;
    return lhs.m_stops == rhs.m_stops
            && lhs.m_imageId == rhs.m_imageId
            && lhs.m_imageY == rhs.m_imageY;
}

bool QCanvasGradientBrushPrivate::equals(const QCanvasBrushPrivate &other) const noexcept
{
    Q_ASSERT(other.type == type);
    const auto &pd = static_cast<const QCanvasGradientBrushPrivate &>(other);

    if (!qCanvasGradientDataEquals(type, data, pd.data))
        return false;

    return gradientStops == pd.gradientStops
           && imageId == pd.imageId
           && imageY == pd.imageY;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasGradientStop &stop)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCanvasGradientStop(" << stop.position << ", " << stop.color << ')';
    return dbg;
}

/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasGradient &g)
{
    QDebugStateSaver saver(dbg);
    const auto t = g.type();
    if (t == QCanvasBrush::BrushType::LinearGradient)
        dbg.nospace() << "QCanvasLinearGradient(" << g.stops() << ')';
    else if (t == QCanvasBrush::BrushType::RadialGradient)
        dbg.nospace() << "QCanvasRadialGradient(" << g.stops() << ')';
    else if (t == QCanvasBrush::BrushType::ConicalGradient)
        dbg.nospace() << "QCanvasConicalGradient(" << g.stops() << ')';
    else if (t == QCanvasBrush::BrushType::BoxGradient)
        dbg.nospace() << "QCanvasBoxGradient(" << g.stops() << ')';
    else
        dbg.nospace() << "QCanvasGradient(" << g.stops() << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCanvasGradient stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM

/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasGradientStop &stop)
    \relates QCanvasGradient

    Writes gradient \a stop to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator<<(QDataStream &s, const QCanvasGradientStop &stop)
{
    s << stop.position;
    s << stop.color;
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasGradientStop &stop)
    \relates QCanvasGradient

    Reads a gradient \a stop from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator>>(QDataStream &s, QCanvasGradientStop &stop)
{
    float position;
    s >> position;
    QColor color;
    s >> color;
    stop.position = position;
    stop.color = color;
    return s;
}

/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasGradient &gradient)
    \relates QCanvasGradient

    Writes the given \a gradient to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCanvasGradient &g)
{
    s << g.type();
    s << g.stops();
    if (g.type() == QCanvasBrush::BrushType::LinearGradient) {
        const auto &lg = static_cast<const QCanvasLinearGradient &>(g);
        const auto &sp = lg.startPosition();
        const auto &ep = lg.endPosition();
        s << sp.x() << sp.y() << ep.x() << ep.y();
    } else if (g.type() == QCanvasBrush::BrushType::RadialGradient) {
        const auto &rg = static_cast<const QCanvasRadialGradient &>(g);
        const auto &icp = rg.innerCenterPosition();
        s << icp.x() << icp.y() << rg.innerRadius();
        const auto &ocp = rg.outerCenterPosition();
        s << ocp.x() << ocp.y() << rg.outerRadius();
    } else if (g.type() == QCanvasBrush::BrushType::ConicalGradient) {
        const auto &cg = static_cast<const QCanvasConicalGradient &>(g);
        const auto &cp = cg.centerPosition();
        s << cp.x() << cp.y();
        s << cg.startAngle();
    } else if (g.type() == QCanvasBrush::BrushType::BoxGradient) {
        const auto &bg = static_cast<const QCanvasBoxGradient &>(g);
        const auto &r = bg.rect();
        s << r.x() << r.y() << r.width() << r.height();
        s << bg.feather();
        s << bg.radius();
    }
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasGradient &gradient)
    \relates QCanvasGradient

    Reads the given \a gradient from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCanvasGradient &g)
{
    int type_as_int;
    s >> type_as_int;
    QCanvasBrush::BrushType type = QCanvasBrush::BrushType(type_as_int);
    // Stops
    QCanvasGradientStops stops;
    s >> stops;
    // Gradient specifics
    if (type == QCanvasBrush::BrushType::LinearGradient) {
        float startX, startY, endX, endY;
        s >> startX >> startY >> endX >> endY;
        g = QCanvasLinearGradient(startX, startY, endX, endY);
    } else if (type == QCanvasBrush::BrushType::RadialGradient) {
        float icX, icY, iRad, ocX, ocY, oRad;
        s >> icX >> icY >> iRad >> ocX >> ocY >> oRad;
        g = QCanvasRadialGradient(icX, icY, iRad, ocX, ocY, oRad);
    } else if (type == QCanvasBrush::BrushType::ConicalGradient) {
        float cX, cY, angle;
        s >> cX >> cY >> angle;
        g = QCanvasConicalGradient(cX, cY, angle);
    } else if (type == QCanvasBrush::BrushType::BoxGradient) {
        float x, y, w, h, feather, radius;
        s >> x >> y >> w >> h >> feather >> radius;
        QCanvasBoxGradient bg(x, y, w, h);
        bg.setFeather(feather);
        bg.setRadius(radius);
        g = bg;
    }
    g.setStops(stops);
    return s;
}

#endif // QT_NO_DATASTREAM

// ***** QCanvasGradientBrushPrivate *****

QCanvasGradientBrushPrivate::QCanvasGradientBrushPrivate(QCanvasBrush::BrushType type)
    : QCanvasBrushPrivate(type)
    , dirty(DirtyFlag::All)
    , imageId(0)
    , imageY(0.5f)
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
qint64 QCanvasGradientBrushPrivate::generateGradientKey() const
{
    quint64 id = 0;
    for (const auto &v : std::as_const(gradientStops)) {
        id += qHash(int(v.position * QCPAINTER_GRADIENT_SIZE))
              ^ qHash(v.color.rgba());
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

void QCanvasGradientBrushPrivate::updateGradientTexture(QCanvasPainter *painter)
{
    // If stops haven't changed, texture doesn't need changes
    if (!(dirty & QCanvasGradientBrushPrivate::DirtyFlag::Stops))
        return;

    const qint64 key = generateGradientKey();
    auto *painterPriv = QCanvasPainterPrivate::get(painter);
    if (painterPriv->m_imageTracker.contains(key)) {
        // Texture for the current stops is available in the cache
        imageId = painterPriv->m_imageTracker.image(key).id();
    } else {
        const int gradStops = gradientStops.size();
        Q_ASSERT(gradStops >= 3); // Only gets called for gradients with more stops
        quint32 data[QCPAINTER_GRADIENT_SIZE];
        for (int i = 0; i < (gradStops - 1); i++)
        {
            const auto &grad1 = gradientStops[i];
            const auto &grad2 = gradientStops[i + 1];
            // Premultipled alpha
            QRgb c1 = qPremultiply(grad1.color.rgba());
            QRgb c2 = qPremultiply(grad2.color.rgba());
            float o1 = std::clamp(grad1.position, 0.0f, 1.0f);
            float o2 = std::clamp(grad2.position, 0.0f, 1.0f);
            gradientColorSpan(data, c1, c2, o1, o2);
        }
        // Make the first & last pixels to contain the colors
        // of the first & last stops
        data[0] = ARGB2RGBA(qPremultiply(gradientStops.constFirst().color.rgba()));
        data[QCPAINTER_GRADIENT_SIZE - 1] = ARGB2RGBA(qPremultiply(gradientStops.constLast().color.rgba()));

        // Create image texture
        QImage gradientTexture = QImage((uchar*)data, QCPAINTER_GRADIENT_SIZE, 1, QImage::Format_RGBA8888_Premultiplied);
        QCanvasPainter::ImageFlags flags = {QCanvasPainter::ImageFlag::Premultiplied};
        imageId = painterPriv->getQCanvasImage(gradientTexture, flags, key).id();
    }
}

QT_END_NAMESPACE
