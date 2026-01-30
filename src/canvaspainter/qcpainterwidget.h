// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCPAINTERWIDGET_H
#define QCPAINTERWIDGET_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtWidgets/qrhiwidget.h>
#include <QtGui/qcolor.h>
#include <functional>
#include <QtCanvasPainter/qcoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCPainter;
class QCPainterWidgetPrivate;

class Q_CANVASPAINTER_EXPORT QCPainterWidget : public QRhiWidget
{
    Q_OBJECT
public:
    explicit QCPainterWidget(QWidget *parent = nullptr);
    ~QCPainterWidget() override;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    bool hasSharedPainter() const;
    void setSharedPainter(bool enable);

    void grabCanvas(const QCOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

protected:
    virtual void initializeResources(QCPainter *painter);
    virtual void prePaint(QCPainter *painter);
    virtual void paint(QCPainter *painter);
    virtual void graphicsResourcesInvalidated();

    void initialize(QRhiCommandBuffer *cb) override;
    void render(QRhiCommandBuffer *cb) override;
    void releaseResources() override;

    void beginCanvasPainting(QCOffscreenCanvas &canvas);
    void endCanvasPainting();

private:
    Q_DECLARE_PRIVATE(QCPainterWidget)
};

QT_END_NAMESPACE

#endif // QCPAINTERWIDGET_H
