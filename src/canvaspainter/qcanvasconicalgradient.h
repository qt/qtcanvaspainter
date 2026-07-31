// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVASCONICALGRADIENT_H
#define QCANVASCONICALGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>

#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCanvasConicalGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasConicalGradient();
    Q_CANVASPAINTER_EXPORT QCanvasConicalGradient(qreal centerX, qreal centerY, qreal startAngle);
    Q_CANVASPAINTER_EXPORT QCanvasConicalGradient(QPointF center, qreal startAngle);
    QCanvasConicalGradient(const QCanvasConicalGradient &) = default;
    QCanvasConicalGradient &operator=(const QCanvasConicalGradient &) = default;
    QCanvasConicalGradient(QCanvasConicalGradient &&) = default;
    QCanvasConicalGradient &operator=(QCanvasConicalGradient &&) = default;
    ~QCanvasConicalGradient() = default;

    Q_CANVASPAINTER_EXPORT QPointF centerPosition() const;
    Q_CANVASPAINTER_EXPORT void setCenterPosition(qreal x, qreal y);
    inline void setCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT qreal startAngle() const;
    Q_CANVASPAINTER_EXPORT void setStartAngle(qreal angle);

private:
    explicit QCanvasConicalGradient(const QCanvasBrush &brush) : QCanvasGradient(brush) {}
    friend class QCanvasGradientBrushPrivate;
    friend size_t qHash(const QCanvasConicalGradient &, size_t seed) = delete;
};

void QCanvasConicalGradient::setCenterPosition(QPointF center)
{
    setCenterPosition(center.x(), center.y());
}

template<> Q_CANVASPAINTER_EXPORT QCanvasConicalGradient QCanvasBrush::as<QCanvasConicalGradient>() const;

QT_END_NAMESPACE

#endif // QCANVASCONICALGRADIENT_H
