// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcpainterwidget_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"
#include "engine/qcpainterrhirenderer_p.h"
#include "qcpainterfactory.h"
#include "qcrhipaintdriver.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

// Does not really need to be atomic since widgets operate on gui thread, but for consistency with the Quick item.
QAtomicInt QCPainterWidgetPrivate::m_rendered;


/*!
    \class QCPainterWidget
    \brief QCPainterWidget is a widget for rendering using QCPainter.
    \inmodule QtCanvasPainter

    Implement the \l paint virtual function in a subclass to perform rendering
    using a QCPainter.

    The below code snippet shows the typical structure of a QCPainterWidget
    subclass:

    \snippet widget-ex-1.cpp 0
*/

/*!
   Constructs a QCPainterWidget with the given \a parent.
 */

QCPainterWidget::QCPainterWidget(QWidget *parent)
    : QRhiWidget(*new QCPainterWidgetPrivate, parent)
{
}

/*!
   Destroys the QCPainterWidget.
 */
QCPainterWidget::~QCPainterWidget()
{
    Q_D(QCPainterWidget);
    if (!d->m_sharedPainter) {
        // Must be destroyed here, not in the Private dtor, see ~QRhiWidget().
        delete d->m_factory;
    }
}

/*!
   \reimp
*/
void QCPainterWidget::releaseResources()
{
    Q_D(QCPainterWidget);

    // Happens f.ex. when the widget is moved to a new top-level window. The
    // QRhi is then different, and there will be an initialize() following this.
    // We'll need a new renderer then.

    graphicsResourcesInvalidated();

    if (!d->m_sharedPainter)
        d->m_factory->destroy();
    else
        d->m_factory = nullptr;
}

/*!
    \fn QColor QCPainterWidget::fillColor() const

    Returns the current fill color.

    \sa setFillColor()
*/

QColor QCPainterWidget::fillColor() const
{
    Q_D(const QCPainterWidget);
    return d->m_fillColor;
}

/*!
    \fn void QCPainterWidget::setFillColor (const QColor &color)

    Set the fill color to \a color. This color will be used to draw
    the background of the item. The default color is black.

    \sa fillColor()
*/

void QCPainterWidget::setFillColor(const QColor &color)
{
    Q_D(QCPainterWidget);
    if (d->m_fillColor == color)
        return;
    d->m_fillColor = color;

    update();
}

/*!
    Returns \c true if this widget uses a shared painter.

    \sa setSharedPainter
 */
bool QCPainterWidget::hasSharedPainter() const
{
    Q_D(const QCPainterWidget);
    return d->m_sharedPainter;
}

/*!
    Disable painter sharing if \a enable is \c false.

    If painter sharing is enabled, all QCPainterWidget instances inside the same QWindow will use
    the same QCPainter. This function must be early, e.g. from the derived class' constructor,
    and must not be changed afterwards.

    Painter sharing is enabled by default.

    If two widgets use dedicated, non-shared painters, each other's graphics
    resources, such as the ones backing QCImage or QOffscreenCanvas, will not be
    visible to them. Whereas if the widgets are in the same window, and sharing
    is enabled, they can use images or canvases created by the other widget,
    because they both use the same QCPainter.

    \note Even when \a enable is true, painters are not shared between
    widgets belonging to different windows (top-level widgets).

    \sa hasSharedPainter
 */
void QCPainterWidget::setSharedPainter(bool enable)
{
    Q_D(QCPainterWidget);
    d->m_sharedPainter = enable;
}

/*!
   \reimp
*/
void QCPainterWidget::initialize(QRhiCommandBuffer *)
{
    Q_D(QCPainterWidget);
    bool callInitRes = false;
    if (!d->m_factory) {
        callInitRes = true;
        if (!d->m_sharedPainter) {
            d->m_factory = new QCPainterFactory;
        } else {
            // m_factory here is not owned by us, and is valid as long as the QRhi lives
            d->m_factory = QCPainterFactory::sharedInstance(rhi());
            connect(this, &QRhiWidget::frameSubmitted, this, [this] {
                Q_D(QCPainterWidget);
                if (QCPainterWidgetPrivate::m_rendered.testAndSetAcquire(1, 0))
                    d->m_factory->paintDriver()->resetForNewFrame();
            });
        }
    }

    if (!d->m_factory->isValid()) {
        d->m_factory->create(rhi());
        callInitRes = true;
    }

    if (callInitRes && d->m_factory->isValid())
        initializeResources(d->m_factory->painter());
}

