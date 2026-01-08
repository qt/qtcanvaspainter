// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "canvasglwidget.h"
#include "helper.h"

#include <QPainter>
#include <QTimer>

CanvasGLWidget::CanvasGLWidget(Helper *helper, QWidget *parent)
    : QCPainterWidget(parent), helper(helper)
{
    setApi(QRhiWidget::Api::OpenGL);

    elapsed = 0;
    setFixedSize(200, 200);
    setAutoFillBackground(false);
}

void CanvasGLWidget::animate()
{
    elapsed = (elapsed + qobject_cast<QTimer*>(sender())->interval()) % 1000;
    update();
}

void CanvasGLWidget::initializeResources(QCPainter *painter)
{
    helper->canvasInit(painter);
}

void CanvasGLWidget::paint(QCPainter *painter)
{
    helper->paint(painter, QRect(0, 0, width(), height()), elapsed);
}
