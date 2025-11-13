// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCRHIPAINTPAINTDRIVER_H
#define QCRHIPAINTPAINTDRIVER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qsize.h>
#include <functional>
#include <QtCanvasPainter/qccanvas.h>

QT_BEGIN_NAMESPACE

class QRhi;
class QRhiCommandBuffer;
class QRhiRenderTarget;
class QCRhiPaintDriverPrivate;

class Q_CANVASPAINTER_EXPORT QCRhiPaintDriver
{
public:
    enum class EndPaintFlag {
        DoNotRecordRenderPass = 0x01
    };
    Q_DECLARE_FLAGS(EndPaintFlags, EndPaintFlag)

    QCRhiPaintDriver();
    ~QCRhiPaintDriver();

    void resetForNewFrame();
    void beginPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QColor &fillColor = Qt::black, const QSize &logicalSize = QSize(), float dpr = 1.0f);
    void beginPaint(QCCanvas &canvas, QRhiCommandBuffer *cb);
    void endPaint(EndPaintFlags flags = {});
    void renderPaint();
    void grabCanvas(const QCCanvas &canvas, std::function<void(const QImage &)> callback);

private:
    Q_DISABLE_COPY(QCRhiPaintDriver)
    QCRhiPaintDriverPrivate *d = nullptr;
    friend class QCRhiPaintDriverPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCRhiPaintDriver::EndPaintFlags)

QT_END_NAMESPACE

#endif // QCRHIPAINTPAINTDRIVER_H
