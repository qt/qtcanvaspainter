// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EVENTITEMRENDERER_H
#define EVENTITEMRENDERER_H

#include "qquickcpainterrenderer.h"

class EventItemRenderer : public QQuickCPainterRenderer
{
public:
    EventItemRenderer();
    // Reimplement
    void synchronize(QQuickCPainterItem *item);
    void paint(QCPainter *p);

private:
    QList<QRectF> m_items;
    int m_activeItem = -1;
    int m_pressedItem = -1;
    int m_resizableItem = -1;
    int m_circleSize = 10;
    bool m_hoverEnabled = true;
    bool m_resizing = false;
    bool m_hovered = false;
};

#endif // EVENTITEMRENDERER_H
