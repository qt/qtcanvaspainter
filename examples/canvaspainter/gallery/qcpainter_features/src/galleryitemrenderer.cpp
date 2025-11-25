// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "galleryitemrenderer.h"
#include "galleryitem.h"
#include "qcradialgradient.h"
#include "qcboxgradient.h"
#include "qcconicalgradient.h"
#include "qclineargradient.h"
#include "qcimagepattern.h"
#include "qcboxshadow.h"
#include "qcgridpattern.h"
#include "qcpainterpath.h"
#include "qctext.h"
#include <math.h>
#include <QFontDatabase>

GalleryItemRenderer::GalleryItemRenderer()
{
    QFontDatabase::addApplicationFont(":/qcpainter_features/fonts/Pacifico.ttf");

    m_radGrad.setStops({QCGradientStop(0.0f, QColor(255, 0, 0, 255)),
                        QCGradientStop(0.1f, QColor(255, 255, 0, 255)),
                        QCGradientStop(0.5f, QColor(0, 255, 0, 255)),
                        QCGradientStop(1.0f, QColor(0, 0, 0, 0)) });
}

GalleryItemRenderer::~GalleryItemRenderer()
{
}

void GalleryItemRenderer::initializeResources(QCPainter *painter)
{
    QCPainter::ImageFlags flags = QCPainter::ImageFlag::Repeat | QCPainter::ImageFlag::GenerateMipmaps;
    m_patternImage = painter->addImage(QImage(":/qcpainter_features/images/pattern1.png"), flags);
    m_patternImage2 = painter->addImage(QImage(":/qcpainter_features/images/pattern2.png"), flags);
    m_patternImage3 = painter->addImage(QImage(":/qcpainter_features/images/pattern3.png"), flags);
    m_testImage = painter->addImage(QImage(":/qcpainter_features/images/qt_development_white.png"));
    image3Gray = painter->addImage(QImage(":/qcpainter_features/images/face-smile-bw.png"));
    image3Plain = painter->addImage(QImage(":/qcpainter_features/images/pattern2.png"));
    image3Nearest = painter->addImage(QImage(":/qcpainter_features/images/pattern2.png"),
                                        QCPainter::ImageFlag::Nearest);
    image3Mips = painter->addImage(QImage(":/qcpainter_features/images/pattern2.png"),
                                     QCPainter::ImageFlag::GenerateMipmaps);
    image3NearestMips = painter->addImage(QImage(":/qcpainter_features/images/pattern2.png"),
                                            QCPainter::ImageFlag::Nearest | QCPainter::ImageFlag::GenerateMipmaps);
    m_customBrush.setFragmentShader(":/qcgalleryexample/qcpainter_features/brush1.frag.qsb");
    m_customBrush2.setFragmentShader(":/qcgalleryexample/qcpainter_features/brush2.frag.qsb");
    m_customBrush3.setFragmentShader(":/qcgalleryexample/qcpainter_features/brush3.frag.qsb");
    m_customBrush3.setVertexShader(":/qcgalleryexample/qcpainter_features/brush3.vert.qsb");
    m_customBrush4.setFragmentShader(":/qcgalleryexample/qcpainter_features/brush4.frag.qsb");
    // Enable iTime animations
    m_customBrush.setTimeRunning(true);
    m_customBrush2.setTimeRunning(true);
    m_customBrush3.setTimeRunning(true);
    m_customBrush4.setTimeRunning(true);
}

void GalleryItemRenderer::synchronize(QQuickCPainterItem *item)
{
    // Setting values here synchronized
    GalleryItem *realItem = static_cast<GalleryItem*>(item);
    if (realItem) {
        m_animationTime = realItem->animationTime();
        m_animationSine = realItem->animationSine();
        m_viewIndex = realItem->galleryView();
    }
}

void GalleryItemRenderer::paint(QCPainter *painter)
{
    Q_UNUSED(painter)

    // Draw similarly colored rectangles
    switch (m_viewIndex) {
    case 0:
        drawRectsWithSameColor();
        //drawRectsWithMix();
        drawRectsWithLinearGradient();
        drawRectsWithRadialGradient();
        drawRectsWithBoxGradient();
        drawRectsWithConicalGradients();
        drawRectsWithImagePattern();
        drawRectsWithBrushStroke();
        break;
    case 1:
        drawPaths();
        break;
    case 2:
        drawPainterPaths();
        break;
    case 3:
        drawTransforms();
        break;
    case 4:
        drawAntialiasing();
        break;
    case 5:
        drawCompositeModes();
        break;
    case 6:
        drawGridPatterns();
        break;
    case 7:
        drawRectangularShadows();
        break;
    case 8:
        drawCustomBrushes();
        break;
    case 9:
        drawTextsFonts();
        break;
    case 10:
        drawTextsBrushes();
        break;
    case 11:
        drawTextsAlignments();
        break;
    case 12:
        drawTextsWrapping();
        break;
    case 13:
        drawImages();
        break;
    case 14:
        //drawFrameBuffers();
        break;
    default:
        break;
    }
}

void GalleryItemRenderer::drawRectsWithSameColor() {

    int rects = 7;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w/2 + margin;
    float posY = w/2;

    painter()->setStrokeStyle(QColor(0,0,0));
    painter()->setFillStyle(QColor(204,102,51,102));
    drawRect(posX,posY,w,w);
    posX += w + margin;

    painter()->setStrokeStyle("#000000");
    painter()->setFillStyle(QColor::fromRgba(0x66CC6633));
    drawRect(posX,posY,w,w);
    posX += w + margin;

    QColor c1; // Note: Using default
    QColor c2;
    c2.setRed(204);
    c2.setGreen(102);
    c2.setBlue(51);
    c2.setAlpha(102);
    painter()->setStrokeStyle(c1);
    painter()->setFillStyle(c2);
    drawRect(posX,posY,w,w);
    posX += w + margin;

    QColor cf1; // Note: Using default
    QColor cf2;
    cf2.setRedF(0.8f);
    cf2.setGreenF(0.4f);
    cf2.setBlueF(0.2f);
    cf2.setAlphaF(0.4f);
    painter()->setStrokeStyle(cf1);
    painter()->setFillStyle(cf2);
    drawRect(posX,posY,w,w);
    posX += w + margin;

    QColor qc1(0,0,0,255);
    QColor qc2(204,102,51,102);
    painter()->setStrokeStyle(qc1);
    painter()->setFillStyle(qc2);
    drawRect(posX,posY,w,w);
    posX += w + margin;

    painter()->setStrokeStyle(QColor::fromHsl(0, 0, 0, 255));
    // HSL: 20° 60% 50%
    painter()->setFillStyle(QColor::fromHsl(20, 255*0.6, 255*0.5, 102));
    drawRect(posX,posY,w,w);
    posX += w + margin;

    QColor cc1(c1);
    QColor cc2(c2);
    painter()->setStrokeStyle(cc1);
    painter()->setFillStyle(cc2);
    drawRect(posX,posY,w,w);
}

// Disabled as QColor doesn't have mix / lerp methods.
/*
void GalleryItemRenderer::drawRectsWithMix() {
    int rects = 3;
    float margin = width()*0.02f;
    float w = width() / (rects+2) - margin;
    float posX = w + margin;
    float posY = w+margin;

    QCColor c1 = QCColor::fromMix(QCColor(255,0,0,255), QCColor(0,0,0,255), m_animationSine);
    QCColor c2 = QCColor::fromMix(QCColor(255,0,0,100), QCColor(0,0,0,0), m_animationSine);
    painter()->setStrokeStyle(c1);
    painter()->setFillStyle(c2);
    drawRect(posX,posY,w,w);
    posX += w + margin;

    QCColor c12 = QCColor::fromMix(QCColor(0,255,0,255), QCColor(0,0,0,255), m_animationSine);
    QCColor c22 = QCColor::fromMix(QCColor(0,255,0,100), QCColor(0,0,0,0), m_animationSine);
    painter()->setStrokeStyle(c12);
    painter()->setFillStyle(c22);
    drawRect(posX,posY,w,w);
    posX += w + margin;

    QCColor c23 = QCColor::fromMix(QCColor(0,0,255,255), QCColor(0,0,255,0), m_animationSine);
    painter()->setStrokeStyle(QCColor(255,255,255));
    painter()->setFillStyle(c23);
    drawRect(posX,posY,w,w);
    posX += w + margin;
}
*/

