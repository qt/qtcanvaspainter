// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcimage.h"
#include "qcimage_p.h"
#include "qvariant.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCImage
    \since 6.11
    \brief QCImage is the image class for QCPainter.
    \inmodule QtCanvasPainter

    QCImage is the image class used by QCPainter.
    To be able to paint images, they fist need to be made available
    with e.g. \l QCPainter::addImage(). Then images can be painted
    as-is using \l QCPainter::drawImage() or used with
    \l QCImagePattern brush to fill / stroke.

    Here is a simple example:
    \code
    static QImage logoImage(":/qtlogo.png");
    // Paint an image pattern.
    QCImage bg = painter.addImage(logoImage,
                 QCPainter::ImageFlag::Repeat |
                 QCPainter::ImageFlag::GenerateMipmaps);
    QCImagePattern ip(bg, 0, 0, 44, 32);
    painter.setFillStyle(ip);
    painter.fillRect(50, 50, 320, 230);
    // Paint a single image, with tint color.
    QCImage logo = painter.addImage(logoImage);
    logo.setTintColor("#2cde85");
    painter.drawImage(logo, 100, 80);
    \endcode

    \image image_example_1.png

    In the above example the QImage is static and addImage() is called
    on every repaint. This is not a problem as when the image and flags
    remain the same, addImage() fetches the image from cache instead of
    uploads it again as a texture. But a more common approac is having
    QCImage variables as class members and calling \l QCPainter::addImage()
    e.g. in \l QQuickCPainterRenderer::initializeResources().

    Similarly to QCBrush and QCOffscreenCanvas, QCImage is explicitly shared.
    See \l{Implicit Data Sharing} and \l QSharedDataPointer for details.

    \note A QCImage object contains only a handle to a graphics resource, such
    as a texture. Even when a detach occurs, the actual resource, i.e. the
    underlying texture and the image data in it, is never actually copied or
    duplicated. The actual owner of the real graphics resource (e.g., a
    QRhiTexture) is the QCPainter that handed out the QCImage via
    \l{QCPainter::}{addImage()}.

    A QCImage always belongs to the QCPainter that created it. Manually
    removing images is done by calling \l{QCPainter::}{removeImage()}. In
    most cases this will not be necessary, however, since the painter will
    automatically destroy any images during its own destruction.
*/

/*!
    Constructs a default image.
*/

QCImage::QCImage()
    : d(new QCImagePrivate())
{
}

/*!
    Constructs an image that is a copy of the given \a image.
*/

QCImage::QCImage(const QCImage &image)
    : d(image.d)
{
}

/*!
    Destroys the image.
*/

QCImage::~QCImage() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCImagePrivate)

/*!
    Assigns the given \a image to this image and returns a reference to
    this image.
*/

QCImage &QCImage::operator=(const QCImage &image) noexcept
{
    QCImage(image).swap(*this);
    return *this;
}

/*!
    \fn QCImage::QCImage(QCImage &&other) noexcept

    Move-constructs a new QCImage from \a other.
*/

/*!
    \fn QCImage &QCImage::operator=(QCImage &&other)

    Move-assigns \a other to this QCImage instance.
*/

/*!
    \fn void QCImage::swap(QCImage &other)
    \memberswap{image}
*/

/*!
   Returns the image as a QVariant.
*/

QCImage::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCImage::operator!=(const QCImage &image) const

    Returns \c true if the image is different from the given \a image;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCImage::operator==(const QCImage &image) const

    Returns \c true if the image is equal to the given \a image; otherwise
    false.

    \sa operator!=()
*/

bool QCImage::operator==(const QCImage &i) const noexcept
{
    if (i.d == d)
        return true;
    if (i.d->id != d->id ||
        i.d->width != d->width ||
        i.d->height != d->height ||
        i.d->size != d->size ||
        i.d->type != d->type ||
        i.d->tintColor != d->tintColor)
        return false;
    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCImage &i)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCImage(" << i.id() << ": " << i.width() << " X " << i.height() << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*!
    Returns the texture id for this image.
    When the image has not been assigned yet, this returns 0.
    \sa isNull()
*/

int QCImage::id() const
{
    return d->id;
}

/*!
    Returns the width of this image.
*/

int QCImage::width() const
{
    return d->width;
}

/*!
    Returns the height of this image.
*/

int QCImage::height() const
{
    return d->height;
}

/*!
    Returns the size of this image in bytes.
*/

int QCImage::size() const
{
    return d->size;
}

/*!
    Returns true if the image has not been assigned yet.
    See \l QCPainter::addImage() for details.
*/

bool QCImage::isNull() const
{
    return d->id == 0;
}

/*!
    Returns the tint color of the image.
    The default value is Qt::white.
*/

QColor QCImage::tintColor() const
{
    return d->tintColor;
}

/*!
    Set the tint \a color of the image.
    This color will be multiplied with the image color to
    colorize the image. This can be used for example to highlight
    icons or to adjust images becase on the theme.
*/

void QCImage::setTintColor(const QColor &color)
{
    detach();
    d->tintColor = color;
}

/*!
    \internal
*/

void QCImage::detach()
{
    if (d)
        d.detach();
    else
        d = new QCImagePrivate();
}

// ***** Private *****

QCImagePrivate::QCImagePrivate()
    : id(0)
    , width(0)
    , height(0)
    , type(DataType::Unknown)
    , size(0)
    , tintColor(QColorConstants::White)
{
}

QT_END_NAMESPACE
