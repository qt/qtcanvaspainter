// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PAINTINGITEM_H
#define PAINTINGITEM_H

#include "qquickcpainteritem.h"
#include <QVector>

class QQuickCPainterRenderer;

class PaintingItem: public QQuickCPainterItem
{
    Q_OBJECT
public:
    PaintingItem(QQuickItem *parent = nullptr);

    // Reimplement
    QQuickCPainterRenderer *createItemRenderer() const override;

protected:
    // Reimplement from QQuickItem
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    friend class PaintingItemRenderer;
    QVector<QPoint> m_points;
    QPoint m_prevPoint;
    int m_pointsSynced = 0;
};

#endif // PAINTINGITEM_H