/*!
   \reimp
*/
void QCPainterWidget::render(QRhiCommandBuffer *cb)
{
    Q_D(QCPainterWidget);
    if (!d->m_factory || !d->m_factory->isValid())
        return;

    d->m_currentCb = cb;

    static bool renderDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_RENDER");
    if (renderDebug)
        d->m_debug.start();

    QCRhiPaintDriver *pd = d->m_factory->paintDriver();
    QCPainter *painter = d->m_factory->painter();

    if (!d->m_sharedPainter)
        pd->resetForNewFrame();

    prePaint(painter);

    pd->beginPaint(cb, renderTarget(), d->m_fillColor, size(), devicePixelRatio());

    paint(painter);

    if (renderDebug) {
        d->m_debug.paintDrawDebug(painter, width(), height());
        // Re-render once to show the initial rendering data.
        if (d->m_firstRender) {
            update();
            d->m_firstRender = false;
        }
    }

    pd->endPaint();

    d->m_currentCb = nullptr;

    QCPainterWidgetPrivate::m_rendered.storeRelease(1);
}

/*!
    Reimplement this method to initialize resources using \a painter. Generally,
    this will be called once before the first prePaint() and paint(). An
    exception is when graphics resources are lost, see
    graphicsResourcesInvalidated(). In that case, this method will get invoked
    again afterwards.

    The default implementation is empty.
*/
void QCPainterWidget::initializeResources(QCPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Reimplement this function to perform drawing into one or more offscreen
    canvases using \a painter.

    The default implementation is empty.

    \sa beginCanvasPainting(), endCanvasPainting()
 */
void QCPainterWidget::prePaint(QCPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Reimplement this method to paint using \a painter.

    The widget is first filled with fillColor().

    The default implementation is empty.
 */
void QCPainterWidget::paint(QCPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Called when underlying graphics resources, such as textures, are lost.

    This indicates that QCImage objects returned from addImage() are no longer
    valid, and addImage() needs to be called again. If the paint()
    implementation is such that this does not matter, for example because images
    are not used, or addImage() is always called, then no action is necessary.
    Otherwise, it is recommended to toggle a flag, or similar, and act
    accordingly in the next invocation of paint().

    The same applies to QCOffscreenCanvas objects returned from
    QCPainter::createCanvas(). When this function is called, the next invocation
    of paint() should create new canvases and redraw their contents.

    Graphics resources can be lost, for example, when the widget is moved to a
    new top-level window, because that implies being associated with a new QRhi
    instance.

    \sa QRhiWidget::releaseResources()
 */
void QCPainterWidget::graphicsResourcesInvalidated()
{
}

/*!
    Starts recording QCPainter draw commands targeting \a canvas.

    \note This function should only be called from prePaint().

    beginCanvasPainting() must always be followed by corresponding
    endCanvasPainting() before returning from prePaint().

    The following snippet from a QCPainterWidget subclass shows how an offscreen
    canvas could be rendered into and then used as an image or image pattern
    when drawing the contents for the widget:

    \snippet widget-canvas-ex-1.cpp 0
 */
void QCPainterWidget::beginCanvasPainting(QCOffscreenCanvas &canvas)
{
    Q_D(QCPainterWidget);
    if (!d->m_currentCb) {
        qWarning("beginCanvasPainting() can only be called from prePaint()");
        return;
    }

    d->m_factory->paintDriver()->beginPaint(canvas, d->m_currentCb);
}

/*!
    Indicates the end of the drawing targeting the canvas specified in
    beginCanvasPainting().

    \note This function should only be called from prePaint().

    beginCanvasPainting() must always be followed by corresponding
    endCanvasPainting() before returning from prePaint().
 */
void QCPainterWidget::endCanvasPainting()
{
    Q_D(QCPainterWidget);
    if (!d->m_currentCb)
        return;

    d->m_factory->paintDriver()->endPaint();
}

/*!
    Issues a texture readback request for \a canvas.

    \a callback is invoked either before the function returns, or later,
    depending on the underlying QRhi and 3D API implementation. Reading back
    texture contents may involve a GPU->CPU copy, depending on the GPU
    architecture.
 */
void QCPainterWidget::grabCanvas(const QCOffscreenCanvas &canvas, std::function<void(const QImage &)> callback)
{
    Q_D(QCPainterWidget);
    d->m_factory->paintDriver()->grabCanvas(canvas, callback);
}

QT_END_NAMESPACE
