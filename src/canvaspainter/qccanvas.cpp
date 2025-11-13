// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qccanvas_p.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCCanvas
    \brief QCCanvas represents an offscreen canvas that QCPainter can target.
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

QCCanvas::QCCanvas()
    : d(new QCCanvasPrivate)
{
}

QCCanvas::QCCanvas(const QCCanvas &canvas) noexcept
    : d(canvas.d)
{
}

QCCanvas::~QCCanvas() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCCanvasPrivate)

QCCanvas &QCCanvas::operator=(const QCCanvas &canvas) noexcept
{
    QCCanvas(canvas).swap(*this);
    return *this;
}

bool QCCanvas::operator==(const QCCanvas &i) const
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

void QCCanvas::detach()
{
    if (d)
        d.detach();
    else
        d = new QCCanvasPrivate;
}

bool QCCanvas::isNull() const
{
    return d->rhiCanvas.isNull();
}

QCCanvas::Flags QCCanvas::flags() const
{
    return d->rhiCanvas.flags;
}

QColor QCCanvas::fillColor() const
{
    return d->fillColor;
}

void QCCanvas::setFillColor(const QColor &color)
{
    if (d->fillColor == color)
        return;

    detach();
    d->fillColor = color;
}

QRhiTexture *QCCanvas::texture() const
{
    return d->rhiCanvas.tex;
}

QT_END_NAMESPACE
