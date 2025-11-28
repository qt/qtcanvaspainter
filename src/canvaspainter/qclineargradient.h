// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCLINEARGRADIENT_H
#define QCLINEARGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCLinearGradientPrivate;

class Q_CANVASPAINTER_EXPORT QCLinearGradient : public QCGradient
{
public:
    QCLinearGradient();
    QCLinearGradient(float startX, float startY, float endX, float endY);
    QCLinearGradient(QPointF start, QPointF end);
    ~QCLinearGradient();

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    void setStartPosition(QPointF start);
    QPointF endPosition() const;
    void setEndPosition(float x, float y);
    void setEndPosition(QPointF end);

private:
    QCPaint createPaint(QCPainter *painter) const final;
    void createLinearGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;

};

Q_DECLARE_SHARED(QCLinearGradient)

QT_END_NAMESPACE

#endif // QCLINEARGRADIENT_H