void GalleryItemRenderer::drawRectsWithLinearGradient() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = 1*(w+margin);

    QRectF rect1(posX,posY,w,w);
    painter()->setFillStyle(QCLinearGradient(0, rect1.y(), 0, rect1.y()+rect1.height()));
    painter()->fillRect(rect1);
    posX += w + margin;

    QCLinearGradient g1(0, 0, 0, 0);
    g1.setStartColor(QColor(0,255,0,255));
    g1.setEndColor(QColor(255,0,0,255));
    g1.setStartPosition(posX + (w*0.4f * m_animationSine), 0);
    g1.setEndPosition(posX + w - (w*0.4f * m_animationSine), 0);
    painter()->setFillStyle(g1);
    painter()->fillRect(posX,posY,w,w);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCLinearGradient g2(rect2.x(), rect2.y(), rect2.x()+rect2.width(), rect2.y()+rect2.height());
    g2.setStartColor(QColor(m_animationSine*255, 255, 255));
    g2.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g2);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCLinearGradient g3(rect3.x(), rect3.y(), rect3.x(), rect3.y()+rect3.height());
    g3.setColorAt(0.0f, QColorConstants::Black);
    g3.setColorAt(0.2f, QColorConstants::Red);
    g3.setColorAt(0.4f, QColorConstants::Green);
    g3.setColorAt(0.6f, QColorConstants::Blue);
    g3.setColorAt(0.8f, QColorConstants::White);
    g3.setColorAt(1.0f, QColorConstants::Transparent);
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
}

void GalleryItemRenderer::drawRectsWithRadialGradient() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = 2*(w+margin);

    QRectF rect1(posX,posY,w,w);
    painter()->setFillStyle(QCRadialGradient(rect1.x(), rect1.y(), rect1.width()));
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCRadialGradient g1;
    g1.setStartColor(QColor(0,255,0,255));
    g1.setEndColor(QColor(255,0,0,255));
    g1.setCenterPosition(rect2.x() + rect2.width()/2, rect2.y() + rect2.height()/2);
    g1.setOuterRadius(0.5*w);
    g1.setInnerRadius(0.4*w*m_animationSine);
    painter()->setFillStyle(g1);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCRadialGradient g3(rect3.x() + rect3.width()/2, rect3.y() + rect3.height()/2, w/2, w/4);
    g3.setStartColor(QColor(m_animationSine*255, 255, 255));
    g3.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
    posX += w + margin;

    QRectF rect4(posX,posY,w,w);
    m_radGrad.setCenterPosition(rect4.x() + rect4.width()/2, rect4.y() + rect4.height()/2);
    m_radGrad.setOuterRadius(w/2);
    m_radGrad.setInnerRadius(w/16);
    auto stops = m_radGrad.stops();
    stops[2].first = 0.2 + 0.6 * m_animationSine;
    stops[0].second = QColor(255, m_animationSine*255, 0, 255-m_animationSine*255);
    m_radGrad.setStops(stops);
    painter()->setFillStyle(m_radGrad);
    painter()->fillRect(rect4);
}

void GalleryItemRenderer::drawRectsWithBoxGradient() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = 3*(w+margin);

    QRectF rect1(posX,posY,w,w);
    painter()->setFillStyle(QCBoxGradient(rect1, w/2));
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCBoxGradient g1;
    g1.setStartColor(QColor(0,255,0,255));
    g1.setEndColor(QColor(255,0,0,255));
    g1.setRect(rect2);
    g1.setRadius(w/4);
    g1.setFeather(w/2 - m_animationSine*w/3);
    painter()->setFillStyle(g1);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCBoxGradient g3(rect3, w/4, w/4);
    g3.setStartColor(QColor(m_animationSine*255, 255, 255));
    g3.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
    posX += w + margin;

    QRectF rect4(posX,posY,w,w);
    QCBoxGradient g4(rect4, w/4, w/3);
    g4.setColorAt(0.0f, QColor(255, 255, 255, 255));
    g4.setColorAt(0.2f, QColor(0, 0, 0, 255));
    g4.setColorAt(0.4f, QColor(255, m_animationSine*255, 0, 255));
    g4.setColorAt(0.6f, QColor(0, 0, 0, 255));
    g4.setColorAt(0.8f, QColor(255, 255, 255, 255));
    g4.setColorAt(1.0f, QColor(0, 0, 0, 0));
    painter()->setFillStyle(g4);
    painter()->fillRect(rect4);
}

void GalleryItemRenderer::drawRectsWithConicalGradients() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = 4*(w+margin);

    QRectF rect1(posX,posY,w,w);
    QCConicalGradient g1;
    g1.setCenterPosition(rect1.x(), rect1.y());
    painter()->setFillStyle(g1);
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCConicalGradient g2;
    g2.setCenterPosition(rect2.x() + rect2.width()/2, rect2.y() + rect2.height()/2);
    g2.setStartColor(QColor(0,255,0,255));
    g2.setEndColor(QColor(255,0,0,255));
    painter()->setFillStyle(g2);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCConicalGradient g3;
    g3.setCenterPosition(rect3.x() + rect3.width()/2, rect3.y() + rect3.height()/2);
    g3.setStartColor(QColor(m_animationSine*255, 255, 255));
    g3.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
    posX += w + margin;

    QRectF rect4(posX,posY,w,w);
    QCConicalGradient g4;
    g4.setCenterPosition(rect4.x() + m_animationSine * rect4.width(), rect4.y() + rect4.height()/2);
    g4.setAngle(m_animationTime);
    g4.setStartColor(QColor(255, 255, 0, 255));
    g4.setColorAt(0.25, QColor(0, 255, 0, 255));
    g4.setColorAt(0.5, QColor(0, 0, 255, 255));
    g4.setColorAt(0.75, QColor(255, 0, 255, 255));
    g4.setEndColor(QColor(255, 255, 0, 255));
    painter()->setFillStyle(g4);
    painter()->fillRect(rect4);
}

void GalleryItemRenderer::drawRectsWithImagePattern() {
    int rects = 3;
    float margin = width()*0.02f;
    float w = width() / (rects+2) - margin;
    float posX = w + margin;
    float posY = 5*(w+margin);

    QRectF rect1(posX,posY,w,w);
    QCImagePattern g4(m_patternImage2, rect1);
    QColor aColor = QColor::fromRgbF(1.0f,
                                     0.5f + 0.5f * sin(m_animationTime),
                                     0.5f + 0.5f * sin(m_animationTime * 2 + M_PI),
                                     1.0f);
    g4.setTintColor(aColor);
    painter()->setFillStyle(g4);
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCImagePattern g1;
    g1.setImage(m_patternImage);
    g1.setStartPosition(rect2.x()+w/2, rect2.y()+w/2);
    g1.setImageSize(w/5, w/3);
    g1.setRotation(m_animationTime);
    QColor opacityTint(255, 255, 255, 127 + m_animationSine * 128);
    g1.setTintColor(opacityTint);
    painter()->setFillStyle(g1);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCImagePattern g3(m_patternImage);
    g3.setStartPosition(rect3.x(), rect3.y());
    g3.setImageSize(w*0.1 + m_animationSine * w*0.2f, w*0.15f + m_animationSine * w*0.3f);
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
}

void GalleryItemRenderer::drawRectsWithBrushStroke() {
    int rects = 3;
    float margin = width()*0.02f;
    float border = margin + margin * m_animationSine;
    float w = width() / (rects+2) - margin;
    float w2 = w - border;
    float posX = w + margin + border/2;
    float posY = 6*(w+margin) + border/2;

    QRectF rect1(posX,posY,w2,w2);
    painter()->setLineWidth(border);
    painter()->setFillStyle(QCImagePattern(m_patternImage3));
    QCLinearGradient g1(posX, posY, posX+w2, posY+w2);
    g1.setStartColor("#ffffff");
    g1.setEndColor("#000000");
    painter()->setStrokeStyle(g1);
    painter()->beginPath();
    painter()->roundRect(rect1, border);
    painter()->fill();
    painter()->stroke();
    posX += w + margin;

    QRectF rect2(posX,posY,w2,w2);
    g1.setStartPosition(posX, posY);
    g1.setEndPosition(posX+w2, posY+w2);
    painter()->setFillStyle(g1);
    painter()->setStrokeStyle(QCImagePattern(m_patternImage3));
    painter()->beginPath();
    painter()->roundRect(rect2, border);
    painter()->fill();
    painter()->stroke();
    posX += w + margin;

    QRectF rect3(posX,posY,w2,w2);
    QCRadialGradient g2(posX+w2/4, posY+w2/4, w2);
    g2.setStartColor("#900000ff");
    g2.setEndColor("#90ff0000");
    painter()->setStrokeStyle(g2);
    QCImagePattern p1 = QCImagePattern(m_patternImage3);
    p1.setImageSize(16, 16);
    p1.setStartPosition(m_animationTime*40, 0);
    painter()->setFillStyle(p1);
    painter()->beginPath();
    painter()->roundRect(rect3, border);
    painter()->fill();
    painter()->stroke();
}

void GalleryItemRenderer::drawRect(float x, float y, float w, float h) {
    painter()->beginPath();
    painter()->rect(x, y, w, h);
    painter()->setLineWidth(w*0.05f);
    painter()->fill();
    painter()->stroke();
}

