// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CANVASPAINTING_CPPTESTS_H
#define CANVASPAINTING_CPPTESTS_H

#include <QObject>
#include <QCanvasPainter>

class CanvasPainterLancelotCppTests : public QObject
{
    Q_OBJECT

public:
    QStringList keys();
    void run(const QString &key, QCanvasPainter *painter, const QSize &canvasPixelSize);

    // Each slot is a test case that is invoked for each of the graphics APIs.
    // They should draw something using painter and width() and height().
    // Resources like QCanvasImage must be created in the slots, do not cache/reuse!
private slots:
    void simpleDrawing();
    void testPathWindingUncachedPath();
    void testPathWinding();
    void testStraightLinesInPathUncachedPath();
    void testStraightLinesInPath();
    void testPathCaching();
    void testPathFill();
    void testCurve();
    void testCurveWithPathCaching();
    void testSomeText();
    void testCanvasPathCommandsWithAndWithoutPathGroup();
    void testHighQualityStroking();

    // the tests below are adapted from the Gallery example
    void testRects();
    void testRects2();
    void testPaths();
    void testPaths2();
    void testTransforms();
    void testTransforms2();
    void testGridPatterns();
    void testShadows();
    void testShadows2();
    void testCompositeModes();
    void testImages();
    void testImages2();
    void testImages3();
    void testHighDpiImages();
    void testAntialiasing();
    void testAntialiasing2();
    void testTextAlignment();
    void testTextWrapping();
    void testTextWrapping2();
    void testTextBrushes();
    void testTextCustomBrushes();
    void testTextFonts();
    void testTextDecorations();
    void testTextDecorationsWrapping();
    void testTextDecorationsBrushes();
    void testTextDecorationsCustomBrushes();
    void testCanvasPathWithAddPath();
    void testTiger();

    // tests adapted from tests/manual/paintertest
    void testClipRect();
    void testClipRectWithTransform();
    void testSaveRestore();
    void testPathFillRule();
    void testStencilClip();
    void testStencilClipTransform();
    void testStencilClipIntersect();
    void testStrokingWithStencilClip();
    void testHighQualityStrokingWithStencilClip();
    void testGradientSpread();
    void testGradientCaching();
    void testTextDirection();
    void testVectorPathStencilClip();    

private:
    float width() const { return canvasPixelSize.width(); }
    float height() const { return canvasPixelSize.height(); }

    void drawCompositeItem1(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode);
    void drawCompositeItem2(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode);
    void drawCompositeItem3(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode);

    void testCurveImpl(int pathGroup);
    void testStrokingImpl();

    QCanvasPainter *painter;
    QSize canvasPixelSize;
};

#endif
