// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qquickcpainterrenderer_p.h"
#include "qcpainter_p.h"
#include "qcpainterfactory_p.h"
#include "qcrhipaintdriver.h"
#include "qquickcpainteritem_p.h"
#include "engine/qcpainterengine_p.h"

#include <QDebug>
#include <QQuickWindow>
#include <QtMath>

QT_BEGIN_NAMESPACE

// As in, any item has updated its content since this flag was reset, including
// all QQuickWindows, and so all render threads.
QAtomicInt QQuickCPainterRendererPrivate::m_rendered;

/*!
    \class QQuickCPainterRenderer
    \brief The QQuickCPainterRenderer handles all painting of a QQuickCPainterItem.
    \inmodule QtCanvasPainter

    TODO: Write more documentation here.
*/


/*!
    Constructs a QQuickCPainterRenderer.
*/

QQuickCPainterRenderer::QQuickCPainterRenderer()
    : d_ptr(new QQuickCPainterRendererPrivate(this))
{
}

/*!
    Destroys the QQuickCPainterRenderer.
*/

QQuickCPainterRenderer::~QQuickCPainterRenderer()
{
    Q_D(QQuickCPainterRenderer);
    if (d->m_factory && !d->m_sharedPainter)
        delete d->m_factory;

    QObject::disconnect(d->m_sharedPainterNewFrameConn);

    delete d_ptr;
}


/*!
    \fn void QQuickCPainterRenderer::initializeResources(QCPainter *painter)

    Reimplement this method to initialize resources using \a painter. This will
    be called once before the first synchronize() and when ever resources have
    been dropped and need to be initialized / added again.

    \sa QCPainter::addImage
*/

void QQuickCPainterRenderer::initializeResources(QCPainter *painter)
{
    Q_UNUSED(painter);
}

