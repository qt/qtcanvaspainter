// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcpainterfactory_p.h"
#include "qcrhipaintdriver_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

#include <QMutex>
#include <QMutexLocker>

QT_BEGIN_NAMESPACE

/*!
    \class QCPainterFactory
    \brief QCPainterFactory manages instances of QCPainter and the associated rendering engine.
    \inmodule QtCanvasPainter

    Applications rendering via QCPainter do not create instances of the painter
    class themselves, but rather are expected to use a QCPainterFactory.

    \note This class is relevant only when working with QCPainter \b without a
    convenience class such as QCPainterWidget or QQuickCPainterItem, because
    those provide a QCPainter instance to the application.

    The following is an example code snippet of getting a QCPainter for use with
    an existing, successfully initialized QRhi:

    \code
        std::unique_ptr<QCPainterFactory> factory(new QCPainterFactory);
        QCPainter *painter = factory->create(rhi);
        QCRhiPaintDriver *paintDriver = factory->paintDriver();
    \endcode

    All drawing code that operates on the same thread with the same QRhi is
    recommended to share and reuse the same QCPainter, instead of having a
    dedicated painter (and so factory) in each component. This can be achieved
    by calling sharedInstance(), instead of constructing a new QCPainterFactory:

    \code
        QCPainterFactory *factory = QCPainterFactory::sharedInstance(rhi);
        QCPainter *painter = factory->painter();
        QCRhiPaintDriver *paintDriver = factory->paintDriver();
    \endcode
 */

struct QCPainterFactoryGlobal
{
    QHash<QRhi *, QCPainterFactory *> h;
};

Q_GLOBAL_STATIC(QCPainterFactoryGlobal, qcpd_g);

/*!
    \return a QCPainterFactory instance associated with the given \a rhi.

    Calling this function with the same \a rhi will always return the same
    QCPainterFactory.

    There is no need to call create() on the returned factory, and the \l
    painter() can be queried right away, because the result is always already
    initialized, meaning isValid() returns true.

    This function is thread safe. However, the QRhi threading rules apply as
    usual: a QRhi object, and by extension, an instance of QCPainterFactory,
    QCRhiPaintDriver, and QCPainter associated with that QRhi are all expected
    to be used on the same one single thread.

    \note The returned QCPainterFactory is not owned by the caller, and it
    becomes invalid when \a rhi is destroyed.

    \sa painter()
 */
QCPainterFactory *QCPainterFactory::sharedInstance(QRhi *rhi)
{
    static QMutex m;
    QMutexLocker lck(&m);

    auto &h(qcpd_g()->h);
    auto it = h.constFind(rhi);
    if (it != h.constEnd())
        return it.value();

    QCPainterFactory *pd = new QCPainterFactory;
    pd->create(rhi);
    h.insert(rhi, pd);

    static auto cleanupFunc = [](QRhi *rhiAboutToDie) {
        auto &h(qcpd_g()->h);
        auto it = h.find(rhiAboutToDie);
        if (it != h.end()) {
            delete it.value();
            h.erase(it);
        }
    };
    rhi->addCleanupCallback(cleanupFunc);

    return pd;
}

/*!
    Constructor.

    \sa sharedInstance()
 */
QCPainterFactory::QCPainterFactory()
    : d(new QCPainterFactoryPrivate)
{
}

/*
    Destructor.
 */
QCPainterFactory::~QCPainterFactory()
{
    destroy();
    delete d;
}

/*!
    \return true if the create() has ben called successfully.
 */
bool QCPainterFactory::isValid() const
{
    return d->painter && d->renderer.isValid();
}

/*!
    \return the painter, or null if isValid() is false.

    \note The returned object is not owned by the caller.
 */
QCPainter *QCPainterFactory::painter()
{
    return d->painter.get();
}

/*!
    \return the paint driver object, or null if isValid() is false.

    \note The returned object is not owned by the caller.
 */
QCRhiPaintDriver *QCPainterFactory::paintDriver()
{
    return d->paintDriver.get();
}

/*!
    Initializes the QCPainter rendering infrastructure, if it has not been done
    already for this factory.

    \return the painter, or null if the renderer could not be initialized.

    The factory will be associated with \a rhi, and it cannot be used with any
    other QRhi, unless destroy(), and then create() are called again.

    Repeated calls to this function have no effect and are harmless. Call
    destroy() first if a reinitialization of the painter infrastructure is
    desired, possibly with a different \a rhi.

    \note There is no need to call this function when sharedInstance() was used
    to retrieve a factory instance.

    \sa destroy(), isValid(), painter()
 */
QCPainter *QCPainterFactory::create(QRhi *rhi)
{
    if (!d->paintDriver)
        d->paintDriver.reset(new QCRhiPaintDriver);

    if (!d->painter)
        d->painter.reset(new QCPainter);

    if (!d->renderer.isValid()) {
        // Each painter owns an engine, and each painter+engine combo is associated with one renderer at a time.
        // The renderer we have to manage (create, destroy), and it is per-QRhi.
        d->renderer.create(rhi, d->painter.get());
    }

    if (!d->renderer.isValid()) {
        qWarning("QCPainterFactory::create() failed to initialize renderer");
        return nullptr;
    }

    QCRhiPaintDriverPrivate *pd = QCRhiPaintDriverPrivate::get(d->paintDriver.get());
    pd->painter = d->painter.get();
    pd->renderer = &d->renderer;
    pd->currentCb = nullptr;
    pd->currentRt = nullptr;

    return d->painter.get();
}

/*!
    Tears down the rendering infrastructure. Normally there is no need to call
    this function. Rather, it is used in situations where it will be followed by
    a create().

    \sa create()
 */
void QCPainterFactory::destroy()
{
    if (d->painter)
        QCPainterPrivate::get(d->painter.get())->clearTextureCache();

    if (d->renderer.isValid())
        d->renderer.destroy();

    // Can be followed by a new call to create(). d->painter and its engine stay intact.
}

QT_END_NAMESPACE
