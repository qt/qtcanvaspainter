// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERWIDGET_H
#define QCANVASPAINTERWIDGET_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtWidgets/qrhiwidget.h>
#include <QtGui/qcolor.h>
#include <QtGui/qimage.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCanvasPainter;
class QCanvasPainterWidgetPrivate;

class Q_CANVASPAINTER_EXPORT QCanvasPainterWidget : public QRhiWidget
{
    Q_OBJECT
public:
    explicit QCanvasPainterWidget(QWidget *parent = nullptr);
    ~QCanvasPainterWidget() override;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    bool hasSharedPainter() const;
    void setSharedPainter(bool enable);

#ifdef Q_QDOC
    template <typename Functor>
    void grabCanvas(const QCanvasOffscreenCanvas &canvas, const QObject *context, Functor &&callback);
#else
    template <typename Functor>
    void grabCanvas(const QCanvasOffscreenCanvas &canvas,
                    const typename QtPrivate::ContextTypeForFunctor<Functor>::ContextType *context,
                    Functor &&callback)
    {
        using Prototype = void (*)(const QImage &);
        QtPrivate::AssertCompatibleFunctions<Prototype, Functor>();
        grabCanvasImpl(canvas, context,
                       QtPrivate::makeCallableObject<Prototype>(std::forward<Functor>(callback)));
    }
#endif

protected:
    virtual void initializeResources(QCanvasPainter *painter);
    virtual void prePaint(QCanvasPainter *painter);
    virtual void paint(QCanvasPainter *painter);
    virtual void graphicsResourcesInvalidated();

    void initialize(QRhiCommandBuffer *cb) override;
    void render(QRhiCommandBuffer *cb) override;
    void releaseResources() override;

    void beginCanvasPainting(QCanvasOffscreenCanvas &canvas);
    void endCanvasPainting();

private:
    Q_DECLARE_PRIVATE(QCanvasPainterWidget)
    void grabCanvasImpl(const QCanvasOffscreenCanvas &canvas, const QObject *context,
                        QtPrivate::QSlotObjectBase *slotObj);
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERWIDGET_H