void GalleryItemRenderer::drawPaths() {
    int rects = 3;
    float margin = width()*0.1f;
    float w = width() / (rects) - margin;
    float posX = margin/2;
    float posY = margin;
    float lineWidth = width()*0.02f;

    painter()->setFillStyle(0xFFE0E0E0);
    painter()->setStrokeStyle(0xFFE00000);
    painter()->setLineWidth(lineWidth);
    painter()->setLineCap(QCPainter::LineCap::Round);
    painter()->setLineJoin(QCPainter::LineJoin::Round);

    painter()->beginPath();
    painter()->rect(posX, posY, w, w);
    posX += w + margin;
    painter()->roundRect(posX, posY, w, w, w/2*m_animationSine);
    posX += w + margin;
    painter()->roundRect(posX, posY, w, w, w/2*m_animationSine, 0, w/2*(cos(m_animationTime)*0.5+0.5), w/4*(sin(m_animationTime*10)*0.5+0.5));

    posY += w+margin;
    posX = margin/2;
    painter()->ellipse(posX+w/2, posY+w/2, w/3+w/4*m_animationSine, w/3+w/4*(1-m_animationSine));
    posX += w + margin;
    painter()->moveTo(posX+w/2, posY+w/2);
    painter()->arc(posX+w/2, posY+w/2, w/2, 0, 0.5f + m_animationSine*5);
    painter()->closePath();
    posX += w + margin;
    QRectF r(posX, posY, w, w);
    painter()->ellipse(r);
    float a = w*0.1f*m_animationSine;
    float a2 = w*0.1f*(1-m_animationSine);
    r.adjust(w*0.1f+a, w*0.1f+a2, -w*0.1f-a, -w*0.1f-a2);
    painter()->beginHoleSubPath();
    painter()->ellipse(r);
    r.adjust(w*0.2f, w*0.2f, -w*0.2f, -w*0.2f);
    painter()->beginSolidSubPath();
    painter()->rect(r);

    painter()->stroke();
    painter()->fill();

    posY += w+margin;
    posX = margin/2;
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->bezierCurveTo(posX+w*0.2f*m_animationSine, posY+w*0.8f*m_animationSine, posX+w*0.8f*m_animationSine, posY+w*0.2f*m_animationSine, posX+w, posY+w);
    posX += w + margin;
    painter()->moveTo(posX, posY);
    painter()->quadraticCurveTo(posX+w*0.8f*m_animationSine, posY+w*0.2f*m_animationSine, posX+w, posY+w);
    posX += w + margin;
    painter()->moveTo(posX, posY);
    painter()->arcTo(posX, posY+w, posX+w, posY+w, w*m_animationSine);

    painter()->stroke();

    posY += w+margin;
    posX = margin/2;
    painter()->setLineCap(QCPainter::LineCap::Round);
    painter()->setLineJoin(QCPainter::LineJoin::Round);
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->lineTo(posX, posY+w);
    painter()->lineTo(posX+w*m_animationSine, posY);
    painter()->stroke();
    posX += w + margin;
    painter()->setLineCap(QCPainter::LineCap::Butt);
    painter()->setLineJoin(QCPainter::LineJoin::Miter);
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->lineTo(posX, posY+w);
    painter()->lineTo(posX+w*m_animationSine, posY);
    painter()->stroke();
    posX += w + margin;
    painter()->setLineCap(QCPainter::LineCap::Square);
    painter()->setLineJoin(QCPainter::LineJoin::Bevel);
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->lineTo(posX, posY+w);
    painter()->lineTo(posX+w*m_animationSine, posY);
    painter()->stroke();
}

void GalleryItemRenderer::drawPainterPaths()
{
    float w = width() * 0.8;
    float h = height() * 0.2;
    float posX = width() * 0.5 - w * 0.5;
    float posY = h * 0.5;
    float margin = height()*0.02f;
    const int points = 100;

    enum PathGroups {
        DynamicPath = 0,
        StaticPath
    };

    // Path that doesn't change
    static QCPainterPath path1;
    if (path1.isEmpty()) {
        // Reserve, so allocations are not needed while adding commands.
        path1.reserveCommands(points);
        // All commands are moveTo & lineTo which take 2 data
        // points (x & y) per command.
        path1.reserveCommandsData(points * 2);
        for (int i = 0; i < points; i++) {
            int px = posX + float(i) / (points - 1) * w;
            int py = posY + std::sin(1234.0 * (float(i*i))) * (h * 0.4);
            if (i == 0)
                path1.moveTo(px, py);
            else
                path1.lineTo(px, py);
        }
    }
    painter()->setStrokeStyle("#608020");
    painter()->setLineWidth(4);
    // Use same group for all non-changing paths.
    painter()->stroke(path1, StaticPath);

    posY += h + margin;

    // Path that increases, so old commands are remembered
    static QCPainterPath path2;
    static int p2Index = 0;
    if (p2Index >= points) {
        // When we reach the end, start from the beginning
        path2.clear();
        p2Index = 0;
    }
    int i = p2Index;
    int px = posX + float(i) / (points - 1) * w;
    int py = posY + std::sin(1234.0 * (float(i*i))) * (h * 0.4);
    if (i == 0)
        path2.moveTo(px, py);
    else
        path2.lineTo(px, py);
    painter()->setStrokeStyle("#406080");
    painter()->setLineWidth(4);
    painter()->stroke(path2, DynamicPath);
    p2Index++;

    posY += h + margin;

    // Path that doesn't change but is moved/scaled/rotated
    static QCPainterPath path3;
    QPointF center(posX + 0.5 * w, posY);
    float hSize = h * 0.8;
    if (path3.isEmpty()) {
        path3.moveTo(center.x(), center.y() + hSize*0.5);
        path3.bezierCurveTo(center.x() - hSize, center.y() - hSize*0.3,
                            center.x() - hSize*0.1, center.y() - hSize*0.5,
                            center.x(), center.y() - hSize*0.2);
        path3.bezierCurveTo(center.x() + hSize*0.1, center.y() - hSize*0.5,
                            center.x() + hSize, center.y() - hSize*0.3,
                            center.x(), center.y() + hSize*0.5);
        // Add holes into the path
        path3.setPathWinding(QCPainter::PathWinding::ClockWise);
        path3.circle(center.x() - hSize*0.2, center.y() - hSize*0.1, hSize * 0.1);
        path3.circle(center.x() + hSize*0.2, center.y() - hSize*0.1, hSize * 0.1);
    }
    QTransform transform;
    transform.translate(-w * 0.2 + w * 0.2 * sin(m_animationTime), 0);
    transform.translate(center.x(), center.y());
    transform.rotateRadians(m_animationTime);
    const float s = 0.5f + m_animationSine;
    transform.scale(s, s);
    transform.translate(-center.x(), -center.y());
    painter()->setStrokeStyle("#ff0000");
    QCRadialGradient rg(center, hSize);
    rg.setStops({{0.0f, QColor(255, 50 + 50 * sin(10.0 * m_animationTime), 0)},
                 {1.0f, QColor(100, 0, 0)}});
    painter()->setFillStyle(rg);
    painter()->setLineWidth(2);
    // Using path transformation, so group can be StaticPath
    painter()->fill(path3, StaticPath, transform);
    painter()->stroke(path3, StaticPath, transform);

    // Adding paths into path
    static QCPainterPath path4;
    if (path4.isEmpty()) {
        for (int i = 0; i < 8; i++) {
            // Create path from 8 heart paths
            QTransform t;
            t.translate(center.x(), center.y() - hSize*0.6);
            t.rotateRadians(i * M_PI_2 * 0.5);
            t.scale(0.5, 0.5);
            t.translate(-center.x(), -(center.y() - hSize*0.6));
            path4.addPath(path3, t);
        }
    }
    painter()->translate(w * 0.4, h * 0.4);
    painter()->stroke(path4, StaticPath);

    // Example of reusing parts of paths
    posY += h + margin;
    painter()->resetTransform();
    painter()->save();
    painter()->setRenderHint(QCPainter::RenderHint::HighQualityStroking, true);
    static QCPainterPath pathGraphLine;
    static QCPainterPath pathGraphArea;
    if (pathGraphLine.isEmpty()) {
        // Create linegraph path.
        pathGraphLine.moveTo(posX, posY);
        const int lines = 200;
        for (int i = 0; i < lines; i++) {
            float fl = float(i) / (lines - 1);
            float lY = posY +
                    sin(8 * fl) * h * 0.15 +
                    sin(20 * fl) * h * 0.05;
            pathGraphLine.lineTo(posX + fl * w, lY);
        }
        // Create fillpath, graph + 2 points.
        pathGraphArea = pathGraphLine;
        pathGraphArea.lineTo(posX + w, posY + h);
        pathGraphArea.lineTo(posX, posY + h);
    }
    QCLinearGradient g1(0, posY, 0, posY + h);
    g1.setStartColor(Qt::black);
    g1.setEndColor(Qt::transparent);
    painter()->setLineJoin(QCPainter::LineJoin::Round);
    painter()->setLineCap(QCPainter::LineCap::Round);
    painter()->setFillStyle(g1);
    painter()->fill(pathGraphArea);
    painter()->setStrokeStyle(Qt::white);
    painter()->setLineWidth(1);
    painter()->stroke(pathGraphLine);
    // Selection is part of line graph.
    int commands = pathGraphLine.commandsSize();
    int startIndex = 0.5f * commands * std::max(sin(0.5f * m_animationTime), 0.0f);
    int count = commands - 0.5f * commands * std::max(sin(-0.5f * m_animationTime), 0.0f);
    QCPainterPath pathSelection = pathGraphLine.sliced(startIndex, count);
    // Selection fill is selection + 2 points
    QCPainterPath pathSelectionFill(pathSelection);
    float selectionFirstX = pathSelection.positionAt(0).x();
    float selectionLastX = pathSelection.positionAt(pathSelection.commandsSize() - 1).x();
    pathSelectionFill.lineTo(selectionLastX, posY + h);
    pathSelectionFill.lineTo(selectionFirstX, posY + h);
    painter()->setFillStyle("#20ffff00");
    painter()->fill(pathSelectionFill);
    painter()->setStrokeStyle("#80f04020");
    painter()->setLineWidth(16);
    painter()->stroke(pathSelection);
    painter()->restore();
}

