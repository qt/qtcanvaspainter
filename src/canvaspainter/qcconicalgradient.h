// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCCONICALGRADIENT_H
#define QCCONICALGRADIENT_H

#include <QtCore/qglobal.h>
#include <QtCanvasPainter/qcgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCConicalGradientPrivate;

class Q_CANVASPAINTER_EXPORT QCConicalGradient : public QCGradient
{
public:
    QCConicalGradient();
    QCConicalGradient(float centerX, float centerY, float startAngle);
    QCConicalGradient(const QPointF &center, float startAngle);
    ~QCConicalGradient();

    QPointF centerPosition() const;
    void setCenterPosition(float x, float y);
    void setCenterPosition(const QPointF &center);
    float angle() const;
    void setAngle(float angle);

private:
    QCPaint createPaint(QCPainter *painter) const final;

};

Q_DECLARE_SHARED(QCConicalGradient)

QT_END_NAMESPACE

#endif // QCCONICALGRADIENT_H
