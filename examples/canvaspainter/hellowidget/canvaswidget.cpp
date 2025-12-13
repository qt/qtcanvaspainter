// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "canvaswidget.h"

#include <QCPainter>
#include <QCRadialGradient>
#include <QCImagePattern>

using namespace Qt::StringLiterals;

CanvasWidget::CanvasWidget()
{
    setFillColor(Qt::white);
}

//![image]
void CanvasWidget::initializeResources(QCPainter *p)
{
    Q_ASSERT(m_image.isNull());
    const auto flags = QCPainter::ImageFlag::Repeat | QCPainter::ImageFlag::GenerateMipmaps;
    m_image = p->addImage(QImage(u":/qt-translucent.png"_s), flags);
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
    const float centerX = width() / 2;
    const float centerY = height() / 2;

    // Paint the background circle
    QCRadialGradient gradient1(centerX, centerY - size * 0.1f, size * 0.6f);
    gradient1.setStartColor(QColor(0x909090));
    gradient1.setEndColor(QColor(0x404040));
    p->beginPath();
    p->circle(QPointF(centerX, centerY), size * 0.46f);
    p->setFillStyle(gradient1);
    p->fill();
    p->setStrokeStyle(QColor(0x202020));
    p->setLineWidth(size * 0.02f);
    p->stroke();
//![paint]
    // Hello text
    p->setTextAlign(QCPainter::TextAlign::Center);
    p->setTextBaseline(QCPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(qRound(size * 0.08f));
    p->setFont(font1);
    p->setFillStyle(QColor(0xB0D040));
    p->fillText(u"HELLO"_s, centerX, centerY - size * 0.18f);

    // QCPainter text
    QFont font2;
    font2.setWeight(QFont::Weight::Thin);
    font2.setPixelSize(qRound(size * 0.11f));
    p->setFont(font2);
    p->fillText(u"Qt Canvas Painter"_s, centerX, centerY - size * 0.08f);

//![pattern]
    // Paint heart
    QCImagePattern pattern(m_image, centerX, centerY, size * 0.08f, size * 0.05f);
    p->setFillStyle(pattern);
//![pattern]
    p->setLineCap(QCPainter::LineCap::Round);
    p->setStrokeStyle(QColor(0xB0D040));
    p->beginPath();
    p->moveTo(centerX, centerY + size * 0.3f);
    p->bezierCurveTo(centerX - size * 0.25f, centerY + size * 0.1f,
                     centerX - size * 0.05f, centerY + size * 0.05f,
                     centerX, centerY + size * 0.15f);
    p->bezierCurveTo(centerX + size * 0.05f, centerY + size * 0.05f,
                     centerX + size * 0.25f, centerY + size * 0.1f,
                     centerX, centerY + size * 0.3f);
    p->stroke();
    p->fill();
}