void GalleryItemRenderer::drawTransforms() {
    int rects = 3;
    float margin = width()*0.1f;
    float w = width() / (rects) - margin;
    float posX = margin/2;
    float posY = margin;
    float lineWidth = width()*0.01f;

    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setStrokeStyle(0xFF000000);
    painter()->setLineWidth(lineWidth);
    painter()->fillRect(posX, posY, w, w);
    painter()->strokeRect(posX, posY, w, w);
    // Save the painter state
    painter()->save();

    // Modify painter state
    posX += w + margin;
    painter()->setFillStyle(0xFF900000);
    painter()->setStrokeStyle(0xFFE00000);
    painter()->setLineWidth(lineWidth*2);
    painter()->setLineCap(QCPainter::LineCap::Round);
    painter()->setLineJoin(QCPainter::LineJoin::Round);
    painter()->translate(posX+w/2, posY+w/2);
    painter()->rotate(m_animationSine);
    painter()->scale(0.5f + 0.5f*m_animationSine);
    painter()->translate(-(posX+w/2), -(posY+w/2));
    painter()->fillRect(posX, posY, w, w);
    painter()->strokeRect(posX, posY, w, w);

    posX += w + margin;
    // And restore painter state back
    painter()->restore();
    painter()->fillRect(posX, posY, w, w);
    painter()->strokeRect(posX, posY, w, w);

    posY += w+margin;
    posX = margin/2;
    painter()->setStrokeStyle(0xFF40E040);
    painter()->save();
    painter()->translate(w/8 * m_animationSine, w/4 * m_animationSine);
    painter()->strokeRect(posX, posY, w/2, w/2);
    posX += w + margin;
    painter()->translate(w/8 * m_animationSine, w/4 * m_animationSine);
    painter()->strokeRect(posX, posY, w/2, w/2);
    posX += w + margin;
    painter()->translate(w/8 * m_animationSine, w/4 * m_animationSine);
    painter()->strokeRect(posX, posY, w/2, w/2);
    painter()->restore();

    posY += w+margin;
    posX = margin/2;
    painter()->save();
    painter()->translate(posX+w/2, posY+w/2);
    painter()->skew(0.4f * m_animationSine);
    painter()->translate(-(posX+w/2), -(posY+w/2));
    painter()->strokeRect(posX, posY, w, w);
    painter()->restore();

    posX += w + margin;
    painter()->save();
    painter()->translate(posX+w/2, posY+w/2);
    painter()->skew(0.0f, 0.4f * m_animationSine);
    painter()->translate(-(posX+w/2), -(posY+w/2));
    painter()->strokeRect(posX, posY, w, w);
    painter()->restore();

    posX += w + margin;
    QTransform t;
    t.translate(posX+w/2, posY+w/2);
    t.rotate(m_animationTime*20);
    float scale = 0.5f + 0.5f*m_animationSine;
    t.scale(scale, scale);
    t.translate(-(posX+w/2), -(posY+w/2));
    painter()->setTransform(t);
    painter()->strokeRect(posX, posY, w, w);
    painter()->setClipRect(posX, posY, w, w);
    // Duplicate transformation
    QTransform t2 = painter()->getTransform();
    painter()->transform(t2);

    painter()->setStrokeStyle(0xFF40E0E0);
    painter()->strokeRect(posX, posY, w, w);
    painter()->resetClipping();

    posY += w+margin;
    posX = margin/2;
    painter()->resetTransform();
    painter()->setLineWidth(lineWidth);
    QRectF r(posX, posY, w*2, w);
    painter()->strokeRect(r);
    r.adjust(lineWidth/2, lineWidth/2, -lineWidth/2, -lineWidth/2);
    painter()->setClipRect(r);
    painter()->beginPath();
    painter()->circle(posX + m_animationSine*w*2, posY + w/2, w/2);
    painter()->fill();
    painter()->resetClipping();
}

void GalleryItemRenderer::drawAntialiasing() {
    int rects = 3;
    float margin = width()*0.1f;
    float w = width() / (rects) - margin;
    float posX = margin/2;
    float posY = margin;
    float lineWidth = width()*0.01f;

    painter()->setLineCap(QCPainter::LineCap::Round);
    painter()->setStrokeStyle(0xFFFFFFFF);
    painter()->setLineWidth(lineWidth);
    painter()->setAntialias(0);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w/4);
    painter()->stroke();
    posX += w + margin;
    painter()->setAntialias(1.0f);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w/4);
    painter()->stroke();
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w/4);
    painter()->stroke();

    posY += w+margin;
    posX = margin/2;
    int lines = 12;
    painter()->setAntialias(0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
         // Align to improve thin lines painting
        painter()->moveTo(int(posX), int(posY+i*w/lines));
        painter()->lineTo(int(posX+w), int(posY+i*w/lines));
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines);
        painter()->stroke();
    }

    posY += w+margin;
    posX = margin/2;
    painter()->setAntialias(0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }

    posY += w+margin;
    posX = margin/2;
    posY += m_animationSine*4;
    painter()->setAntialias(0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth(i*0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }

}

void GalleryItemRenderer::drawCompositeItem1(float x, float y, float w, float h, QCPainter::CompositeOperation mode)
{
    painter()->setGlobalCompositeOperation(QCPainter::CompositeOperation::SourceOver);
    // First item
    painter()->beginPath();
    painter()->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter()->setFillStyle("#6060d0");
    painter()->fill();

    painter()->setGlobalCompositeOperation(mode);

    // Second item
    painter()->beginPath();
    painter()->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * m_animationSine), w * 0.4f);
    painter()->setFillStyle("#ff0000");
    painter()->fill();
}

void GalleryItemRenderer::drawCompositeItem2(float x, float y, float w, float h, QCPainter::CompositeOperation mode)
{
    painter()->setGlobalCompositeOperation(QCPainter::CompositeOperation::SourceOver);
    painter()->setLineWidth(w * 0.05f);
    // First item
    painter()->beginPath();
    painter()->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter()->setFillStyle("#6060d0");
    painter()->fill();
    painter()->setStrokeStyle("#ffffff");
    painter()->stroke();

    painter()->setGlobalCompositeOperation(mode);

    // Second item
    painter()->beginPath();
    painter()->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * m_animationSine), w * 0.4f);
    painter()->setFillStyle("#ff0000");
    painter()->fill();
    painter()->setStrokeStyle("#000000");
    painter()->stroke();
}

void GalleryItemRenderer::drawCompositeItem3(float x, float y, float w, float h, QCPainter::CompositeOperation mode)
{
    painter()->setGlobalCompositeOperation(QCPainter::CompositeOperation::SourceOver);
    painter()->setLineWidth(w * 0.05f);
    painter()->setAntialias(10.0f);
    // First item
    painter()->beginPath();
    painter()->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter()->setFillStyle("#6060d0");
    painter()->fill();
    painter()->setStrokeStyle("#ffffff");
    painter()->stroke();

    painter()->setGlobalCompositeOperation(mode);

    // Second item
    painter()->setGlobalAlpha(0.5);
    painter()->beginPath();
    painter()->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * m_animationSine), w * 0.4f);
    painter()->setFillStyle("#ff0000");
    painter()->fill();
    painter()->setStrokeStyle("#000000");
    painter()->stroke();
    painter()->setGlobalAlpha(1.0f);
}

