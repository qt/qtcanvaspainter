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

    Applications wishing to render with QCPainter onto a QRhi-based render
    target, such as into a QRhiTexture, or to the color buffer of a
    QRhiSwapChain, use QCPainterFactory to initialize and retrieve a QCPainter
    and QCRhiPaintDriver associated with the QRhi. The drawing API is provided
    by QCPainter, while the low-level aspects of rendering (what is the render
    target, what is the command buffer, etc.) are controlled by
    QCRhiPaintDriver.

    \note This class is relevant only when working with QCPainter \b without a
    convenience class such as QCPainterWidget or QQuickCPainterItem, because
    those provide a QCPainter instance to the application and manage its
    rendering implicitly.

    Applications do not create instances of QCRhiPaintDriver themselves. Rather,
    retrieve it from a successfully \l{QCPainterFactory::create()}{initialized}
    QCPainterFactory by calling
    \l{QCPainterFactory::paintDriver()}{paintDriver()}.

    The following is an almost complete, standalone console application that
    draws a circle into a QRhiTexture, reads back the result, and saves it into
    a PNG file:

    \snippet paintdriver-ex-1.cpp 0
 */

/*!
    \internal
*/
QCRhiPaintDriver::QCRhiPaintDriver()
    : d(new QCRhiPaintDriverPrivate)
{
}

/*!
    \internal
*/
QCRhiPaintDriver::~QCRhiPaintDriver()
{
    delete d;
}

/*!
    Resets the painter engine state. This function is expected to be called once
    when starting a whole new frame, typically after \l QRhi::beginFrame() or \l
    QRhi::beginOffscreenFrame().
 */
void QCRhiPaintDriver::resetForNewFrame()
{
    d->renderer->resetForNewFrame();
}

/*!
    Begins painting onto the render target \a rt, recording rendering commands
    to the command buffer \a cb.

    \a fillColor specifies the color used for clearing the color buffer.

    \note A beginPaint() must always be followed by an endPaint(). Nesting is
    not currently supported.

    \note \a rt is expected to have both a color and depth-stencil attachment.
    In case there are multiple color attachments, only the color buffer for
    attachment 0 is written. QCPainter requires the presence of a depth-stencil
    buffer. Currently only stencil is utilized, depth testing and writing are
    always disabled.

    \a logicalSize is optional. When non-empty, it specifies the viewport size
    in logical units. \a dpr must then specify the scale factor (device pixel
    ratio) so that \a logicalSize can be converted to pixels internally. In
    practice this will rarely be needed, since the render target's size is used
    automatically by default.

    \note The associated QRhi must be recording a frame (\l QRhi::beginFrame()
    or \l QRhi::beginOffscreenFrame() must have been called), but it should not
    be in render pass recording state when this function is called.

    \overload
 */
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

/*!
    Begins painting onto the specified offscreen \a canvas, recording rendering
    commands to the command buffer \a cb.

    \note A beginPaint() must always be followed by an endPaint(). Nesting is
    not currently supported.

    Clearing happens with the \l{QCOffscreenCanvas::setFillColor()}{fill color
    set on the canvas}, unless the QCOffscreenCanvas::Flag::PreserveContents
    flag is set, in which case there is no clear and the canvas contents is
    preserved (with potential implications for performance, depending on the
    underlying GPU architecture).

    \note The associated QRhi must be recording a frame (\l QRhi::beginFrame()
    or \l QRhi::beginOffscreenFrame() must have been called), but it should not
    be in render pass recording state when this function is called.

    \overload
 */
void QCRhiPaintDriver::beginPaint(QCOffscreenCanvas &canvas, QRhiCommandBuffer *cb)
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

/*!
    \enum QCRhiPaintDriver::EndPaintFlag

    Specifies the flags for endPaint().

    \value DoNotRecordRenderPass Indicates that flushing the QCPainter's
    generated commands and recording the QRhi render pass is not desired,
    because there will be an explicit call to renderPaint() afterwards.
 */

/*!
    Flushes and records all QRhi rendering generated by the QCPainter drawing commands.

    By default, this function records a full render pass, meaning it will
    internally call \l QRhiCommandBuffer::beginPass() and \l
    QRhiCommandBuffer::endPass(). The clear color is specified by the fillColor
    argument of beginPaint(), or the offscreen canvas's fill color.

    \a flags can be used to control the recording of a render pass, since in
    some cases it will not be desirable to let endPaint() issue an entire
    beginPass() - endPass() sequence.

    The following two snippets are identical when it comes to the results, but
    the sceond offers more flexibility, in case the application wants to do more
    within the same render pass:

    \code
        pd->beginPaint(cb, rt, Qt::black);
        // painter->...
        pd->endPaint();
    \endcode

    \code
        pd->beginPaint(cb, rt);
        // painter->...
        pd->endPaint(QCRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);

        cb->beginPass(rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
    \endcode

    \sa beginPaint(), renderPaint()
 */
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

/*!
    Records all QRhi rendering generated by the QCPainter drawing commands. Call
    this function only when endPaint() was invoked with
    \l{QCRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass}{DoNotRecordRenderPass}.
    It must not be called otherwise.

    The associated QRhi must be recording a render pass when this function is invoked.

    \sa endPaint()
 */
void QCRhiPaintDriver::renderPaint()
{
    d->renderer->render();
}

/*!
    Issues a texture readback request for \a canvas.

    \a callback is invoked either before the function returns, or later,
    depending on the underlying QRhi and 3D API implementation. Reading back
    texture contents may involve a GPU->CPU copy, depending on the GPU
    architecture.

    This function can be called both within a beginPaint() - endPaint() block,
    and outside. When called outside, it will internally invoke \l
    QRhi::beginOffscreenFrame() etc., allowing grabs to be performed at any
    time.
 */
void QCRhiPaintDriver::grabCanvas(const QCOffscreenCanvas &canvas, std::function<void(const QImage &)> callback)
{
    d->renderer->grabCanvas(canvas, callback, d->currentCb ? d->currentCb : nullptr);
}

QT_END_NAMESPACE
