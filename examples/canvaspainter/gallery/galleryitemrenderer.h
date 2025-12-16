// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GALLERYITEMRENDERER_H
#define GALLERYITEMRENDERER_H

#include <QtCanvasPainter/qquickcpainterrenderer.h>
#include <QtCanvasPainter/qcpainter.h>
#include <QtCanvasPainter/qcradialgradient.h>
#include <QtCanvasPainter/qccustombrush.h>
#include <QtCanvasPainter/qcimage.h>
#include <QtCanvasPainter/qcpainterpath.h>

#include <QQuickItem>

class GalleryItemRenderer : public QQuickCPainterRenderer
{
public:
    explicit GalleryItemRenderer();
    ~GalleryItemRenderer();

    // Reimplement
    void initializeResources(QCPainter *painter) override;
    void synchronize(QQuickCPainterItem *item) override;
    void paint(QCPainter *painter) override;

private:

    // View - Rectangles
    void drawRectsWithLinearGradient();
    void drawRectsWithRadialGradient();
    void drawRectsWithBoxGradient();
    void drawRectsWithConicalGradients();
    void drawRectsWithImagePattern();
    void drawRectsWithBrushStroke();

    // View - Paths
    void drawPaths();

    // View - Painter Paths
    void drawPainterPaths();

    // View - States and transforms
    void drawTransforms();

    // View - Antialiasing
    void drawAntialiasing();

    // View - Composite
    void drawCompositeItem1(float x, float y, float w, float h, QCPainter::CompositeOperation mode);
    void drawCompositeItem2(float x, float y, float w, float h, QCPainter::CompositeOperation mode);
    void drawCompositeItem3(float x, float y, float w, float h, QCPainter::CompositeOperation mode);
    void drawCompositeModes();

    // View - Grid patterns
    void drawGridPatterns();

    // View - Shadows
    void drawRectangularShadows();

    // View - Custom Brushes
    void drawCustomBrushes();

    // View - Color Effects
    void drawButton(float x, float y, float w, float h, const QString &label);
    void drawColorEffects();

    // View - Texts
    void drawTextsFonts();
    void drawTextsBrushes();
    void drawTextsAlignments();
    void drawTextsWrapping();

    // View - Images
    void drawImages();

    void drawRect(float x, float y, float w, float h);

    //QCPainter *m_painter;
    float m_animationTime = 0.0f;
    float m_animationSine = 0.0f;
    float m_animState = 0.0f;
    float m_viewAlpha = 1.0f;
    float m_viewSaturate = 1.0f;
    int m_viewIndex = 0;
    float m_previousWidth = 0;
    float m_previousHeight = 0;
    bool m_sizeChanged = false;
    float m_topMargin = 0;
    QCRadialGradient m_radGrad;
    QCImage m_testImage;
    QCImage m_patternImage;
    QCImage m_patternImage2;
    QCImage m_patternImage3;
    QCImage image3Gray;
    QCImage image3Plain;
    QCImage image3Nearest;
    QCImage image3Mips;
    QCImage image3NearestMips;
    QCCustomBrush m_customBrush;
    QCCustomBrush m_customBrush2;
    QCCustomBrush m_customBrush3;
    QCCustomBrush m_customBrush4;
    QCPainterPath m_path1;
    QCPainterPath m_path2;
    QCPainterPath m_path3;
    QCPainterPath m_path4;
    QCPainterPath m_pathGraphLine;
    QCPainterPath m_pathGraphArea;
};

#endif // GALLERYITEMRENDERER_H
