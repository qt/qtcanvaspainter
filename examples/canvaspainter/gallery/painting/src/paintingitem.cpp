// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "paintingitem.h"
#include "paintingitemrenderer.h"

PaintingItem::PaintingItem(QQuickItem *parent)
    : QQuickCPainterItem(parent)
{
}

QQuickCPainterRenderer* PaintingItem::createItemRenderer() const
{
    PaintingItemRenderer *itemRenderer = new PaintingItemRenderer();
    QObject::connect(itemRenderer, &PaintingItemRenderer::update, this, &PaintingItem::update);
    return itemRenderer;
}

void PaintingItem::mousePressEvent(QMouseEvent *event)
{
    // Start a new line
    m_pointsSynced = 0;
    m_points.clear();
    m_points.append(event->pos());
    update();
}

void PaintingItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // Use INT_MAX as a mark that drawing the line has ended
    m_points.append(QPoint(INT_MAX, 0));
    update();
}

void PaintingItem::mouseMoveEvent(QMouseEvent *event)
{
    // Ignore points too close to each other
    const QPoint posDiff = event->pos() - m_prevPoint;
    const int minMove = 2;
    if (posDiff.manhattanLength() < minMove)
        return;

    m_prevPoint = event->pos();
    m_points.append(m_prevPoint);
    update();
}
