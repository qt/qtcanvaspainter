// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCRADIALGRADIENT_H
#define QCRADIALGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCRadialGradientPrivate;

class Q_CANVASPAINTER_EXPORT QCRadialGradient : public QCGradient
{
public:
    QCRadialGradient();
    QCRadialGradient(float centerX, float centerY, float outerRadius, float innerRadius = 0.0f);
    QCRadialGradient(QPointF center, float outerRadius, float innerRadius = 0.0f);
    ~QCRadialGradient();

    QPointF centerPosition() const;
    void setCenterPosition(float x, float y);
    void setCenterPosition(QPointF center);
    float outerRadius() const;
    void setOuterRadius(float radius);
    float innerRadius() const;
    void setInnerRadius(float radius);

private:
    QCPaint createPaint(QCPainter *painter) const final;
    void createRadialGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;

};

Q_DECLARE_SHARED(QCRadialGradient)

QT_END_NAMESPACE

#endif // QCRADIALGRADIENT_H
