// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CANVASGLWIDGET_H
#define CANVASGLWIDGET_H

#include <QCPainterWidget>

class Helper;

class CanvasGLWidget : public QCPainterWidget
{
    Q_OBJECT

public:
    CanvasGLWidget(Helper *helper, QWidget *parent);

public slots:
    void animate();

protected:
    void initializeResources(QCPainter *painter) override;
    void paint(QCPainter *painter) override;

private:
    Helper *helper;
    int elapsed;
};

#endif
