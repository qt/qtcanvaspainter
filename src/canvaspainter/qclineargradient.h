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
    QCLinearGradient(const QPointF &start, const QPointF &end);
    ~QCLinearGradient();

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    void setStartPosition(const QPointF &start);
    QPointF endPosition() const;
    void setEndPosition(float x, float y);
    void setEndPosition(const QPointF &end);

private:
   QCPaint createPaint(QCPainter *painter) const final;

};

Q_DECLARE_SHARED(QCLinearGradient)

QT_END_NAMESPACE

#endif // QCLINEARGRADIENT_H
