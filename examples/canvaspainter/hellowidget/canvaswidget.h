// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QCPainterWidget>
#include <QCImage>

//![0]
class CanvasWidget : public QCPainterWidget
{
public:
    CanvasWidget();
    void initializeResources(QCPainter *p) override;
    void paint(QCPainter *p) override;
    void graphicsResourcesInvalidated() override;

private:
    QCImage m_image;
};
//![0]

#endif