void QQuickCPainterRenderer::prePaint(QCPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    \fn void QQuickCPainterRenderer::paint(QCPainter *painter)

    Reimplement this method to paint using \a painter.

    This will get called after the item has been filled with fillColor().

    Paint is called from renderer thread, to access item data do it in
    synchronize().

    \sa synchronize()
*/

void QQuickCPainterRenderer::paint(QCPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    \fn void QQuickCPainterRenderer::synchronize(QQuickCPainterItem *item)

    Reimplement this method to synchronize data between \a item and
    item painter instances. This will be called before paint() each
    time item needs to be repainted.

    This method is the only place where it is safe for the painter and the
    item to read and write each others variables.

    Usually you should static_cast \a item to your real item type, and then
    exchange the data.
*/

void QQuickCPainterRenderer::synchronize(QQuickCPainterItem *item)
{
    Q_UNUSED(item);
}

/*!
    \fn QColor QQuickCPainterRenderer::fillColor() const

    Return the current fill color of the item. This can be set
    by the parent QQuickCPainterItem.

*/

QColor QQuickCPainterRenderer::fillColor() const
{
    Q_D(const QQuickCPainterRenderer);
    return d->m_fillColor;
}

bool QQuickCPainterRenderer::hasSharedPainter() const
{
    Q_D(const QQuickCPainterRenderer);
    return d->m_sharedPainter;
}

// Must be called early enough, e.g. from the derived class' constructor, must
// not be changed afterwards. Hence not a property exposed on the item, it is an
// implementation detail.
// NOTE: The default is true, and even then items in different
// windows, meaning different QRhis, are still going to use different drivers
// (painter/engine/renderer). Items with the same QRhi (in the same
// QQuickWindow) will use the same painter, however. There are consequences and
// pros/cons to both.
void QQuickCPainterRenderer::setSharedPainter(bool enable)
{
    Q_D(QQuickCPainterRenderer);
    d->m_sharedPainter = enable;
}

/*!
    \fn QCPainter *QQuickCPainterRenderer::painter() const

    Returns the painter attached to this painter item.
*/
QCPainter *QQuickCPainterRenderer::painter() const
{
    Q_D(const QQuickCPainterRenderer);
    return d->m_factory && d->m_factory->isValid() ? d->m_factory->painter() : nullptr;
}

/*!
    \fn float QQuickCPainterRenderer::width() const

    Returns the width of the painted area. This is usually the
    same as the painter item width, but if \l QQuickRhiItem::fixedColorBufferWidth
    has been set, width equals to this buffer width (with
    the \l{QQuickWindow::effectiveDevicePixelRatio()}{device pixel ratio} taken
    into account).
*/
float QQuickCPainterRenderer::width() const {
    Q_D(const QQuickCPainterRenderer);
    return float(d->m_itemData.width);
}

/*!
    \fn float QQuickCPainterRenderer::height() const

    Returns the height of the painted area. This is usually the
    same as the painter item height, but if \l QQuickRhiItem::fixedColorBufferHeight
    has been set, height equals to this buffer height (with
    the \l{QQuickWindow::effectiveDevicePixelRatio()}{device pixel ratio} taken
    into account).
*/
float QQuickCPainterRenderer::height() const {
    Q_D(const QQuickCPainterRenderer);
    return float(d->m_itemData.height);
}

/*!
   \internal
*/

void QQuickCPainterRenderer::synchronize(QQuickRhiItem * item)
{
    Q_D(QQuickCPainterRenderer);
    if (!item)
        return;

    d->m_window = item->window();
    QQuickCPainterItem *realItem = static_cast<QQuickCPainterItem*>(item);
    if (!realItem)
        return;

    // Make sure initialization has been done
    if (!d->m_initialized)
        initialize(nullptr);

    auto *painterPriv = QCPainterPrivate::get(d->m_factory->painter());
    painterPriv->handleCleanupTextures();
    d->m_fillColor = realItem->fillColor();
    bool antialiasing = realItem->antialiasing();
    if (antialiasing != d->m_antialiasing) {
        d->m_antialiasing = antialiasing;
        auto *painter = d->m_factory->painter();
        painter->setRenderHint(QCPainter::RenderHint::Antialiasing, antialiasing);
    }

    realItem->d_func()->setBackendName(painterPriv->m_rhiBackendName);

    static bool collectDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_COLLECT");
    static bool renderDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_RENDER");

    if (collectDebug && d->m_renderedOnce)
        realItem->d_func()->updateDebugData(d->m_drawDebug);
    if (renderDebug)
        d->m_debug.start();

    if (!d->m_synchronized)
        initializeResources(d->m_factory->painter());
    synchronize(realItem);
    d->m_synchronized = true;
}

QQuickCPainterRendererPrivate::QQuickCPainterRendererPrivate(QQuickCPainterRenderer *q)
    : q_ptr(q)
{
}

QQuickCPainterRendererPrivate::~QQuickCPainterRendererPrivate()
{
}

void QQuickCPainterRenderer::initialize(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
    Q_D(QQuickCPainterRenderer);
    if (!d->m_window)
        return;

    if (!d->m_rhi || rhi() != d->m_rhi) {
        d->m_rhi = rhi();
        if (!d->m_sharedPainter) {
            if (!d->m_factory)
                d->m_factory = new QCPainterFactory;
            else
                d->m_factory->destroy();
        } else {
            d->m_factory = QCPainterFactory::sharedInstance(d->m_rhi);
            d->m_sharedPainterNewFrameConn = QObject::connect(d->m_window, &QQuickWindow::beforeFrameBegin, [this] {
                Q_D(QQuickCPainterRenderer);
                if (QQuickCPainterRendererPrivate::m_rendered.testAndSetAcquire(1, 0))
                    d->m_factory->paintDriver()->resetForNewFrame();
            });
        }
        if (!d->m_factory->isValid())
            d->m_factory->create(d->m_rhi);
        QCPainterPrivate::get(d->m_factory->painter())->updateBackendName(d->m_rhi);
        d->m_renderedOnce = false;
    }

    if (renderTarget() != d->m_rt) {
        if (d->m_rt) {
            // in case the QRhiRenderPassDescriptor is incompatible with the new rt
            if (d->m_factory && !d->m_sharedPainter) {
                d->m_factory->destroy();
                d->m_factory->create(d->m_rhi);
                d->m_renderedOnce = false;
            } // cannot do anything for the shared painter case
        }
        d->m_rt = renderTarget();
    }

    // Set dpr and width & height from the colorTexture.
    const qreal dpr = d->m_window->effectiveDevicePixelRatio();
    d->m_itemData.devicePixelRatio = float(dpr);
    if (auto ct = colorTexture()) {
        d->m_itemData.width = int(ct->pixelSize().width() / dpr);
        d->m_itemData.height = int(ct->pixelSize().height() / dpr);
    } else if (auto msaacb = msaaColorBuffer()) {
        d->m_itemData.width = int(msaacb->pixelSize().width() / dpr);
        d->m_itemData.height = int(msaacb->pixelSize().height() / dpr);
    }

    d->m_initialized = true;
}

void QQuickCPainterRenderer::render(QRhiCommandBuffer *cb)
{
    Q_D(QQuickCPainterRenderer);
    if (!d->m_initialized)
        return;

    QCRhiPaintDriver *pd = d->m_factory->paintDriver();
    QCPainter *painter = d->m_factory->painter();

    QCPainterPrivate::get(painter)->m_devicePixelRatio = d->m_itemData.devicePixelRatio;

    if (!d->m_sharedPainter)
        pd->resetForNewFrame();
    // otherwise this is hooked up to the QQuickWindow's signal

    if ((d->m_itemData.width > 0 && d->m_itemData.height > 0)) {
        d->m_renderedOnce = true;
#ifdef QCPAINTER_PERF_DEBUG
        QCPainterFactoryPrivate::get(d->m_factory)->renderer.engine()->perfLogger()->logStart(QCPerfLogging::PAINT);
#endif

        prePaint(painter);

        const float dpr = d->m_itemData.devicePixelRatio;
        const QSize outputLogicalSize = d->m_rt->pixelSize() / dpr;
        pd->beginPaint(cb, d->m_rt, d->m_fillColor, outputLogicalSize, dpr);

        d->m_currentCb = cb;
        paint(painter);
        d->m_currentCb = nullptr;

        static bool collectDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_COLLECT");
        static bool renderDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_RENDER");
        if (collectDebug)
            d->m_drawDebug = QCPainterFactoryPrivate::get(d->m_factory)->renderer.engine()->drawDebug();
        if (renderDebug) {
            d->m_debug.paintDrawDebug(painter, width(), height());
            // Re-render once to show the initial rendering data.
            if (d->m_firstRender) {
                update();
                d->m_firstRender = false;
            }
        }

        pd->endPaint();

#ifdef QCPAINTER_PERF_DEBUG
        QCPainterFactoryPrivate::get(d->m_factory)->renderer.engine()->perfLogger()->logEnd(QCPerfLogging::PAINT);
#endif

        QQuickCPainterRendererPrivate::m_rendered.storeRelease(1);
    }
}

void QQuickCPainterRenderer::beginCanvasPainting(QCCanvas &canvas)
{
    Q_D(QQuickCPainterRenderer);
    if (!d->m_currentCb) {
        qWarning("beginCanvasPainting() can only be called from prePaint()");
        return;
    }

    d->m_factory->paintDriver()->beginPaint(canvas, d->m_currentCb);
}

void QQuickCPainterRenderer::endCanvasPainting()
{
    Q_D(QQuickCPainterRenderer);
    if (!d->m_currentCb)
        return;

    d->m_factory->paintDriver()->endPaint();
}

void QQuickCPainterRenderer::grabCanvas(const QCCanvas &canvas, std::function<void(const QImage &)> callback)
{
    Q_D(QQuickCPainterRenderer);
    d->m_factory->paintDriver()->grabCanvas(canvas, callback);
}

QT_END_NAMESPACE