void GalleryItemRenderer::drawCompositeModes() {
    int rects = 3;
    float margin = width() * 0.1f;
    float w = width() / rects - margin;
    float h = w * 1.2;
    float posX = margin/2;
    float posY = margin;

    drawCompositeItem1(posX, posY, w, h, QCPainter::CompositeOperation::SourceOver);
    posX += w + margin;
    drawCompositeItem1(posX, posY, w, h, QCPainter::CompositeOperation::SourceAtop);
    posX += w + margin;
    drawCompositeItem1(posX, posY, w, h, QCPainter::CompositeOperation::DestinationOut);

    posY += h + margin;
    posX = margin/2;
    drawCompositeItem2(posX, posY, w, h, QCPainter::CompositeOperation::SourceOver);
    posX += w + margin;
    drawCompositeItem2(posX, posY, w, h, QCPainter::CompositeOperation::SourceAtop);
    posX += w + margin;
    drawCompositeItem2(posX, posY, w, h, QCPainter::CompositeOperation::DestinationOut);

    posY += h + margin;
    posX = margin/2;
    drawCompositeItem3(posX, posY, w, h, QCPainter::CompositeOperation::SourceOver);
    posX += w + margin;
    drawCompositeItem3(posX, posY, w, h, QCPainter::CompositeOperation::SourceAtop);
    posX += w + margin;
    drawCompositeItem3(posX, posY, w, h, QCPainter::CompositeOperation::DestinationOut);
}

void GalleryItemRenderer::drawGridPatterns() {
    auto *p = painter();
    int rects = 2;
    float margin = width() * 0.05f;
    float w = width() / rects - margin;
    float h = height() * 0.25;
    float posX = margin/2;
    float posY = margin;

    QRectF rect1(posX, posY, w, h);
    float cellZoom = 1.0 + 0.95 * sin(0.5 * m_animationTime);
    QPointF cp1 = rect1.center();
    // Minor grid
    QCGridPattern gp1(cp1.x(), cp1.y(), w * 0.1 * cellZoom, h * 0.1 * cellZoom);
    gp1.setLineColor("#404040");
    gp1.setBackgroundColor("#202020");
    p->setFillStyle(gp1);
    p->fillRect(rect1);
    // Major grid
    QCGridPattern gp2(cp1.x(), cp1.y(), w * cellZoom, h * cellZoom);
    gp2.setLineColor("#d0d0d0");
    gp2.setBackgroundColor(Qt::transparent);
    p->setFillStyle(gp2);
    p->fillRect(rect1);

    posX += w + margin;
    QRectF rect2(posX, posY, w, h);
    QPointF cp2 = rect2.center();
    float g1 = w * 0.1f;
    QCGridPattern gp3(cp2.x(), cp2.y(), g1, g1);
    QColor color1(m_animationSine * 150, 80, 40);
    gp3.setLineColor(color1);
    gp3.setBackgroundColor("#202020");
    gp3.setLineWidth(g1 * 0.5f);
    gp3.setFeather(g1 * 0.5f);
    gp3.setRotation(m_animationTime);
    p->setFillStyle(gp3);
    p->fillRect(rect2);

    posY += h + margin;
    posX = margin/2;
    QRectF rect3(posX, posY, w, h);
    QCGridPattern gp4;
    float bar = 20;
    gp4.setStartPosition(15 * m_animationTime, 0);
    gp4.setCellSize(bar, 0);
    gp4.setLineWidth(bar * 0.5f);
    gp4.setRotation(M_PI / 4);
    QCGridPattern gp5;
    gp5.setCellSize(0, 4);
    gp5.setBackgroundColor("#202020");
    gp5.setLineColor("#404040");
    p->setStrokeStyle(gp4);
    p->setFillStyle(gp5);
    p->setLineWidth(10);
    p->beginPath();
    p->roundRect(rect3, 20);
    p->fill();
    p->stroke();

    posX += w + margin;
    QRectF rect4(posX, posY, w, h);
    QPointF cp4 = rect4.center();
    QCGridPattern gp6;
    float strokeW = 10 * m_animationSine;
    gp6.setLineColor(Qt::transparent);
    gp6.setBackgroundColor(Qt::white);
    gp6.setStartPosition(cp4.x(), cp4.y());
    gp6.setCellSize(rect4.width() / 5, rect4.height() / 5);
    gp6.setLineWidth(strokeW);
    p->setStrokeStyle(gp6);
    p->setLineWidth(4);
    p->beginPath();
    p->roundRect(rect4, 10);
    p->stroke();
}

void GalleryItemRenderer::drawRectangularShadows() {
    auto *p = painter();

    int rects = 3;
    float margin = width() * 0.1f;
    float w = width() / rects - margin;
    float h = height() * 0.15;
    float posX = margin/2;
    float posY = margin;

    QRectF rect1(posX, posY, w, h);
    QCBoxShadow bs1(rect1);
    p->drawBoxShadow(bs1);
    rect1.translate(w + margin, 0);
    bs1.setRect(rect1);
    bs1.setBlur(0.2 * w);
    p->drawBoxShadow(bs1);
    rect1.translate(w + margin, 0);
    bs1.setRect(rect1);
    bs1.setBlur(0.4 * w);
    p->drawBoxShadow(bs1);

    posY += h + margin;
    QRectF rect2(posX, posY, w, h);
    QCBoxShadow bs2(rect2);
    bs2.setRadius(h/2);
    p->drawBoxShadow(bs2);
    rect2.translate(w + margin, 0);
    bs2.setRect(rect2);
    bs2.setBlur(0.2 * w);
    p->drawBoxShadow(bs2);
    rect2.translate(w + margin, 0);
    bs2.setRect(rect2);
    bs2.setBlur(0.4 * w);
    p->drawBoxShadow(bs2);

    posY += h + margin;
    QRectF rect3(posX, posY, w, h);
    QCBoxShadow bs3(rect3);
    bs3.setBlur(h/8);
    bs3.setColor("#202020");
    bs3.setTopLeftRadius(h/4 + h/4 * sin(m_animationTime));
    bs3.setTopRightRadius(h/4 + h/4 * sin(m_animationTime + 0.5 * M_PI));
    bs3.setBottomLeftRadius(h/4 + h/4 * sin(m_animationTime + 1.0 * M_PI));
    bs3.setBottomRightRadius(h/4 + h/4 * sin(m_animationTime + 1.5 * M_PI));
    p->drawBoxShadow(bs3);
    rect3.translate(w + margin, 0);
    bs3.setColor("#ffffff");
    bs3.setRect(rect3);
    p->drawBoxShadow(bs3);
    rect3.translate(w + margin, 0);
    bs3.setColor("#40ffff00");
    bs3.setRect(rect3);
    p->drawBoxShadow(bs3);

    posY += h + margin;
    float blur = 30;
    float radius = 5 + m_animationSine * (h / 2 - 5);
    float shadowOffsetX = blur * 0.3;
    float shadowOffsetY = blur * 0.3;
    QRectF buttonRect(posX, posY, w, h);
    // Double shadows
    QRectF shadow1Rect = buttonRect.translated(-shadowOffsetX, shadowOffsetY);
    QCBoxShadow shadow;
    shadow.setBlur(blur);
    shadow.setRadius(radius);
    shadow.setRect(shadow1Rect);
    shadow.setColor("#40000000");
    p->drawBoxShadow(shadow);
    QRectF shadow2Rect = buttonRect.translated(shadowOffsetX, -shadowOffsetY);
    shadow.setRect(shadow2Rect);
    shadow.setColor("#10ffffff");
    p->drawBoxShadow(shadow);
    // Button on top of shadows
    p->beginPath();
    p->roundRect(buttonRect, radius);
    p->setFillStyle("#404040");
    p->fill();

    buttonRect.translate(w + margin, 0);
    shadow1Rect.translate(w + margin, 0);
    shadow2Rect.translate(w + margin, 0);
    radius = h / 8;
    shadow.setRadius(radius);
    shadow.setBlur(h/16 + m_animationSine * h/2);
    // Double shadows
    shadow.setRect(shadow1Rect);
    shadow.setColor("#80000000");
    p->drawBoxShadow(shadow);
    shadow.setRect(shadow2Rect);
    shadow.setColor("#20ffffff");
    p->drawBoxShadow(shadow);
    // Button on top of shadows
    p->beginPath();
    p->roundRect(buttonRect, radius);
    p->setFillStyle("#404040");
    p->fill();

    buttonRect.translate(w + margin, 0);
    shadow1Rect.translate(w + margin, 0);
    shadow2Rect.translate(w + margin, 0);
    radius = h / 8;
    shadow.setRadius(radius);
    shadow.setBlur(h/4);
    // Double shadows
    shadow1Rect.translate(h/8 * sin(m_animationTime), 0);
    shadow.setRect(shadow1Rect);
    shadow.setColor("#40000000");
    p->drawBoxShadow(shadow);
    shadow2Rect.translate(h/8 * sin(m_animationTime + M_PI), 0);
    shadow.setRect(shadow2Rect);
    shadow.setColor("#10ffffff");
    p->drawBoxShadow(shadow);
    // Button on top of shadows
    p->beginPath();
    p->roundRect(buttonRect, radius);
    p->setFillStyle("#404040");
    p->fill();

}

