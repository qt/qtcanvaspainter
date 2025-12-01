// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PAINTHELPER_H
#define PAINTHELPER_H

#include <algorithm>
#include <QImage>
#include "qcpainter.h"
#include "qcradialgradient.h"
#include "qcimagepattern.h"

inline void paintHelloItem(QCPainter *p, float width, float height, QCImage *heartImage = nullptr, float patternSizeFactor = 0.02f)
{
    float size = std::min(width, height);
    QPointF center(width/2, height/2);
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
    font2.setPixelSize(size*0.12);
    p->setFont(font2);
    p->fillText("QCPainter", center.x(), center.y() - size*0.08);
    // Paint heart
    static QImage logoImage(":/quitlogo.png");
    // The QCImage must not be static, the underlying texture needs to be
    // recreated if the QRhi (and so the renderer) changes, when reparenting to
    // a new top-level for instance.
    QCImage logo = p->addImage(logoImage, QCPainter::ImageFlag::Repeat);
    float patternSize = size * patternSizeFactor;
    QCImagePattern pattern(heartImage ? *heartImage : logo, center.x(), center.y(), patternSize, patternSize);
    p->setFillStyle(pattern);
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

#endif // PAINTHELPER_H
