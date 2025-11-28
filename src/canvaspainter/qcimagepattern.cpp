// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcimagepattern.h"
#include "qcimagepattern_p.h"
#include "qcpainter_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCImagePattern
    \brief QCImagePattern is a brush for painting image patterns.
    \inmodule QtCanvasPainter

    QCImagePattern is a brush for painting image patterns.
    To use QCImage as a pattern, it should have some repeat
    flags turned on like \l QCPainter::ImageFlag::Repeat.

    Here is a simple example:
    \code
    // Setup 2 image patterns
    static QImage image1(":/pattern2.png");
    static QImage image2(":/pattern3.png");
    auto flags = QCPainter::ImageFlag::Repeat |
                 QCPainter::ImageFlag::GenerateMipmaps;
    QCImage bg1 = painter.addImage(image1, flags);
    QCImage bg2 = painter.addImage(image2, flags);
    QCImagePattern ip1(bg1, 0, 0, 64, 64);
    QCImagePattern ip2(bg2, 0, 0, 32, 32);
    // Fill and stroke round rectangle with
    // these image pattern brushes.
    painter.beginPath();
    painter.roundRect(50, 50, 180, 180, 40);
    painter.setFillStyle(ip1);
    painter.setStrokeStyle(ip2);
    painter.fill();
    painter.setLineWidth(20);
    painter.stroke();
    \endcode

    \image imagepattern_example_1.png

    \note When using image patterns, images are often scaled to smaller
    and it can be useful to set \l QCPainter::ImageFlag::GenerateMipmaps
    for the used QCImage to have smooth patterns.
*/

/*!
    Constructs a default image pattern.
    Image will not be set, please use setImage() after this constructor.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0 and tint color white (no tinting).

    \sa setImage()
*/

QCImagePattern::QCImagePattern()
    : d(new QCImagePatternPrivate)
{
}

/*!
    Constructs an image pattern.
    Pattern will use \a image. Please note that \a image should usually
    have at least \l QCPainter::ImageFlag::Repeat flag set.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0 and tint color white (no tinting).
*/

QCImagePattern::QCImagePattern(const QCImage &image)
    : d(new QCImagePatternPrivate)
{
    d->image = image;
    d->width = d->image.width();
    d->height = d->image.height();
}

/*!
    Constructs an image pattern.
    Pattern will use \a image. Please note that \a image should usually
    have at least \l QCPainter::ImageFlag::Repeat flag set.
    Pattern image position position and size is defined with \a rect.
    Pattern angle is \a angle and tint color is \a tintColor.
*/

QCImagePattern::QCImagePattern(const QCImage &image, const QRectF &rect, float angle, const QColor &tintColor)
    : d(new QCImagePatternPrivate)
{
    d->image = image;
    d->x = float(rect.x());
    d->y = float(rect.y());
    d->width = float(rect.width());
    d->height = float(rect.height());
    d->angle = angle;
    d->tintColor = tintColor;
}

/*!
    Constructs an image pattern.
    Pattern will use \a image. Please note that \a image should usually
    have at least \l QCPainter::ImageFlag::Repeat flag set.
    Pattern image start position position is ( \a x, \a y) and pattern size ( \a width, \a height).
    Pattern angle is \a angle and tint color is \a tintColor.
*/

QCImagePattern::QCImagePattern(const QCImage &image, float x, float y, float width, float height, float angle, const QColor &tintColor)
    : d(new QCImagePatternPrivate)
{
    d->image = image;
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->angle = angle;
    d->tintColor = tintColor;
}

/*!
    Constructs an image pattern that is a copy of the given \a pattern.
*/

QCImagePattern::QCImagePattern(const QCImagePattern &pattern) noexcept
    : d(pattern.d)
{
}

/*!
    Destroys the image pattern.
*/

QCImagePattern::~QCImagePattern() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCImagePatternPrivate)

/*!
    Assigns the given image \a pattern to this pattern and returns a reference to
    this image pattern.
*/

QCImagePattern &QCImagePattern::operator=(const QCImagePattern &pattern) noexcept
{
    QCImagePattern(pattern).swap(*this);
    return *this;
}

/*!
    \fn QCImagePattern::QCImagePattern(QCImagePattern &&other) noexcept

    Move-constructs a new QCImagePattern from \a other.
*/

/*!
    \fn QCImagePattern &QCImagePattern::operator=(QCImagePattern &&other)

    Move-assigns \a other to this QCImagePattern instance.
*/

/*!
    \fn void QCImagePattern::swap(QCImagePattern &other)
    \memberswap{pattern}
*/

/*!
   Returns the image pattern as a QVariant.
*/

QCImagePattern::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCImagePattern::operator!=(const QCImagePattern &pattern) const

    Returns \c true if the image pattern is different from the given \a pattern;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCImagePattern::operator==(const QCImagePattern &pattern) const

    Returns \c true if the image pattern is equal to the given \a pattern; otherwise
    false.

    \sa operator!=()
*/

