// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcboxshadow.h"
#include "qcboxshadow_p.h"
#include "qcpainter_p.h"

QT_BEGIN_NAMESPACE

static const float QCPAINTER_BOX_SHADOW_MULTIPLIER = 1.0f;

/*!
    \class QCBoxShadow
    \brief QCBoxShadow is a brush for painting rectangle shadows / glows.
    \inmodule QtCanvasPainter

    QCBoxShadow is a rounded rectangle brush with blur applied.
    The performance of QCBoxShadow is much better than a general shadow that
    creates blurred shadow/glow of any shaped item, as it uses SDF approach.

    The features of QCBoxShadow are similar to CSS box-shadow, with radius,
    spread, blur and color values.
    The rendering output also matches the CSS box-shadow, with few notable
    differences to make the QCBoxShadow as high-performance as possible.
    Blurring is calculated mathematically in the shader rather than using
    Gaussian blur, which CSS box-shadow implementations often use. This makes
    the shadow look slightly different, especially when the blur amount
    grows bigger than half of the shadow width / height.

    Here are screenshots to compare the rendering output of QCBoxShadow and
    CSS box-shadow in the Chrome browser. These show how blur radius of
    0, 10, 20 and 40 pixels affect smaller shadows.

    CSS box-shadow:
    \image examples_css_box_shadow.png

    QCBoxShadow:
    \image examples_qcpainter_box_shadow.png
*/

/*!
    Constructs a default box shadow.
    The box shadow position is (0, 0) and size (100, 100).
    Shadow corner radius is \c 0.0 and blur \c 0.0.
    Shadow color is \c Qt::black.
*/

QCBoxShadow::QCBoxShadow()
    : d(new QCBoxShadowPrivate)
{
}

/*!
    Constructs a box shadow.
    The box shadow position and size are defined by \a rect.
    Shadow corner radius is \a radius and blur \a blur.
    Shadow color is \a color.
*/

QCBoxShadow::QCBoxShadow(const QRectF &rect, float radius, float blur, const QColor &color)
    : d(new QCBoxShadowPrivate)
{
    d->x = float(rect.x());
    d->y = float(rect.y());
    d->width = float(rect.width());
    d->height = float(rect.height());
    d->radius = radius;
    d->blur = blur;
    d->color = color;
}

/*!
    Constructs a box shadow.
    The box shadow position is \a x, \a y and size \a width, \a height.
    Shadow corner radius is \a radius and blur \a blur.
    Shadow color is \a color.
*/

QCBoxShadow::QCBoxShadow(float x, float y, float width, float height, float radius, float blur, const QColor &color)
    : d(new QCBoxShadowPrivate)
{
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->radius = radius;
    d->blur = blur;
    d->color = color;
}

/*!
    Constructs a box shadow that is a copy of the given \a shadow.
*/

QCBoxShadow::QCBoxShadow(const QCBoxShadow &shadow) noexcept
    : d(shadow.d)
{
}

/*!
    Destroys the box shadow.
*/

QCBoxShadow::~QCBoxShadow() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCBoxShadowPrivate)

/*!
    Assigns the given \a shadow to this shadow and returns a reference to
    this box shadow.
*/

QCBoxShadow &QCBoxShadow::operator=(const QCBoxShadow &shadow) noexcept
{
    QCBoxShadow(shadow).swap(*this);
    return *this;
}

/*!
    \fn QCBoxShadow::QCBoxShadow(QCBoxShadow &&other) noexcept

    Move-constructs a new QCBoxShadow from \a other.
*/

/*!
    \fn QCBoxShadow &QCBoxShadow::operator=(QCBoxShadow &&other)

    Move-assigns \a other to this QCBoxShadow instance.
*/

/*!
    \fn void QCBoxShadow::swap(QCBoxShadow &other)
    \memberswap{other}
*/

/*!
   Returns the box shadow as a QVariant.
*/

QCBoxShadow::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCBoxShadow::operator!=(const QCBoxShadow &shadow) const

    Returns \c true if the box shadow is different from the given \a shadow;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCBoxShadow::operator==(const QCBoxShadow &shadow) const

    Returns \c true if the box shadow is equal to the given \a shadow;
    otherwise false.

    \sa operator!=()
*/

