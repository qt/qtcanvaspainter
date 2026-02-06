// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCBOXGRADIENT_H
#define QCBOXGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcgradient.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

class Q_CANVASPAINTER_EXPORT QCBoxGradient : public QCGradient
{
public:
    QCBoxGradient();
    QCBoxGradient(float x, float y, float width, float height, float feather, float radius = 0.0f);
    QCBoxGradient(const QRectF &rect, float feather, float radius = 0.0f);
    ~QCBoxGradient();

    QRectF rect() const;
    void setRect(float x, float y, float width, float height);
    inline void setRect(const QRectF &rect);
    float feather() const;
    void setFeather(float feather);
    float radius() const;
    void setRadius(float radius);
};

inline void QCBoxGradient::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

QT_END_NAMESPACE

#endif // QCBOXGRADIENT_H
