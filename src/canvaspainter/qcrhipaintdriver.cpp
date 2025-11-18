// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcrhipaintdriver_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCRhiPaintDriver
    \brief The QCRhiPaintDriver class manages the lower level aspects of QCPainter-based rendering for QRhi render targets and offscreen canvases.
    \inmodule QtCanvasPainter
 */

QCRhiPaintDriver::QCRhiPaintDriver()
    : d(new QCRhiPaintDriverPrivate)
{
}

QCRhiPaintDriver::~QCRhiPaintDriver()
{
    delete d;
}

void QCRhiPaintDriver::resetForNewFrame()
{
    d->renderer->resetForNewFrame();
}

void QCRhiPaintDriver::beginPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QColor &fillColor, QSize logicalSize, float dpr)
{
    if (d->currentCb) {
        qWarning("Attempted to begin painting without ending the previous one first");
        return;
    }

    d->currentCb = cb;
    d->currentRt = rt;
    d->mainFillColor = fillColor;

    if (logicalSize.isEmpty()) {
        const float dpr = rt->devicePixelRatio();
        d->mainLogicalWidth = rt->pixelSize().width() / dpr;
        d->mainLogicalHeight = rt->pixelSize().height() / dpr;
        d->mainDpr = dpr;
    } else {
        d->mainLogicalWidth = float(logicalSize.width());
        d->mainLogicalHeight = float(logicalSize.height());
        d->mainDpr = dpr;
    }

    QCPainterPrivate::get(d->painter)->handleCleanupTextures();

    d->renderer->beginPrepareAndPaint(d->currentCb, d->currentRt, d->mainLogicalWidth, d->mainLogicalHeight, d->mainDpr);
}

void QCRhiPaintDriver::beginPaint(QCCanvas &canvas, QRhiCommandBuffer *cb)
{
    if (canvas.isNull()) {
        qWarning("Cannot paint on null canvas");
        return;
    }

    if (!d->currentCanvas.isNull()) {
        qWarning("Cannot begin canvas painting while another canvas is active");
        return;
    }

    if (d->currentCb) {
        qWarning("Cannot begin canvas painting when main painting pass is active");
        return;
    }

    d->currentCanvas = canvas;
    d->currentCb = cb;
    d->currentRt = d->renderer->canvasRenderTarget(canvas);

    d->renderer->beginPrepareAndPaint(d->currentCb, d->currentRt);
}

void QCRhiPaintDriver::endPaint(EndPaintFlags flags)
{
    if (d->currentCanvas.isNull()) {
        if (d->currentCb) {
            d->renderer->endPrepareAndPaint();
            if (!flags.testFlag(EndPaintFlag::DoNotRecordRenderPass))
                d->renderer->recordRenderPass(d->currentCb, d->currentRt, d->mainFillColor);
            d->currentCb = nullptr;
            d->currentRt = nullptr;
        } else {
            qWarning("endPaint() without matching beginPaint()");
        }
    } else {
        d->renderer->endPrepareAndPaint();
        if (!flags.testFlag(EndPaintFlag::DoNotRecordRenderPass))
            d->renderer->recordCanvasRenderPass(d->currentCb, d->currentCanvas);
        d->currentCanvas = {};
        d->currentCb = nullptr;
        d->currentRt = nullptr;
    }
}

void QCRhiPaintDriver::renderPaint()
{
    d->renderer->render();
}

void QCRhiPaintDriver::grabCanvas(const QCCanvas &canvas, std::function<void(const QImage &)> callback)
{
    d->renderer->grabCanvas(canvas, callback, d->currentCb ? d->currentCb : nullptr);
}

QT_END_NAMESPACE
