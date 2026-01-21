// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCPAINTERFACTORY_H
#define QCPAINTERFACTORY_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qsize.h>
#include <functional>
#include <QtCanvasPainter/qcoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCPainter;
class QCPainterFactoryPrivate;
class QCRhiPaintDriver;
class QRhi;

class Q_CANVASPAINTER_EXPORT QCPainterFactory
{
public:
    QCPainterFactory();
    ~QCPainterFactory();

    static QCPainterFactory *sharedInstance(QRhi *rhi);

    bool isValid() const;
    QCPainter *create(QRhi *rhi);
    void destroy();
    QCPainter *painter();

    QCRhiPaintDriver *paintDriver();

private:
    Q_DISABLE_COPY(QCPainterFactory)
    QCPainterFactoryPrivate *d = nullptr;
    friend class QCPainterFactoryPrivate;
};

QT_END_NAMESPACE

#endif // QCPAINTERFACTORY_H