bool QCImagePattern::operator==(const QCImagePattern &p) const
{
    if (p.d == d)
        return true;

    if (d->x != p.d->x
        || d->y != p.d->y
        || d->width != p.d->width
        || d->height != p.d->height
        || d->angle != p.d->angle
        || d->tintColor != p.d->tintColor)
        return false;

    // For images comparing the id is enough
    if (d->image.id() != p.d->image.id())
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCImagePattern &p)
{
    QDebugStateSaver saver(dbg);
    const auto &sp = p.startPosition();
    const auto &size = p.imageSize();
    dbg.nospace() << "QCImagePattern(" << sp << ", " << size << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCImagePattern stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCImagePattern &pattern)
    \relates QCImagePattern

    Writes the given \a pattern to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCImagePattern &p)
{
    const auto &sp = p.startPosition();
    const auto &size = p.imageSize();
    s << sp.x() << sp.y();
    s << size.width() << size.height();
    s << p.rotation() << p.tintColor();
    // Note: Image not handled as it is mostly ID into texture
    //s << p.image();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCImagePattern &pattern)
    \relates QCImagePattern

    Reads the given \a pattern from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCImagePattern &p)
{
    float x, y, width, height, angle;
    QColor tintColor;
    s >> x >> y >> width >> height >> angle>> tintColor;
    p.setStartPosition(x, y);
    p.setImageSize(width, height);
    p.setRotation(angle);
    p.setTintColor(tintColor);
    // Note: Image not handled as it is mostly ID into texture
    //QImage image;
    //s >> image;
    //p.setImage(std::move(image));
    return s;
}

#endif // QT_NO_DATASTREAM

/*!
    Returns the type of brush, \c QCBrush::BrushType::ImagePattern.
*/

QCBrush::BrushType QCImagePattern::type() const
{
    return QCBrush::BrushType::ImagePattern;
}

/*!
    Returns the start point of image pattern.
    \sa setStartPosition()
*/

QPointF QCImagePattern::startPosition() const
{
    return QPointF(d->x, d->y);
}

/*!
    Sets the start point of image pattern to (\a x, \a y).
    Start position means top-left corner of an image in pattern.
    Pattern will then be extended to all positions from here
    (if image \c Repeat flag has been set).
*/

void QCImagePattern::setStartPosition(float x, float y)
{
    detach();
    d->x = x;
    d->y = y;
    d->changed = true;
}

/*!
    Sets the start point of image pattern to \a point.
    Start position means top-left corner of an image in pattern.
    Pattern will then be extended to all positions from here
    (if image \c Repeat flag has been set).
*/

void QCImagePattern::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()),
                     float(point.y()));
}

/*!
    Returns the size of a single image in pattern.
    \sa setImageSize()
*/

QSizeF QCImagePattern::imageSize() const
{
    return QSizeF(d->width,
                  d->height);
}

/*!
    Sets the size of a single image in pattern to ( \a width, \a height).
*/

void QCImagePattern::setImageSize(float width, float height)
{
    detach();
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    Sets the size of a single image in pattern to \a size.
*/

void QCImagePattern::setImageSize(QSizeF size)
{
    setImageSize(float(size.width()),
                 float(size.height()));
}

/*!
    Returns the image of the pattern.
    \sa setImage()
*/
QCImage QCImagePattern::image() const
{
    return d->image;
}

/*!
    Sets the image of the pattern to \a image.
    Note: Image should usually have \c Repeat flag set
    when used in image pattern.
*/

void QCImagePattern::setImage(const QCImage &image)
{
    detach();
    d->image = image;
    d->changed = true;
}

/*!
    Returns the pattern rotation in radians.
*/

float QCImagePattern::rotation() const
{
    return d->angle;
}

/*!
    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the image startPosition().
    The default value is \c 0.0, meaning the image is not rotated.
*/

void QCImagePattern::setRotation(float rotation)
{
    detach();
    d->angle = rotation;
    d->changed = true;
}

/*!
    Returns the pattern tint color.
*/

QColor QCImagePattern::tintColor() const
{
    return d->tintColor;
}

/*!
    Sets the pattern tint color to \a color.
    The color of the pattern image will be multiplied with
    this tint color in the shader.
    The default value is white, meaning no tinting.
    \note To set alpha globally, use QCPainter::setGlobalAlpha()
*/

void QCImagePattern::setTintColor(const QColor &color)
{
    detach();
    d->tintColor = color;
    d->changed = true;
}

/*!
    \internal
*/

void QCImagePattern::detach()
{
    if (d)
        d.detach();
    else
        d = new QCImagePatternPrivate;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCImagePattern::createPaint(QCPainter *painter) const
{
    auto *painterPriv = QCPainterPrivate::get(painter);
    if (d->changed) {
        auto *e = painterPriv->engine();
        if (d->image.isNull()) {
            qWarning() << "No image set for pattern, please use setImage()";
        } else {
            d->paint = e->createImagePattern(d->x, d->y, d->width, d->height,
                                             d->image.id(), d->angle, d->tintColor);
        }
        d->changed = false;
    }
    painterPriv->markTextureIdUsed(d->image.id());
    return d->paint;
}

QT_END_NAMESPACE
