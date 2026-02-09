// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcoffscreencanvas_p.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCOffscreenCanvas
    \since 6.11
    \brief QCOffscreenCanvas represents an offscreen canvas that QCPainter can target.
    \inmodule QtCanvasPainter

    An offscreen canvas is essentially a color buffer (a QRhiTexture), and
    possibly other associated graphics resources. After content has been
    rendered into the canvas's color buffer, the canvas can be
    \l{QCPainter::addImage()}{registered} to the painter to retrieve a QCImage
    that can then be used in \l{QCPainter::drawImage()}{drawImage()} calls or as
    an \l{QCImagePattern}{image pattern}.

    To create an offscreen canvas, call \l QCPainter::createCanvas(). To target
    an offscreen canvas with with draw commands, call the appropriate \l
    QCRhiPaintDriver::beginPaint() overload when working with the lower level
    API, or \l QCPainterWidget::beginCanvasPainting() or \l
    QQuickCPainterRenderer::beginCanvasPainting() when using the convenience
    widget or Qt Quick item classes.

    Similarly to QCImage and QCBrush, QCOffscreenCanvas is explicitly shared.
    See \l{Implicit Data Sharing} and \l QSharedDataPointer for details.

    \note As with QCImage, a QCOffscreenCanvas can be seen as an object merely
    containing handles. Even when a detach occurs, the actual resources, e.g.
    the underlying texture and the image data in it, are never actually copied
    or duplicated. The actual owner of any real graphics resources (e.g., a
    QRhiTexture) is the QCPainter that handed out the QCOffscreenCanvas via
    \l{QCPainter::}{createCanvas()}.

    A canvas always belongs to the QCPainter that created it. Manually
    destroying canvases is done by calling \l{QCPainter::}{destroyCanvas()}. In
    most cases this will not be necessary, however, since the painter will
    automatically destroy any canvases during its own destruction.
 */

 /*!
    \enum QCOffscreenCanvas::Flag

    Specifies the flags for the canvas.

    \value PreserveContents Indicates that the contents of the canvas is
    preserved when painting to it. This can have a negative effect on
    performance, depending on the GPU architecture. See
    \l{QRhiTextureRenderTarget::PreserveColorContents} for details. Setting
    this flag is not supported when the sample count is greater than 1.

    \value MipMaps Indicates that the canvas' backing texture should have
    mipmapping enabled. Setting this flag is not supported when the sample count
    is greater than 1. Note that the mipmap sequence is generated when calling
    \l{QCPainter::}{addImage()} with the
    \l{QCPainter::ImageFlag::}{GenerateMipmaps} flag set. This implies that
    addImage() must be called every time after the canvas' content changes, so
    that the mipmap chain is regenerated. Frequent mipmap generation can have a
    negative effect on performance.
 */

/*!
    \internal
*/
QCOffscreenCanvas::QCOffscreenCanvas()
    : d(new QCOffscreenCanvasPrivate)
{
}

/*!
    Copy constructor.
*/
QCOffscreenCanvas::QCOffscreenCanvas(const QCOffscreenCanvas &canvas)
    : d(canvas.d)
{
}

/*!
    Destructor.
*/
QCOffscreenCanvas::~QCOffscreenCanvas() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCOffscreenCanvasPrivate)

/*!
    Assigns a shallow copy of the given \a canvas to this canvas and returns a
    reference to this canvas. For more information about shallow copies, see the
    \l{Implicit Data Sharing}{implicit data sharing} documentation.

    \note The underlying resources, the graphics resources such as the
    QRhiTexture, are never copied or duplicated.
*/
QCOffscreenCanvas &QCOffscreenCanvas::operator=(const QCOffscreenCanvas &canvas) noexcept
{
    QCOffscreenCanvas(canvas).swap(*this);
    return *this;
}

/*!
    \fn bool QCOffscreenCanvas::operator!=(const QCOffscreenCanvas &lhs, const QCOffscreenCanvas &rhs)

    \return \c true if the canvas handle \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCOffscreenCanvas::operator==(const QCOffscreenCanvas &lhs, const QCOffscreenCanvas &rhs)

    \return \c true if the canvas handle \a lhs is equal to \a rhs; \c false otherwise.

    \note Equality means that the two canvas objects' fill colors are the same and they
    reference the same graphics resources. The contents (pixel data) is not
    compared.

    \sa operator!=()
*/
bool comparesEqual(const QCOffscreenCanvas &lhs, const QCOffscreenCanvas &rhs) noexcept
{
    auto *d = QCOffscreenCanvasPrivate::get(&lhs);
    auto *pd = QCOffscreenCanvasPrivate::get(&rhs);

    if (pd == d)
        return true;

    if (pd->rhiCanvas != d->rhiCanvas
        || pd->fillColor != d->fillColor)
    {
        return false;
    }

    return true;
}

/*!
    If multiple canvases share common data, this image makes a copy of the data
    and detaches itself from the sharing mechanism, making sure that this image
    is the only one referring to the data.

    Nothing is done if there is just a single reference.

    \note The canvas only contains handles to the underlying graphics resources,
    and data such as the fill color. The graphics resources themselves, such as
    the QRhiTexture, are never copied or duplicated.
 */
void QCOffscreenCanvas::detach()
{
    if (d)
        d.detach();
    else
        d = new QCOffscreenCanvasPrivate;
}

/*!
    \return true if this canvas has valid data.

    Normally this will always be true.

    \sa texture()
 */
bool QCOffscreenCanvas::isNull() const
{
    return d->rhiCanvas.isNull();
}

/*!
    \return the flags with which the canvas was created.

    The flags are immutable and cannot be changed once the canvas has been
    created.
 */
QCOffscreenCanvas::Flags QCOffscreenCanvas::flags() const
{
    return d->rhiCanvas.flags;
}

/*!
    \return the current fill color.
 */
QColor QCOffscreenCanvas::fillColor() const
{
    return d->fillColor;
}

/*!
    Sets the fill (clear) color to \a color.

    By default this is set to the value passed to
    \l{QCPainter::}{createCanvas()}.
 */
void QCOffscreenCanvas::setFillColor(const QColor &color)
{
    if (d->fillColor == color)
        return;

    detach();
    d->fillColor = color;
}

/*!
    \return the texture backing this offscreen canvas.

    \sa isNull()
 */
QRhiTexture *QCOffscreenCanvas::texture() const
{
    return d->rhiCanvas.tex;
}

QT_END_NAMESPACE
