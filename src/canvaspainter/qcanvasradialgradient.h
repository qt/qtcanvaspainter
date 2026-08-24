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
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(float centerX, float centerY,
                                                 float innerRadius, float outerRadius);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(QPointF centerPosition,
                                                 float innerRadius, float outerRadius);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(float innerCenterX, float innerCenterY, float innerRadius,
                                                 float outerCenterX, float outerCenterY, float outerRadius);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(QPointF innerCenterPosition, float innerRadius,
                                                 QPointF outerCenterPosition, float outerRadius);
    QCanvasRadialGradient(const QCanvasRadialGradient &) = default;
    QCanvasRadialGradient &operator=(const QCanvasRadialGradient &) = default;
    QCanvasRadialGradient(QCanvasRadialGradient &&) = default;
    QCanvasRadialGradient &operator=(QCanvasRadialGradient &&) = default;
    ~QCanvasRadialGradient() = default;

    Q_CANVASPAINTER_EXPORT QPointF centerPosition() const;
    Q_CANVASPAINTER_EXPORT void setCenterPosition(float x, float y);
    inline void setCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT QPointF innerCenterPosition() const;
    Q_CANVASPAINTER_EXPORT void setInnerCenterPosition(float x, float y);
    inline void setInnerCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT QPointF outerCenterPosition() const;
    Q_CANVASPAINTER_EXPORT void setOuterCenterPosition(float x, float y);
    inline void setOuterCenterPosition(QPointF center);

    Q_CANVASPAINTER_EXPORT float outerRadius() const;
    Q_CANVASPAINTER_EXPORT void setOuterRadius(float radius);
    Q_CANVASPAINTER_EXPORT float innerRadius() const;
    Q_CANVASPAINTER_EXPORT void setInnerRadius(float radius);

private:
    friend size_t qHash(const QCanvasRadialGradient &, size_t seed) = delete;
};

void QCanvasRadialGradient::setCenterPosition(QPointF center)
{
    setCenterPosition(float(center.x()), float(center.y()));
}

void QCanvasRadialGradient::setInnerCenterPosition(QPointF center)
{
    setInnerCenterPosition(float(center.x()), float(center.y()));
}

void QCanvasRadialGradient::setOuterCenterPosition(QPointF center)
{
    setOuterCenterPosition(float(center.x()), float(center.y()));
}

template<> Q_CANVASPAINTER_EXPORT QCanvasRadialGradient QCanvasBrush::as<QCanvasRadialGradient>() const;

QT_END_NAMESPACE

#endif // QCANVASRADIALGRADIENT_H
