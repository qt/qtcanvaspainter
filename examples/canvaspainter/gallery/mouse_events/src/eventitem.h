// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EVENTITEM_H
#define EVENTITEM_H

#include "qquickcpainteritem.h"

class EventItem : public QQuickCPainterItem
{
    Q_OBJECT
public:
    EventItem(QQuickItem *parent = nullptr);

    // Reimplement
    QQuickCPainterRenderer *createItemRenderer() const override;

public Q_SLOTS:
    void generateRandomItems();

protected:
    // Reimplement from QQuickItem
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;

private:
    friend class EventItemRenderer;
    int topItemAt(QPointF pos);
    int resizeItemAt(QPointF pos);

    QList<QRectF> m_items;
    int m_activeItem = -1;
    int m_pressedItem = -1;
    int m_resizableItem = -1;
    int m_circleSize = 10;
    bool m_resizing = false;
    bool m_hovered = false;
    QPointF m_pressPos;

};

#endif // EVENTITEM_H
