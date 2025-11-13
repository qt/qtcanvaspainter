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
    class themselves, but rather are expected to use a QCPainterFactory. This is
    relevant only when working with QCPainter without a convenience class such
    as QCPainterWidget or QQuickCPainterItem, because those provide a QCPainter
    instance to the application.

    All drawing code that operates on the same thread and is using the same QRhi
    underneath is recommended to share and reuse the same QCPainter, instead of
    having a dedicated painter (and so factory) in each component. This can be achieved
    by calling sharedInstance(), instead of constructing a new QCPainterFactory.
 */

struct QCPainterFactoryGlobal
{
    QHash<QRhi *, QCPainterFactory *> h;
};

Q_GLOBAL_STATIC(QCPainterFactoryGlobal, qcpd_g);

// what's returned from here needs no create(), although it is nor harmful either to call it
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

QCPainterFactory::QCPainterFactory()
    : d(new QCPainterFactoryPrivate)
{
}

QCPainterFactory::~QCPainterFactory()
{
    destroy();
    delete d;
}

bool QCPainterFactory::isValid() const
{
    return d->painter && d->renderer.isValid();
}

QCPainter *QCPainterFactory::painter()
{
    return d->painter.get();
}

QCRhiPaintDriver *QCPainterFactory::paintDriver()
{
    return d->paintDriver.get();
}

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

    QCRhiPaintDriverPrivate *pd = QCRhiPaintDriverPrivate::get(d->paintDriver.get());
    pd->painter = d->painter.get();
    pd->renderer = &d->renderer;
    pd->currentCb = nullptr;
    pd->currentRt = nullptr;

    return d->painter.get();
}

void QCPainterFactory::destroy()
{
    if (d->painter)
        QCPainterPrivate::get(d->painter.get())->clearTextureCache();

    if (d->renderer.isValid())
        d->renderer.destroy();

    // Can be followed by a new call to create(). d->painter and its engine stay intact.
}

QT_END_NAMESPACE
