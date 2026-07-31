// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERITEMRENDERER_H
#define QCANVASPAINTERITEMRENDERER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtQuick/qquickrhiitem.h>
#include <QtGui/qcolor.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCanvasPainter/qcanvaspainter.h>
#include <QtCanvasPainter/qcanvaspainteritem.h>

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QRhiCommandBuffer;
class QCanvasPainterItemRendererPrivate;

class Q_CANVASPAINTER_EXPORT QCanvasPainterItemRenderer : public QQuickRhiItemRenderer
{
public:
    QCanvasPainterItemRenderer();
    ~QCanvasPainterItemRenderer() override;

    QColor fillColor() const;
    QCanvasPainter *painter() const;
    qreal width() const;
    qreal height() const;

    bool hasSharedPainter() const;
    void setSharedPainter(bool enable);

protected:
    virtual void initializeResources(QCanvasPainter *painter);
    virtual void prePaint(QCanvasPainter *painter);
    virtual void paint(QCanvasPainter *painter);
    virtual void synchronizeData(QCanvasPainterItem *item);
    void initialize(QRhiCommandBuffer *cb) override;

    void render(QRhiCommandBuffer *cb) override;
    void synchronize(QQuickRhiItem *item) override;

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

    void beginCanvasPainting(QCanvasOffscreenCanvas &canvas);
    void endCanvasPainting();

private:
    friend class QCanvasPainterItem;
    Q_DECLARE_PRIVATE(QCanvasPainterItemRenderer)
    void grabCanvasImpl(const QCanvasOffscreenCanvas &canvas, const QObject *context,
                        QtPrivate::QSlotObjectBase *slotObj);
    QCanvasPainterItemRendererPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERITEMRENDERER_H
