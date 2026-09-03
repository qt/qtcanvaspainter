// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBOXGRADIENT_H
#define QCANVASBOXGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

class QCanvasBoxGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasBoxGradient();
    Q_CANVASPAINTER_EXPORT QCanvasBoxGradient(qreal x, qreal y, qreal width, qreal height);
    QCanvasBoxGradient(const QRectF &rect)
        : QCanvasBoxGradient(rect.x(), rect.y(), rect.width(), rect.height())
    {}
    QCanvasBoxGradient(const QCanvasBoxGradient &) = default;
    QCanvasBoxGradient &operator=(const QCanvasBoxGradient &) = default;
    QCanvasBoxGradient(QCanvasBoxGradient &&) = default;
    QCanvasBoxGradient &operator=(QCanvasBoxGradient &&) = default;
    ~QCanvasBoxGradient() = default;

    void swap(QCanvasBoxGradient &other) noexcept { QCanvasGradient::swap(other); }

    Q_CANVASPAINTER_EXPORT QRectF rect() const;
    Q_CANVASPAINTER_EXPORT void setRect(qreal x, qreal y, qreal width, qreal height);
    inline void setRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT qreal feather() const;
    Q_CANVASPAINTER_EXPORT void setFeather(qreal feather);
    Q_CANVASPAINTER_EXPORT qreal radius() const;
    Q_CANVASPAINTER_EXPORT void setRadius(qreal radius);

private:
    explicit QCanvasBoxGradient(const QCanvasBrush &brush) : QCanvasGradient(brush) {}
    friend class QCanvasGradientBrushPrivate;
    friend size_t qHash(const QCanvasBoxGradient &, size_t seed) = delete;
};

Q_DECLARE_SHARED(QCanvasBoxGradient)

void QCanvasBoxGradient::setRect(const QRectF &rect)
{
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QT_END_NAMESPACE

#endif // QCANVASBOXGRADIENT_H