void GalleryItemRenderer::drawCustomBrushes() {
    float w = width() * 0.8;
    float h = w * 0.3;
    float posX = width() * 0.5 - w * 0.5;
    float posY = h * 0.5;
    float margin = width()*0.02f;
    float border = h * 0.1;

    //painter()->setGlobalAlpha(0.2);
    QRectF rect1(posX,posY,w,h);
    // Set the color for custom brush
    m_customBrush.setData1(QVector4D(1.0, 0.0, 0.0, 1.0));
    painter()->setAntialias(1.0f);
    painter()->setLineWidth(border);
    painter()->setFillStyle(m_customBrush);
    painter()->setStrokeStyle(Qt::transparent);
    painter()->beginPath();
    painter()->roundRect(rect1, border * 2);
    painter()->fill();
    painter()->stroke();

    posY += h + border + margin;
    QRectF rect2(posX,posY,w,h);
    // Adjust the color in custom data
    m_customBrush.setData1(QVector4D(1.0, 1.0, 0.0, 1.0));
    painter()->setStrokeStyle(m_customBrush);
    painter()->setFillStyle(Qt::transparent);
    painter()->beginPath();
    painter()->roundRect(rect2, border * 2);
    painter()->fill();
    painter()->stroke();

    posY += h + border + margin;
    QRectF rect3(posX,posY,w,h);
    // Adjust the color in custom data
    m_customBrush.setData1(QVector4D(1.0, 1.0, 1.0, 1.0));
    m_customBrush2.setData1(QVector4D(1.0, 1.0, 1.0, 1.0));
    painter()->setStrokeStyle(m_customBrush2);
    painter()->setFillStyle(m_customBrush);
    painter()->setLineWidth(border*0.3);
    painter()->beginPath();
    painter()->roundRect(rect3, border * 2);
    painter()->fill();
    painter()->stroke();

    posY += h + border + margin;
    posX = width() * 0.5 - w * 0.5;
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->bezierCurveTo(posX+w*0.2f*m_animationSine, posY+h*0.8f*m_animationSine, posX+w*0.8f*m_animationSine, posY-h*0.2f*m_animationSine, posX+w, posY+h);
    painter()->setLineCap(QCPainter::LineCap::Round);
    m_customBrush2.setData1(QVector4D(0.0, 0.0, 10.0, 1.0));
    painter()->setStrokeStyle(m_customBrush2);
    painter()->setLineWidth(20);
    painter()->stroke();
}

void GalleryItemRenderer::drawTextsFonts() {

    float margin = height() * 0.04f;
    float posX = margin/2;
    float posY = margin;
    painter()->reset();
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->fillText("This is text without setting any font", posX, posY);

    posY += margin;
    QFont f;
    painter()->setFont(f);
    painter()->fillText("This is text with default font", posX, posY);

    posY += margin;
    painter()->fillText("This is text aligned to pixels", int(posX), int(posY));

    posY += margin;
    painter()->setFillStyle(0xFFD06060);
    f.setPixelSize(20);
    painter()->setFont(f);
    painter()->fillText("Size 20px", posX, posY);
    posY += margin;
    f.setPointSize(20);
    painter()->setFont(f);
    painter()->fillText("Size 20pt", posX, posY);

    posY += margin;
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setTextAntialias(1.0 + m_animationSine * 5.0);
    f.setPixelSize(14);
    painter()->setFont(f);
    painter()->fillText("Size 14px, blurring", posX, posY);
    posY += margin;
    f.setPixelSize(24);
    painter()->setFont(f);
    painter()->fillText("Size 24px, blurring", posX, posY);
    posY += 1.3 * margin;
    f.setPixelSize(34);
    painter()->setFont(f);
    painter()->fillText("Size 34px, blurring", posX, posY);
    painter()->setTextAntialias(1.0);

    posY += margin;
    painter()->setFillStyle(0xFFFFFFFF);
    f.setPixelSize(margin * 0.8);
    f.setWeight(QFont::Weight::Light);
    f.setItalic(false);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Light", posX, posY);
    f.setItalic(true);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Light Italic", posX, posY);

    f.setWeight(QFont::Weight::Normal);
    f.setItalic(false);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Normal", posX, posY);
    f.setItalic(true);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Normal Italic", posX, posY);

    f.setWeight(QFont::Weight::Bold);
    f.setItalic(false);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Bold", posX, posY);
    f.setItalic(true);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Bold Italic", posX, posY);

    f.setItalic(false);
    f.setCapitalization(QFont::SmallCaps);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: SmallCaps", posX, posY);

    posY += 2*margin;
    QFont f2("Pacifico", margin);
    painter()->setFont(f2);
    painter()->fillText("This is text with custom font", posX, posY);

    posY += 2*margin;
    QFont f5;
    f5.setPixelSize(margin*0.8f);
    f5.setLetterSpacing(QFont::AbsoluteSpacing, 6 * m_animationSine - 2);
    painter()->setFont(f5);
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->fillText("This is text with varying letter spacing", posX, posY);

    posY += 2*margin;
    f5.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    f5.setWordSpacing(20 * m_animationSine - 5);
    painter()->setFont(f5);
    painter()->fillText("This is text with varying word spacing", posX, posY);

    /* TODO: Test how font size animation behaves
    posY += 2*margin;
    f5.setSize(margin*0.5 + margin*4*m_animationSine);
    painter()->setFont(f5);
    painter()->fillText("Font SIZES", posX, posY);
    */
}

void GalleryItemRenderer::drawTextsBrushes() {
    float margin = height() * 0.04f;
    float posX = margin * 0.5;
    float posY = margin * 2;
    float fontSize = 40;
    QFont f;
    f.setPixelSize(fontSize);
    painter()->setFont(f);
    float w = width();
    QCLinearGradient g1(0, 0, 0, 0);
    g1.setStartColor(QColor(255,255,255));
    g1.setEndColor(QColor(0,0,0));
    g1.setStartPosition(posX + (w*0.5f * m_animationSine), 0);
    g1.setEndPosition(posX + w - (w*0.5f * m_animationSine), 0);
    painter()->setFillStyle(g1);
    painter()->fillText("Text with linear gradient", posX, posY);

    posY += margin + fontSize;
    float h = 80;
    QCRadialGradient g2;
    g2.setStartColor(QColor(200, 200, 50, 255));
    g2.setColorAt(0.5, QColor(240, 100, 50, 255));
    g2.setEndColor(QColor(0,0,0,0));
    g2.setCenterPosition(posX + w/2, posY + h/2);
    g2.setOuterRadius(0.6*w*m_animationSine);
    g2.setInnerRadius(0.1*w*m_animationSine);
    painter()->setFillStyle(g2);
    painter()->fillText("Text with radial gradient", posX, posY);

    fontSize = 60;
    posY += margin + fontSize;
    QCConicalGradient g4;
    g4.setCenterPosition(width() * 0.5, posY - fontSize * 0.5);
    g4.setAngle(m_animationTime);
    g4.setStartColor(QColor(255, 255, 0, 255));
    g4.setColorAt(0.25, QColor(0, 255, 0, 255));
    g4.setColorAt(0.5, QColor(0, 0, 255, 255));
    g4.setColorAt(0.75, QColor(255, 0, 255, 255));
    g4.setEndColor(QColor(255, 255, 0, 255));
    painter()->setFillStyle(g4);
    f.setPixelSize(fontSize);
    painter()->setFont(f);
    //painter()->fillText("MULTIGRADIENT", posX, posY);
    m_text1.setText("MULTIGRADIENT");
    m_text1.setX(posX);
    m_text1.setY(posY);
    painter()->fillText(m_text1);

    posY += margin + fontSize;
    QCImagePattern p1 = QCImagePattern(m_patternImage2);
    p1.setImageSize(64, 64);
    p1.setStartPosition(0, m_animationTime * 10);
    painter()->setFillStyle(p1);
    painter()->fillText("IMAGE PATTERN", posX, posY);

    posY += margin + fontSize;
    painter()->setFillStyle(m_customBrush3);
    painter()->fillText("Custom Brush", posX, posY);

    fontSize = 40;
    posY += margin + fontSize;
    f.setPixelSize(fontSize);
    painter()->setFont(f);
    // Font color in data1
    m_customBrush4.setData1(QVector4D(0.0, 0.0, 0.0, 1.0));
    painter()->setFillStyle(m_customBrush4);
    QString glowingString("Text with some glowing");
    painter()->fillText(glowingString, posX, posY);
}

