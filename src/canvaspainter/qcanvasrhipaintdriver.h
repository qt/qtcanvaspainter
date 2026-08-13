// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASRHIPAINTDRIVER_H
#define QCANVASRHIPAINTDRIVER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtGui/qimage.h>
#include <QtGui/qmatrix4x4.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QRhi;
class QRhiCommandBuffer;
class QRhiRenderTarget;
class QCanvasRhiPaintDriverPrivate;

class QCanvasRhiPaintDriver
{
public:
    enum class BeginPaintFlag {
        DepthTest = 0x01
    };
    Q_DECLARE_FLAGS(BeginPaintFlags, BeginPaintFlag)
    enum class EndPaintFlag {
        DoNotRecordRenderPass = 0x01
    };
    Q_DECLARE_FLAGS(EndPaintFlags, EndPaintFlag)

    Q_CANVASPAINTER_EXPORT QCanvasRhiPaintDriver();
    Q_CANVASPAINTER_EXPORT ~QCanvasRhiPaintDriver();

    Q_CANVASPAINTER_EXPORT void resetForNewFrame();
    Q_CANVASPAINTER_EXPORT void beginPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt,
                                           const QColor &fillColor = Qt::black, QSize logicalSize = QSize(), float dpr = 1.0f,
                                           BeginPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void beginPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QMatrix4x4 &matrix, BeginPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void beginPaint(QCanvasOffscreenCanvas &canvas, QRhiCommandBuffer *cb, BeginPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void endPaint(EndPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void renderPaint();

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

private:
    Q_DISABLE_COPY_MOVE(QCanvasRhiPaintDriver)
    Q_CANVASPAINTER_EXPORT void grabCanvasImpl(const QCanvasOffscreenCanvas &canvas,
                                               const QObject *context,
                                               QtPrivate::QSlotObjectBase *slotObj);
    QCanvasRhiPaintDriverPrivate *d = nullptr;
    friend class QCanvasRhiPaintDriverPrivate;
    friend class QCanvasPainterWidget;
    friend class QCanvasPainterItemRenderer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasRhiPaintDriver::BeginPaintFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasRhiPaintDriver::EndPaintFlags)

QT_END_NAMESPACE

#endif // QCANVASRHIPAINTDRIVER_H
