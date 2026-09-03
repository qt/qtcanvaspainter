// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASLINEARGRADIENT_H
#define QCANVASLINEARGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCanvasLinearGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasLinearGradient();
    Q_CANVASPAINTER_EXPORT QCanvasLinearGradient(qreal startX, qreal startY, qreal endX, qreal endY);
    Q_CANVASPAINTER_EXPORT QCanvasLinearGradient(QPointF start, QPointF end);
    QCanvasLinearGradient(const QCanvasLinearGradient &) = default;
    QCanvasLinearGradient &operator=(const QCanvasLinearGradient &) = default;
    QCanvasLinearGradient(QCanvasLinearGradient &&) = default;
    QCanvasLinearGradient &operator=(QCanvasLinearGradient &&) = default;
    ~QCanvasLinearGradient() = default;

    void swap(QCanvasLinearGradient &other) noexcept { QCanvasGradient::swap(other); }

    Q_CANVASPAINTER_EXPORT QPointF startPosition() const;
    Q_CANVASPAINTER_EXPORT void setStartPosition(qreal x, qreal y);
    inline void setStartPosition(QPointF start);
    Q_CANVASPAINTER_EXPORT QPointF endPosition() const;
    Q_CANVASPAINTER_EXPORT void setEndPosition(qreal x, qreal y);
    inline void setEndPosition(QPointF end);

private:
    explicit QCanvasLinearGradient(const QCanvasBrush &brush) : QCanvasGradient(brush) {}
    friend class QCanvasGradientBrushPrivate;
    friend size_t qHash(const QCanvasLinearGradient &, size_t seed) = delete;
};

Q_DECLARE_SHARED(QCanvasLinearGradient)

void QCanvasLinearGradient::setStartPosition(QPointF start)
{
    setStartPosition(start.x(), start.y());
}

void QCanvasLinearGradient::setEndPosition(QPointF end)
{
    setEndPosition(end.x(), end.y());
}

QT_END_NAMESPACE

#endif // QCANVASLINEARGRADIENT_H
