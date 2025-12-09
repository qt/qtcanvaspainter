// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "canvaswidget.h"
#include <QCPainter>
#include <QCRadialGradient>
#include <QCImagePattern>

CanvasWidget::CanvasWidget()
{
    setFillColor(Qt::white);
}

//![image]
void CanvasWidget::initializeResources(QCPainter *p)
{
    Q_ASSERT(m_image.isNull());
    m_image = p->addImage(QImage(":/qt-translucent.png"), QCPainter::ImageFlag::Repeat);
}

void CanvasWidget::graphicsResourcesInvalidated()
{
    m_image = {};
}
//![image]

//![paint]
void CanvasWidget::paint(QCPainter *p)
{
    const float size = std::min(width(), height());
    const QPointF center(width() / 2, height() / 2);

    // Paint the background circle
    QCRadialGradient gradient1(center.x(), center.y() - size*0.1, size*0.6);
    gradient1.setStartColor("#909090");
    gradient1.setEndColor("#404040");
    p->beginPath();
    p->circle(center, size*0.46);
    p->setFillStyle(gradient1);
    p->fill();
    p->setStrokeStyle("#202020");
    p->setLineWidth(size*0.02);
    p->stroke();
//![paint]
    // Hello text
    p->setTextAlign(QCPainter::TextAlign::Center);
    p->setTextBaseline(QCPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(size*0.08);
    p->setFont(font1);
    p->setFillStyle("#B0D040");
    p->fillText("HELLO", center.x(), center.y() - size*0.18);

    // QCPainter text
    QFont font2;
    font2.setWeight(QFont::Weight::Thin);
    font2.setPixelSize(size*0.11);
    p->setFont(font2);
    p->fillText("Qt Canvas Painter", center.x(), center.y() - size*0.08);

//![pattern]
    // Paint heart
    QCImagePattern pattern(m_image, center.x(), center.y(), size * 0.08, size * 0.05);
    p->setFillStyle(pattern);
//![pattern]
    p->setLineCap(QCPainter::LineCap::Round);
    p->setStrokeStyle("#B0D040");
    p->beginPath();
    p->moveTo(center.x(), center.y() + size*0.3);
    p->bezierCurveTo(center.x() - size*0.25, center.y() + size*0.1,
                     center.x() - size*0.05, center.y() + size*0.05,
                     center.x(), center.y() + size*0.15);
    p->bezierCurveTo(center.x() + size*0.05, center.y() + size*0.05,
                     center.x() + size*0.25, center.y() + size*0.1,
                     center.x(), center.y() + size*0.3);
    p->stroke();
    p->fill();
}
