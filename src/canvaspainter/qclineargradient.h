// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCLINEARGRADIENT_H
#define QCLINEARGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class Q_CANVASPAINTER_EXPORT QCLinearGradient : public QCGradient
{
public:
    QCLinearGradient();
    QCLinearGradient(float startX, float startY, float endX, float endY);
    QCLinearGradient(QPointF start, QPointF end);
    ~QCLinearGradient();

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    inline void setStartPosition(QPointF start);
    QPointF endPosition() const;
    void setEndPosition(float x, float y);
    inline void setEndPosition(QPointF end);
};

inline void QCLinearGradient::setStartPosition(QPointF start)
{
    setStartPosition(float(start.x()), float(start.y()));
}

inline void QCLinearGradient::setEndPosition(QPointF end)
{
    setEndPosition(float(end.x()), float(end.y()));
}

QT_END_NAMESPACE

#endif // QCLINEARGRADIENT_H