void GalleryItemRenderer::drawTextsAlignments() {
    float margin = height() * 0.15f;
    float fontSize = height() * 0.03f;
    float posX = margin/2;
    float posY = margin/2;

    QFont f3;
    f3.setPixelSize(fontSize);
    painter()->setFont(f3);

    // Test horizontal alignments
    posX = width() * 0.5f;
    painter()->setFillStyle(0xFF808080);
    painter()->fillRect(posX-1, posY-fontSize, 2, fontSize*3);
    painter()->setTextBaseline(QCPainter::TextBaseline::Bottom);
    painter()->setFillStyle(0xFFFF9090);
    painter()->setTextAlign(QCPainter::TextAlign::Left);
    painter()->fillText("Left", posX, posY);
    painter()->setFillStyle(0xFF90FF90);
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    painter()->fillText("Center", posX, posY+fontSize);
    painter()->setFillStyle(0xFF9090FF);
    painter()->setTextAlign(QCPainter::TextAlign::Right);
    painter()->fillText("Right", posX, posY+fontSize*2);

    // Test vertical (baseline) alignments
    posY += margin;
    posX = width() * 0.1f;
    const float spacing = width() * 0.19;
    painter()->setFillStyle(0xFF808080);
    painter()->fillRect(posX, posY-1, width()*0.8f, 2);
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    painter()->setFillStyle(0xFFFF90FF);
    painter()->setTextBaseline(QCPainter::TextBaseline::Top);
    painter()->fillText("Top", posX, posY);
    painter()->setFillStyle(0xFFFFFF90);
    painter()->setTextBaseline(QCPainter::TextBaseline::Hanging);
    painter()->fillText("Hanging", posX+spacing, posY);
    painter()->setFillStyle(0xFF9090FF);
    painter()->setTextBaseline(QCPainter::TextBaseline::Middle);
    painter()->fillText("Middle", posX + 2 * spacing, posY);
    painter()->setFillStyle(0xFF90FF90);
    painter()->setTextBaseline(QCPainter::TextBaseline::Alphabetic);
    painter()->fillText("Alphabetic", posX + 3 * spacing, posY);
    painter()->setFillStyle(0xFFFF9090);
    painter()->setTextBaseline(QCPainter::TextBaseline::Bottom);
    painter()->fillText("Bottom", posX + 4 * spacing, posY);

    // Test horizontal alignments with rect
    posY += margin;
    QRectF r1(width() * 0.5f, posY, 0, fontSize);
    float animX = width() * 0.4f * m_animationSine;
    r1.adjust(-animX, 0, animX, 0);
    painter()->setStrokeStyle(0xFF808080);
    painter()->strokeRect(r1);

    painter()->setTextBaseline(QCPainter::TextBaseline::Middle);
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setTextAlign(QCPainter::TextAlign::Left);
    painter()->fillText("Left", r1);
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    painter()->fillText("Center", r1);
    painter()->setTextAlign(QCPainter::TextAlign::Right);
    painter()->fillText("Right", r1);

    // Test vertical (baseline) alignments with rect
    posY += margin;
    posX = width() * 0.1f;
    float rectW = width() * 0.8f;
    QRectF r2(posX, posY, rectW, margin * m_animationSine);
    painter()->setStrokeStyle(0xFF808080);
    painter()->strokeRect(r2);

    painter()->setTextAlign(QCPainter::TextAlign::Left);
    painter()->setTextBaseline(QCPainter::TextBaseline::Alphabetic);
    painter()->fillText("Left & Alphabetic", r2);
    painter()->setTextAlign(QCPainter::TextAlign::Right);
    painter()->setTextBaseline(QCPainter::TextBaseline::Bottom);
    painter()->fillText("Right & Bottom", r2);
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    painter()->setTextBaseline(QCPainter::TextBaseline::Middle);
    painter()->fillText("Center & Middle", r2);
    painter()->setTextBaseline(QCPainter::TextBaseline::Top);
    painter()->fillText("Center & Top", r2);

    posY += margin;
    painter()->strokeRect(posX, posY, rectW, 8.5 * fontSize);
    painter()->setTextDirection(QCPainter::TextDirection::LeftToRight);
    QRectF r3(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCPainter::TextAlign::Start);
    painter()->fillText("LeftToRight: Start-aligned", r3);
    posY += fontSize;
    QRectF r4(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCPainter::TextAlign::End);
    painter()->fillText("LeftToRight: End-aligned", r4);
    painter()->setTextDirection(QCPainter::TextDirection::RightToLeft);
    posY += fontSize*2;
    QRectF r5(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCPainter::TextAlign::Start);
    painter()->fillText("RightToLeft: Start-aligned", r5);
    posY += fontSize;
    QRectF r6(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCPainter::TextAlign::End);
    painter()->fillText("RightToLeft: End-aligned", r6);
    // Autodetect direction from the text string.
    painter()->setTextDirection(QCPainter::TextDirection::Auto);
    posY += fontSize*2;
    QRectF r7(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCPainter::TextAlign::Start);
    painter()->fillText("English", r7);
    posY += fontSize;
    QRectF r8(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCPainter::TextAlign::Start);
    painter()->fillText("خامل", r8);
    painter()->setTextDirection(QCPainter::TextDirection::LeftToRight);

}

void GalleryItemRenderer::drawTextsWrapping() {
    float margin = height() * 0.05f;
    float fontSize = height() * 0.03f;
    float posX = margin;
    float posY = margin;

    QFont f3;
    f3.setPixelSize(fontSize);
    painter()->setFont(f3);

    // Test text alignment and line wrapping
    QString s("This is a test string which is slightly longer one. Yes, we want long string for this.");
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setTextWrapMode(QCPainter::WrapMode::WordWrap);
    painter()->setTextBaseline(QCPainter::TextBaseline::Top);
    painter()->setTextAlign(QCPainter::TextAlign::Left);
    painter()->setTextLineHeight(-10);
    float rectW = (width() - 2 * margin) - m_animationSine*width()*0.4f;
    float rectH = 120;
    QRectF rect1(posX, posY, rectW, rectH);
    painter()->setStrokeStyle(0xFF808080);
    painter()->strokeRect(rect1);
    painter()->fillText(s, rect1);
    QRectF boundingRect1 = painter()->textBoundingBox(s, rect1);
    painter()->setStrokeStyle(0xFFFF8080);
    painter()->strokeRect(boundingRect1);

    posY += rectH + margin;
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    painter()->setTextBaseline(QCPainter::TextBaseline::Middle);
    painter()->setTextLineHeight(0);
    QRectF rect2(posX, posY, rectW, rectH);
    painter()->setStrokeStyle(0xFF808080);
    painter()->strokeRect(rect2);
    painter()->fillText(s, rect2);
    QRectF boundingRect2 = painter()->textBoundingBox(s, rect2);
    painter()->setStrokeStyle(0xFFFF8080);
    painter()->strokeRect(boundingRect2);

    posY += rectH + margin;
    painter()->setTextAlign(QCPainter::TextAlign::Right);
    painter()->setTextBaseline(QCPainter::TextBaseline::Bottom);
    painter()->setTextLineHeight(10);
    QRectF rect3(posX, posY, rectW, rectH);
    painter()->setStrokeStyle(0xFF808080);
    painter()->strokeRect(rect3);
    painter()->fillText(s, rect3);
    QRectF boundingRect3 = painter()->textBoundingBox(s, rect3);
    painter()->setStrokeStyle(0xFFFF8080);
    painter()->strokeRect(boundingRect3);

    posY += 1.6 * rectH + margin;
    posX = 0.5f * width();
    QFont f4;
    f4.setPixelSize(18);
    painter()->setFont(f4);
    painter()->setTextLineHeight(0);
    painter()->setTextWrapMode(QCPainter::WrapMode::WordWrap);
    painter()->setTextBaseline(QCPainter::TextBaseline::Middle);
    //painter()->setTextBaseline(QCPainter::TextBaseline::Bottom);
    //painter()->setTextBaseline(QCPainter::TextBaseline::Top);
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    //painter()->setTextAlign(QCPainter::TextAlign::Right);
    //painter()->setTextAlign(QCPainter::TextAlign::Left);
    float circleRadius = 40 + 50 * m_animationSine;
    float textMargin = 10;
    float textW = 2 * (circleRadius - textMargin);
    QString text = QStringLiteral("Circle radius: %1").arg(qRound(circleRadius));
    painter()->fillText(text, posX, posY, textW);
    painter()->beginPath();
    painter()->setLineWidth(1);
    painter()->circle(posX, posY, circleRadius);
    painter()->setStrokeStyle(0xFF808080);
    painter()->stroke();
    QRectF boundingRect4 = painter()->textBoundingBox(text, posX, posY, textW);
    painter()->setStrokeStyle(0xFFFF8080);
    painter()->strokeRect(boundingRect4);
}