bool QCBoxShadow::operator==(const QCBoxShadow &p) const
{
    if (p.d == d)
        return true;

    if (d->x != p.d->x
        || d->y != p.d->y
        || d->width != p.d->width
        || d->height != p.d->height
        || d->radius != p.d->radius
        || d->blur != p.d->blur
        || d->color != p.d->color
        || d->topLeftRadius != p.d->topLeftRadius
        || d->topRightRadius != p.d->topRightRadius
        || d->bottomLeftRadius != p.d->bottomLeftRadius
        || d->bottomRightRadius != p.d->bottomRightRadius)
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCBoxShadow &p)
{
    QDebugStateSaver saver(dbg);
    const auto &rect = p.rect();
    dbg.nospace() << "QCBoxShadow(" << rect << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCBoxShadow stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCBoxShadow &shadow)
    \relates QCBoxShadow

    Writes the given \a shadow to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCBoxShadow &p)
{
    const auto &r = p.rect();
    s << r.x() << r.y() << r.width() << r.height();
    s << p.radius() << p.blur() << p.spread();
    s << p.topLeftRadius() << p.topRightRadius();
    s << p.bottomLeftRadius() << p.bottomRightRadius();
    s << p.color();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCBoxShadow &shadow)
    \relates QCBoxShadow

    Reads the given \a shadow from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCBoxShadow &p)
{
    float x, y, width, height;
    float radius, blur, spread;
    float tlRad, trRad, blRad, brRad;
    QColor color;
    s >> x >> y >> width >> height;
    s >> radius >> blur >> spread;
    s >> tlRad >> trRad >> blRad >> brRad;
    s >> color;
    p.setRect(x, y, width, height);
    p.setRadius(radius);
    p.setBlur(blur);
    p.setSpread(spread);
    p.setTopLeftRadius(tlRad);
    p.setTopRightRadius(trRad);
    p.setBottomLeftRadius(blRad);
    p.setBottomRightRadius(brRad);
    p.setColor(color);
    return s;
}

#endif // QT_NO_DATASTREAM

/*!
    Returns the type of brush, \c QCBrush::BrushType::BoxShadow.
*/

QCBrush::BrushType QCBoxShadow::type() const
{
    return QCBrush::BrushType::BoxShadow;
}

/*!
    Returns the rect area of shadow box.
    \sa setRect()
*/

QRectF QCBoxShadow::rect() const
{
    return QRectF(d->x,
                  d->y,
                  d->width,
                  d->height);
}

/*!
    Sets the rect area of shadow box to \a rect.
    \sa rect()
*/

