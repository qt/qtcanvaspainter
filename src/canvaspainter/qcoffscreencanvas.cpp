// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcoffscreencanvas_p.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCOffscreenCanvas
    \brief QCOffscreenCanvas represents an offscreen canvas that QCPainter can target.
    \inmodule QtCanvasPainter

    An offscreen canvas is essentially a color buffer (a QRhiTexture), and
    possibly other associated graphics resources. After content has been
    rendered into the canvas's color buffer, the canvas can be
    \l{QCPainter::addImage()}{registered} to the painter to retrieve a QCImage
    that can then be used in \l{QCPainter::drawImage()}{drawImage()} calls or as
    an \l{QCImagePattern}{image pattern}.

    To create an offscreen canvas, call \l QCPainter::createCanvas(). To target
    with with draw commands, call \l QRhiPaintDriver::beginPaint() when working
    with the lower level API, or \l QCPainterWidget::beginCanvasPainting() or \l
    QQuickCPainterRenderer::beginCanvasPainting() when using the QCPainter
    convenience widget and item implementations.
 */

QCOffscreenCanvas::QCOffscreenCanvas()
    : d(new QCOffscreenCanvasPrivate)
{
}

QCOffscreenCanvas::QCOffscreenCanvas(const QCOffscreenCanvas &canvas) noexcept
    : d(canvas.d)
{
}

QCOffscreenCanvas::~QCOffscreenCanvas() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCOffscreenCanvasPrivate)

QCOffscreenCanvas &QCOffscreenCanvas::operator=(const QCOffscreenCanvas &canvas) noexcept
{
    QCOffscreenCanvas(canvas).swap(*this);
    return *this;
}

bool QCOffscreenCanvas::operator==(const QCOffscreenCanvas &i) const
{
    if (i.d == d)
        return true;

    if (i.d->rhiCanvas != d->rhiCanvas
        || i.d->fillColor != d->fillColor)
    {
        return false;
    }

    return true;
}

void QCOffscreenCanvas::detach()
{
    if (d)
        d.detach();
    else
        d = new QCOffscreenCanvasPrivate;
}

bool QCOffscreenCanvas::isNull() const
{
    return d->rhiCanvas.isNull();
}

QCOffscreenCanvas::Flags QCOffscreenCanvas::flags() const
{
    return d->rhiCanvas.flags;
}

QColor QCOffscreenCanvas::fillColor() const
{
    return d->fillColor;
}

void QCOffscreenCanvas::setFillColor(const QColor &color)
{
    if (d->fillColor == color)
        return;

    detach();
    d->fillColor = color;
}

QRhiTexture *QCOffscreenCanvas::texture() const
{
    return d->rhiCanvas.tex;
}

QT_END_NAMESPACE