void GalleryItemRenderer::drawImages() {

    float margin = height()*0.05f;
    float posX = width()*0.1f;
    float posY = margin;

    QRectF rect1(0, 0, width(), height());
    // Tint background image darker
    auto pattern = QCImagePattern(m_patternImage2);
    pattern.setTintColor(QColor(10, 10, 10));
    painter()->setFillStyle(pattern);
    painter()->fillRect(rect1);

    // Draw image at its native resolution
    float imgX = 0.5 * width() - 0.5 * m_testImage.width();
    painter()->drawImage(m_testImage, imgX, posY);

    // Draw just the "Development" part of the image
    posY += m_testImage.height() + margin;
    float cropW = 105;
    QRectF sourceArea(cropW, 0, m_testImage.width() - cropW, m_testImage.height());
    float cx = width() * 0.5f;
    float r = width() * 0.1f + (m_animationSine * width() * 0.2f);
    QRectF targetArea(cx-r, posY, r*2, width()*0.1);
    painter()->drawImage(m_testImage, sourceArea, targetArea);

    // Draw rotating & scaling & tinted image
    posY += targetArea.height() + height() * 0.15;
    QRectF rect(cx-r * 0.5f, posY - r * 0.5f, r, r);
    QPointF c(rect.x()+rect.width() * 0.5f, rect.y()+rect.height() * 0.5f);
    painter()->save();
    painter()->translate(c);
    painter()->rotate(m_animationTime);
    painter()->translate(-c);
    QColor tintColor(255, 255, 255 - m_animationSine * 200);
    image3Gray.setTintColor(tintColor);
    painter()->drawImage(image3Gray,rect);
    painter()->restore();

    // Scaled images with and without mipmapping
    posY = height() * 0.55f;
    float posY2 = posY + (height() * 0.2f);
    bool useNearest = (m_animationSine > 0.5f);
    float sizeDiff = width() * 0.025f;
    float centerY = 4 * sizeDiff;
    for (int i = 0; i < 7; ++i) {
        float size = (i + 1) * sizeDiff;
        QRectF rect2(posX, posY+centerY, size, size);
        painter()->drawImage(useNearest ? image3Nearest : image3Plain, rect2);
        QRectF rect3(posX, posY2+centerY, size, size);
        painter()->drawImage(useNearest ? image3NearestMips : image3Mips, rect3);
        posX += size + 2;
    }
    QFont font;
    font.setPixelSize(QCPainter::mmToPx(4));
    painter()->setTextAlign(QCPainter::TextAlign::Center);
    painter()->setTextBaseline(QCPainter::TextBaseline::Top);
    QString scaling = useNearest ? "NEAREST" : "LINEAR";
    QString offString = QString("MIPMAPS: OFF, TEXTURE: %1").arg(scaling);
    QString onString = QString("MIPMAPS: ON, TEXTURE: %1").arg(scaling);
    float text1PosY = posY + 0.5f * centerY;
    float text2PosY = posY2 + 0.5f * centerY;
    painter()->setFillStyle(QColorConstants::White);
    painter()->setFont(font);
    painter()->fillText(offString, cx, text1PosY);
    painter()->fillText(onString, cx, text2PosY);
}
/*
void GalleryItemRenderer::drawFrameBuffers()
{
    int rects = 2;
    float dp = painter()->devicePixelRatio();
    float dpWidth = width() * dp;
    float dpHeight = height() * dp;
    float margin = dpWidth*0.1f;
    float w = (dpWidth / rects) - margin;
    float posX = margin/2;
    float posY = margin;
    float lineWidth = 2;
    QOpenGLFunctions glF(QOpenGLContext::currentContext());

    // Cancel current frame as we will instead paint into fbo.
    // We can do this instead of endFrame() as there hasn't
    // been QCPainter commands for this item yet.
    painter()->cancelFrame();

    // Create and bind fbo1 into use
    if (!m_fbo1) {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo1 = new QOpenGLFramebufferObject(w, w, format);
        // Clear fbo intially
        m_fbo1->bind();
        glF.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glF.glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    }
    m_fbo1->bind();

    // Begin QCPainter frame for fbo1
    painter()->beginFrame(m_fbo1->width(), m_fbo1->height());

    // Paint borders
    QRectF rect1(lineWidth/2, lineWidth/2, w-lineWidth, w-lineWidth);
    painter()->setStrokeStyle("#ffffff");
    painter()->setLineWidth(2);
    painter()->strokeRect(rect1);

    // Paint random circles inside rect
    painter()->beginPath();
    float cR = w * 0.1f;
    float cX = cR + lineWidth + rand() % int(w-cR*2-lineWidth*2);
    float cY = cR + lineWidth + rand() % int(w-cR*2-lineWidth*2);
    QColor cColor = QColor(50 + rand() % 200, 50, 50, 100);
    QPointF circlePos = QPointF(cX, cY);
    painter()->circle(circlePos, cR);
    painter()->setStrokeStyle("#000000");
    painter()->setFillStyle(cColor);
    painter()->fill();
    painter()->stroke();

    // Paint rect + text
    painter()->setFillStyle("#202020");
    painter()->fillRect(w*0.1, w*0.35, w*0.8, w*0.3);
    QCFont f(QCFont::DEFAULT_FONT_NORMAL);
    f.setPointSize(20);
    painter()->setFont(f);
    painter()->setFillStyle("#ffffff");
    painter()->setTextAlign(QCPainter::ALIGN_CENTER);
    painter()->setTextBaseline(QCPainter::BASELINE_MIDDLE);
    painter()->fillText("FBO #1", w/2, w/2);

    // We are done with fbo1, so end frame and unbind it
    painter()->endFrame();
    m_fbo1->release();

    // Create and bind fbo2 into use
    if (!m_fbo2) {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo2 = new QOpenGLFramebufferObject(w*2 + margin, w, format);
    }
    m_fbo2->bind();

    // Clear fbo, let's not overdraw this time
    glF.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glF.glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    // Begin QCPainter frame for fbo2
    painter()->beginFrame(m_fbo2->width(), m_fbo2->height());

    // Blit fbo1 into fbo2 few times
    QRect blitSourceRect = QRect(0,0,m_fbo1->width(),m_fbo1->height());
    float centerX = m_fbo2->width()/2 - w/4;
    int blits = 5;
    for (int i=0; i<blits; i++) {
        float range = (float(i+1) / blits) * (m_animationSine - 0.5);
        float blitPosX = centerX + (range * m_fbo2->width());
        QRect blitTargetRect(blitPosX, lineWidth*2, m_fbo1->width()/2, m_fbo1->height()/2);
        QOpenGLFramebufferObject::blitFramebuffer(m_fbo2, blitTargetRect,
                                                  m_fbo1, blitSourceRect,
                                                  GL_COLOR_BUFFER_BIT,
                                                  GL_LINEAR);
    }

    // Paint rect + text
    QRectF rect2(lineWidth/2, lineWidth/2, m_fbo2->width() - lineWidth, m_fbo2->height() - lineWidth);
    painter()->setStrokeStyle("#ffffff");
    painter()->setLineWidth(2);
    painter()->strokeRect(rect2);
    f.setPointSize(20);
    painter()->setFont(f);
    painter()->setFillStyle("#ffffff");
    painter()->setTextAlign(QCPainter::ALIGN_CENTER);
    painter()->setTextBaseline(QCPainter::BASELINE_MIDDLE);
    painter()->fillText("FBO #2 - Blitting", m_fbo2->width()/2, m_fbo2->height()/4);

    // We are done with fbo2, so end frame and unbind it
    painter()->endFrame();
    m_fbo2->release();

    // Bind default QtQuick FBO back
    // Handled a bit differently with RENDERNODE
    // TODO: Consider creating helper for these
#ifdef QCPAINTER_USE_RENDERNODE
    QOpenGLFramebufferObject::bindDefault();
    painter()->beginFrameAt(itemData().x*dp, itemData().y*dp, dpWidth, dpHeight);
    glF.glViewport(0, int(itemData().y*dp), int(dpWidth), int(dpHeight));
#elif QC_USE_RHI
    // TODO: Not implemented for RHI
#else
    framebufferObject()->bind();
    painter()->beginFrameAt(0, 0, dpWidth, dpHeight);
#endif

    // Draw fbo1 as image
    QCImage fbo1Image = QCImage::fromFrameBuffer(m_fbo1);
    painter()->drawImage(fbo1Image, posX, posY);

    // Draw rotating & scaling fbo1
    posX += w + margin;
    float r = w * 0.5f + m_animationSine * w * 0.5f;
    QRectF rect(posX + w/2 - r/2, posY + w/2 - r/2, r, r);
    QPointF c(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    painter()->save();
    painter()->translate(c);
    painter()->rotate(m_animationTime);
    painter()->translate(-c);
    painter()->drawImage(fbo1Image, rect);
    painter()->restore();

    // Draw fbo2 as image
    posY += w + margin;
    posX = margin/2;
    QCImage fbo2Image = QCImage::fromFrameBuffer(m_fbo2);
    painter()->drawImage(fbo2Image, posX, posY);
}*/