void QCBoxShadow::setRect(const QRectF &rect)
{
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

/*!
    Sets the rect area of shadow box to (\a x, \a y, \a width, \a height).
    \sa rect()
*/

void QCBoxShadow::setRect(float x, float y, float width, float height)
{
    detach();
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    \readonly

    Returns the area covered by the shadow. This takes into
    account the shadow \l rect(), \l blur() and \l spread().
    If you don't want to use \l QCPainter::drawBoxShadow() helper
    method, this can be used to for e.g. adding correctly sized
    \l QCPainter::rect() into the path.

    \sa setRect(), setBlur(), setSpread()
*/

QRectF QCBoxShadow::boundingRect() const
{
    // Extend the rect with blur, spread and aa
    const float aa = 1.0f;
    const float extend = d->blur + d->spread + aa;
    const float x = d->x - extend;
    const float y = d->y - extend;
    const float width = d->width + 2 * extend;
    const float height = d->height + 2 * extend;
    return QRectF(x, y, width, height);
}

/*!
    Returns the radius of shadow box.
    \sa setRadius()
*/

float QCBoxShadow::radius() const
{
    return d->radius;
}

/*!
    Sets the shadow corner radius to \a radius in pixels.
    The default value is \c 0.0 meaning no radius.
*/

void QCBoxShadow::setRadius(float radius)
{
    detach();
    d->radius = radius;
    d->changed = true;
}

/*!
    Returns the blur of shadow box.
    \sa setBlur()
*/

float QCBoxShadow::blur() const
{
    return d->blur;
}

/*!
    Sets the shadow blur to \a blur in pixels.
    The default value is \c 0.0 meaning no blur.
*/

void QCBoxShadow::setBlur(float blur)
{
    detach();
    d->blur = blur;
    d->changed = true;
}

/*!
    Returns the spread of shadow box.
    \sa setSpread()
*/

float QCBoxShadow::spread() const
{
    return d->spread;
}

/*!
    Sets the shadow spread to \a spread in pixels.
    The default value is \c 0.0 meaning no spread.
*/

void QCBoxShadow::setSpread(float spread)
{
    detach();
    d->spread = spread;
    d->changed = true;
}

/*!
    Returns the color of shadow box.
    \sa setColor()
*/

QColor QCBoxShadow::color() const
{
    return d->color;
}

/*!
    Sets the shadow color to \a color.
    The default value is black with full opacity.
*/

void QCBoxShadow::setColor(const QColor &color)
{
    detach();
    d->color = color;
    d->changed = true;
}

/*!
    Returns the top-left radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCBoxShadow::topLeftRadius() const
{
    return d->topLeftRadius;
}

/*!
    Sets the top-left corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCBoxShadow::setTopLeftRadius(float radius)
{
    detach();
    d->topLeftRadius = radius;
    d->changed = true;
}

/*!
    Returns the top-right radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCBoxShadow::topRightRadius() const
{
    return d->topRightRadius;
}

/*!
    Sets the top-right corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCBoxShadow::setTopRightRadius(float radius)
{
    detach();
    d->topRightRadius = radius;
    d->changed = true;
}

/*!
    Returns the bottom-left radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCBoxShadow::bottomLeftRadius() const
{
    return d->bottomLeftRadius;
}

/*!
    Sets the bottom-left corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCBoxShadow::setBottomLeftRadius(float radius)
{
    detach();
    d->bottomLeftRadius = radius;
    d->changed = true;
}

/*!
    Returns the bottom-right radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCBoxShadow::bottomRightRadius() const
{
    return d->bottomRightRadius;
}

/*!
    Sets the bottom-right corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCBoxShadow::setBottomRightRadius(float radius)
{
    detach();
    d->bottomRightRadius = radius;
    d->changed = true;
}


/*!
    \internal
*/

void QCBoxShadow::detach()
{
    if (d)
        d.detach();
    else
        d = new QCBoxShadowPrivate;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCBoxShadow::createPaint(QCPainter *painter) const
{
    auto *painterPriv = QCPainterPrivate::get(painter);
    if (d->changed) {
        auto *e = painterPriv->engine();

        // TODO: Support non-antialiased shadows?
        const float aa = 1.0f;

        // Adjust blur to grow equally towards in & out.
        const float blurIn = QCPAINTER_BOX_SHADOW_MULTIPLIER * d->blur + 0.5 * aa;
        const float blurOut = 2 * QCPAINTER_BOX_SHADOW_MULTIPLIER * d->blur + aa;

        float extendX = d->spread - blurIn;
        float extendY = d->spread - blurIn;

        // Limit max extends when width & height are < 0.
        // This reduces rendering issues of thin rects with big blur
        // values being to sharp at center (smoothstep is clipped),
        // but causes some extra blurriness to those.
        if (extendX <  -0.5f * d->width) {
            float diff = extendX + 0.5f * d->width;
            extendX -= 0.25 * diff;
        }
        if (extendY <  -0.5f * d->height) {
            float diff = extendY + 0.5f * d->height;
            extendY -= 0.25 * diff;
        }

        float x = d->x - extendX;
        float y = d->y - extendY;
        float width = d->width + 2 * extendX;
        float height = d->height + 2 * extendY;

        QColor color = d->clampedColor();

        // Use adjusted width & height extended with blur & spread.
        float commonRadius = d->clampedRadius(d->radius, width, height);
        QVector4D radius { commonRadius, commonRadius, commonRadius, commonRadius };

        if (d->topLeftRadius >= 0)
            radius.setX(d->clampedRadius(d->topLeftRadius, width, height));
        if (d->topRightRadius >= 0)
            radius.setY(d->clampedRadius(d->topRightRadius, width, height));
        if (d->bottomLeftRadius >= 0)
            radius.setZ(d->clampedRadius(d->bottomLeftRadius, width, height));
        if (d->bottomRightRadius >= 0)
            radius.setW(d->clampedRadius(d->bottomRightRadius, width, height));
        d->paint = e->createBoxShadow(x, y, width, height, radius, blurOut, color);

        d->changed = false;
    }

    return d->paint;
}

float QCBoxShadowPrivate::clampedRadius(float rad, float width, float height) const
{
    float maxRadius = qMin(width, height) * 0.5f;
    float spreadRadius = rad + spread;
    if (radius < spread && !qFuzzyIsNull(spread)) {
        // CSS box-shadow has a specific math to calculate radius with spread
        // https://www.w3.org/TR/css-backgrounds-3/#shadow-shape
        // "the spread distance is first multiplied by the proportion 1 + (r-1)^3,
        // where r is the ratio of the border radius to the spread distance".
        float r = (rad / spread) - 1;
        spreadRadius = rad + spread * (1 + r * r * r);
    }
    spreadRadius = qMax(0.0f, spreadRadius);
    return qMin(spreadRadius, maxRadius);
}

// Clamp the color to fade when the blur amount is
// bigger than the rect size.
QColor QCBoxShadowPrivate::clampedColor() const
{
    QColor c = color;
    const float minSize = qMin(width + 2 * spread, height + 2 * spread);
    const float blurTreshold = minSize * 0.5f * QCPAINTER_BOX_SHADOW_MULTIPLIER;
    if (blur > blurTreshold) {
        float fade = blurTreshold / blur;
        c.setAlphaF(color.alphaF() * fade);
    }
    return c;
}

QT_END_NAMESPACE
