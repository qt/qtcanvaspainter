// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCQUICKITEMPAINTER_H
#define QCQUICKITEMPAINTER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtQuick/qquickrhiitem.h>
#include <QtGui/qcolor.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCanvasPainter/qcpainter.h>
#include <QtCanvasPainter/qquickcpainteritem.h>

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QRhiCommandBuffer;
class QQuickCPainterRendererPrivate;

class Q_CANVASPAINTER_EXPORT QQuickCPainterRenderer : public QQuickRhiItemRenderer
{
public:
    QQuickCPainterRenderer();
    virtual ~QQuickCPainterRenderer() override;

    QColor fillColor() const;
    QCPainter *painter() const;
    float width() const;
    float height() const;

    bool hasSharedPainter() const;
    void setSharedPainter(bool enable);

protected:
    virtual void initializeResources(QCPainter *painter);
    virtual void prePaint(QCPainter *painter);
    virtual void paint(QCPainter *painter);
    virtual void synchronize(QQuickCPainterItem *item);
    virtual void initialize(QRhiCommandBuffer *cb) override;

    void render(QRhiCommandBuffer *cb) override;
    void synchronize(QQuickRhiItem *item) override;

    void grabCanvas(const QCOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

    void beginCanvasPainting(QCOffscreenCanvas &canvas);
    void endCanvasPainting();

private:
    friend class QQuickCPainterItem;
    Q_DECLARE_PRIVATE(QQuickCPainterRenderer)
    QQuickCPainterRendererPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QCQUICKITEMPAINTER_H
