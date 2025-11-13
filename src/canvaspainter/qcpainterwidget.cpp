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

QCPainterWidget::QCPainterWidget(QWidget *parent)
    : QRhiWidget(*new QCPainterWidgetPrivate, parent)
{
}

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
    \fn void QCPainterWidget::paint(QCPainter *painter)

    Reimplement this method to paint using \a painter.

    This will get called after the item has been filled with fillColor().
*/

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
    the background of the item. The default color is transparent.

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

bool QCPainterWidget::hasSharedPainter() const
{
    Q_D(const QCPainterWidget);
    return d->m_sharedPainter;
}

// Must be called early enough, e.g. from the derived class' constructor, must
// not be changed afterwards.
// NOTE: The default is true, and even then widgets in
// different windows, meaning different QRhis, are still going to use different
// drivers (painter/engine/renderer). Widgets with the same QRhi (in the same
// window) will use the same painter, however. There are consequences and
// pros/cons to both.
void QCPainterWidget::setSharedPainter(bool enable)
{
    Q_D(QCPainterWidget);
    d->m_sharedPainter = enable;
}

/*!
   \internal
*/

void QCPainterWidget::initialize(QRhiCommandBuffer *)
{
    Q_D(QCPainterWidget);
    if (!d->m_factory) {
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

    if (!d->m_factory->isValid())
        d->m_factory->create(rhi());
}

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

void QCPainterWidget::prePaint(QCPainter *painter)
{
    Q_UNUSED(painter);
}

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

    The same applies to QCCanvas objects returned from
    QCPainter::createCanvas(). When this function is called, the next invocation
    of paint() should create new canvases and redraw their contents.

    \sa QRhiWidget::releaseResources()
 */
void QCPainterWidget::graphicsResourcesInvalidated()
{
}

void QCPainterWidget::beginCanvasPainting(QCCanvas &canvas)
{
    Q_D(QCPainterWidget);
    if (!d->m_currentCb) {
        qWarning("beginCanvasPainting() can only be called from prePaint()");
        return;
    }

    d->m_factory->paintDriver()->beginPaint(canvas, d->m_currentCb);
}

void QCPainterWidget::endCanvasPainting()
{
    Q_D(QCPainterWidget);
    if (!d->m_currentCb)
        return;

    d->m_factory->paintDriver()->endPaint();
}

void QCPainterWidget::grabCanvas(const QCCanvas &canvas, std::function<void(const QImage &)> callback)
{
    Q_D(QCPainterWidget);
    d->m_factory->paintDriver()->grabCanvas(canvas, callback);
}

QT_END_NAMESPACE
