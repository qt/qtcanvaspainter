// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASRADIALGRADIENT_H
#define QCANVASRADIALGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCanvasRadialGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient();
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(qreal centerX, qreal centerY,
                                                 qreal innerRadius, qreal outerRadius);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(QPointF centerPosition,
                                                 qreal innerRadius, qreal outerRadius);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(qreal innerCenterX, qreal innerCenterY, qreal innerRadius,
                                                 qreal outerCenterX, qreal outerCenterY, qreal outerRadius);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(QPointF innerCenterPosition, qreal innerRadius,
                                                 QPointF outerCenterPosition, qreal outerRadius);
    QCanvasRadialGradient(const QCanvasRadialGradient &) = default;
    QCanvasRadialGradient &operator=(const QCanvasRadialGradient &) = default;
    QCanvasRadialGradient(QCanvasRadialGradient &&) = default;
    QCanvasRadialGradient &operator=(QCanvasRadialGradient &&) = default;
    ~QCanvasRadialGradient() = default;

    Q_CANVASPAINTER_EXPORT QPointF centerPosition() const;
    Q_CANVASPAINTER_EXPORT void setCenterPosition(qreal x, qreal y);
    inline void setCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT QPointF innerCenterPosition() const;
    Q_CANVASPAINTER_EXPORT void setInnerCenterPosition(qreal x, qreal y);
    inline void setInnerCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT QPointF outerCenterPosition() const;
    Q_CANVASPAINTER_EXPORT void setOuterCenterPosition(qreal x, qreal y);
    inline void setOuterCenterPosition(QPointF center);

    Q_CANVASPAINTER_EXPORT qreal outerRadius() const;
    Q_CANVASPAINTER_EXPORT void setOuterRadius(qreal radius);
    Q_CANVASPAINTER_EXPORT qreal innerRadius() const;
    Q_CANVASPAINTER_EXPORT void setInnerRadius(qreal radius);

private:
    explicit QCanvasRadialGradient(const QCanvasBrush &brush) : QCanvasGradient(brush) {}
    friend class QCanvasGradientBrushPrivate;
    friend size_t qHash(const QCanvasRadialGradient &, size_t seed) = delete;
};

void QCanvasRadialGradient::setCenterPosition(QPointF center)
{
    setCenterPosition(center.x(), center.y());
}

void QCanvasRadialGradient::setInnerCenterPosition(QPointF center)
{
    setInnerCenterPosition(center.x(), center.y());
}

void QCanvasRadialGradient::setOuterCenterPosition(QPointF center)
{
    setOuterCenterPosition(center.x(), center.y());
}

QT_END_NAMESPACE

#endif // QCANVASRADIALGRADIENT_H
