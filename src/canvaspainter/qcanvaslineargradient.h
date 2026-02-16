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

class Q_CANVASPAINTER_EXPORT QCanvasLinearGradient : public QCanvasGradient
{
public:
    QCanvasLinearGradient();
    QCanvasLinearGradient(float startX, float startY, float endX, float endY);
    QCanvasLinearGradient(QPointF start, QPointF end);
    ~QCanvasLinearGradient();

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    inline void setStartPosition(QPointF start);
    QPointF endPosition() const;
    void setEndPosition(float x, float y);
    inline void setEndPosition(QPointF end);
};

inline void QCanvasLinearGradient::setStartPosition(QPointF start)
{
    setStartPosition(float(start.x()), float(start.y()));
}

inline void QCanvasLinearGradient::setEndPosition(QPointF end)
{
    setEndPosition(float(end.x()), float(end.y()));
}

QT_END_NAMESPACE

#endif // QCANVASLINEARGRADIENT_H
