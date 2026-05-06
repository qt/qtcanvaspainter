// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "canvaspainting_cpptests.h"
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>
#include <QImage>

#include <QCanvasLinearGradient>
#include <QCanvasRadialGradient>
#include <QCanvasConicalGradient>
#include <QCanvasBoxGradient>
#include <QCanvasImagePattern>
#include <QCanvasGridPattern>
#include <QCanvasBoxShadow>
#include <QCanvasPath>

QStringList CanvasPainterLancelotCppTests::keys()
{
    QStringList result;
    const QMetaObject *o = metaObject();
    for (int i = o->methodOffset(); i < o->methodCount(); ++i)
        result.append(QString::fromLatin1(o->method(i).methodSignature()).replace("()", ""));

    return result;
}

void CanvasPainterLancelotCppTests::run(const QString &key, QCanvasPainter *painter, const QSize &canvasPixelSize)
{
    this->painter = painter;
    this->canvasPixelSize = canvasPixelSize;

    const QString methodName = key + QLatin1String("()");
    const QMetaObject *o = metaObject();
    for (int i = o->methodOffset(); i < o->methodCount(); ++i) {
        if (methodName == QString::fromLatin1(o->method(i).methodSignature()))
            o->method(i).invoke(this);
    }
}

void CanvasPainterLancelotCppTests::simpleDrawing()
{
    float size = std::min(width(), height());
    QPointF center(width()/2, height()/2);

    // Paint the background circle
    QCanvasRadialGradient gradient1(center.x(), center.y() - size*0.1, size*0.6);
    gradient1.setStartColor("#909090");
    gradient1.setEndColor("#404040");
    painter->beginPath();
    painter->circle(center, size*0.46);
    painter->setFillStyle(gradient1);
    painter->fill();
    painter->setStrokeStyle("#202020");
    painter->setLineWidth(size*0.02);
    painter->stroke();

    // Text line 1
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(size*0.07);
    painter->setFont(font1);
    painter->setFillStyle("#B0D040");
    painter->fillText("Hello", center.x(), center.y() - size*0.18);

    // Text line 2
    QFont font2;
    font2.setWeight(QFont::Weight::Thin);
    font2.setPixelSize(size*0.07);
    painter->setFont(font2);
    painter->fillText(QLatin1String("Lancelot"), center.x(), center.y() - size*0.08);

    // Paint heart
    painter->setFillStyle(Qt::green);
    painter->setLineCap(QCanvasPainter::LineCap::Round);
    painter->setStrokeStyle("#B0D040");
    painter->beginPath();
    painter->moveTo(center.x(), center.y() + size*0.3);
    painter->bezierCurveTo(center.x() - size*0.25, center.y() + size*0.1,
                     center.x() - size*0.05, center.y() + size*0.05,
                     center.x(), center.y() + size*0.15);
    painter->bezierCurveTo(center.x() + size*0.05, center.y() + size*0.05,
                     center.x() + size*0.25, center.y() + size*0.1,
                     center.x(), center.y() + size*0.3);
    painter->stroke();
    painter->fill();
}

void CanvasPainterLancelotCppTests::testPathWindingUncachedPath()
{
    painter->setLineWidth(4);
    painter->setLineJoin(QCanvasPainter::LineJoin::Bevel);

    painter->fillText("CW, Default, set DisableWindingEnforce, clear DisableWindingEnforce", 10, 10);

    QCanvasPath p;
    p.moveTo(20, 20);
    p.lineTo(100, 180);
    p.lineTo(180, 20);
    p.closePath();
    p.moveTo(100, 40);
    p.lineTo(125, 90);
    p.lineTo(75, 90);
    p.closePath();

    QCanvasPath p2;
    p2.moveTo(20, 20);
    p2.lineTo(100, 180);
    p2.lineTo(180, 20);
    p2.closePath();
    p2.setPathWinding(QCanvasPainter::PathWinding::ClockWise);
    p2.moveTo(100, 40);
    p2.lineTo(125, 90);
    p2.lineTo(75, 90);
    p2.closePath();

    painter->setFillStyle(Qt::green);
    painter->setStrokeStyle(Qt::red);

    painter->fill(p2, -1);
    painter->stroke(p2, -1);

    painter->translate(200, 0);
    painter->fill(p, -1);
    painter->stroke(p, -1);

    painter->setRenderHint(QCanvasPainter::RenderHint::DisableWindingEnforce, true);
    painter->translate(200, 0);
    painter->fill(p, -1);
    painter->stroke(p, -1);

    painter->setRenderHint(QCanvasPainter::RenderHint::DisableWindingEnforce, false);
    painter->translate(200, 0);
    painter->fill(p, -1);
    painter->stroke(p, -1);
}

void CanvasPainterLancelotCppTests::testPathWinding()
{
    painter->setLineWidth(4);
    painter->setLineJoin(QCanvasPainter::LineJoin::Bevel);

    painter->fillText("CW, Default, set DisableWindingEnforce, clear DisableWindingEnforce", 10, 10);

    QCanvasPath p;
    p.moveTo(20, 20);
    p.lineTo(100, 180);
    p.lineTo(180, 20);
    p.closePath();
    p.moveTo(100, 40);
    p.lineTo(125, 90);
    p.lineTo(75, 90);
    p.closePath();

    QCanvasPath p2;
    p2.moveTo(20, 20);
    p2.lineTo(100, 180);
    p2.lineTo(180, 20);
    p2.closePath();
    p2.setPathWinding(QCanvasPainter::PathWinding::ClockWise);
    p2.moveTo(100, 40);
    p2.lineTo(125, 90);
    p2.lineTo(75, 90);
    p2.closePath();

    painter->setFillStyle(Qt::green);
    painter->setStrokeStyle(Qt::red);

    painter->fill(p2, 234);
    painter->stroke(p2, 234);

    painter->translate(200, 0);
    painter->fill(p, 234);
    painter->stroke(p, 234);

    painter->setRenderHint(QCanvasPainter::RenderHint::DisableWindingEnforce, true);
    painter->translate(200, 0);
    painter->fill(p, 234);
    painter->stroke(p, 234);

    painter->setRenderHint(QCanvasPainter::RenderHint::DisableWindingEnforce, false);
    painter->translate(200, 0);
    painter->fill(p, 234);
    painter->stroke(p, 234);

    painter->resetTransform();
    painter->translate(0, 200);
    painter->setFillStyle(Qt::black);
    painter->fillText("Repeat the last and first (cache should hit for both)", 10, 0);
    painter->setFillStyle(Qt::green);

    painter->translate(0, 20);
    painter->fill(p, 234);
    painter->stroke(p, 234);

    painter->translate(200, 0);
    painter->fill(p2, 234);
    painter->stroke(p2, 234);
}

void CanvasPainterLancelotCppTests::testStraightLinesInPathUncachedPath()
{
    painter->translate(0, 10);
    painter->fillText("Default (AA = 1)", 10, 40);
    painter->fillText("Disabled", 10, 80);
    painter->fillText("AA = 2.5", 10, 120);
    painter->fillText("Disabled", 10, 160);
    painter->fillText("AA = 5.0", 10, 200);
    painter->fillText("Disabled", 10, 240);
    painter->fillText("AA = 8.0", 10, 280);
    painter->translate(100, -50);

    painter->setLineWidth(10);

    QCanvasPath p;
    p.moveTo(20, 50);
    p.lineTo(380, 50);

    painter->translate(0, 40);
    painter->stroke(p, -1);

    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, false);
    painter->translate(0, 40);
    painter->stroke(p, -1);

    painter->setAntialias(2.5f);
    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, true);
    painter->translate(0, 40);
    painter->stroke(p, -1);

    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, false);
    painter->translate(0, 40);
    painter->stroke(p, -1);

    painter->setAntialias(5.0f);
    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, true);
    painter->translate(0, 40);
    painter->stroke(p, -1);

    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, false);
    painter->translate(0, 40);
    painter->stroke(p, -1);

    painter->setAntialias(8.0f);
    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, true);
    painter->translate(0, 40);
    painter->stroke(p, -1);
}

void CanvasPainterLancelotCppTests::testStraightLinesInPath()
{
    painter->translate(0, 10);
    painter->fillText("Default (AA = 1)", 10, 40);
    painter->fillText("Disabled", 10, 80);
    painter->fillText("AA = 2.5", 10, 120);
    painter->fillText("Disabled", 10, 160);
    painter->fillText("AA = 5.0", 10, 200);
    painter->fillText("Disabled", 10, 240);
    painter->fillText("AA = 8.0", 10, 280);
    painter->translate(100, -50);

    painter->setLineWidth(10);

    QCanvasPath p;
    p.moveTo(20, 50);
    p.lineTo(380, 50);

    painter->translate(0, 40);
    painter->stroke(p, 123);

    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, false);
    painter->translate(0, 40);
    painter->stroke(p, 123);

    painter->setAntialias(2.5f);
    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, true);
    painter->translate(0, 40);
    painter->stroke(p, 123);

    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, false);
    painter->translate(0, 40);
    painter->stroke(p, 123);

    painter->setAntialias(5.0f);
    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, true);
    painter->translate(0, 40);
    painter->stroke(p, 123);

    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, false);
    painter->translate(0, 40);
    painter->stroke(p, 123);

    painter->setAntialias(8.0f);
    painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, true);
    painter->translate(0, 40);
    painter->stroke(p, 123);
}

void CanvasPainterLancelotCppTests::testPathCaching()
{
    painter->setLineWidth(10);

    // shorter line
    QCanvasPath p;
    p.moveTo(20, 50);
    p.lineTo(380, 50);

    // longer line
    QCanvasPath p2;
    p2.moveTo(20, 50);
    p2.lineTo(580, 50);

    // This should render the following, if the engine works as expected, each
    // entry refers to two short lines with one longer line below, all three should
    // look identical when it comes to line width and AA.
    //
    // width 10 default AA
    // width 20 default AA
    // width 10 default AA
    // width 10 AA increased to 8
    // width 10 default AA

    // draw both with the same group
    // width 10 default AA
    painter->translate(0, 40);
    painter->stroke(p, 123);
    painter->translate(400, 0);
    painter->stroke(p, 123);
    painter->translate(-400, 0);

    painter->translate(0, 40);
    painter->stroke(p2, 123);

    // repeat with a different line width
    // width 20 default AA
    painter->setLineWidth(20);
    painter->translate(0, 40);
    painter->stroke(p, 123);
    painter->translate(400, 0);
    painter->stroke(p, 123);
    painter->translate(-400, 0);

    painter->translate(0, 40);
    painter->stroke(p2, 123);

    // without caching
    // width 10 default AA
    painter->setLineWidth(10);
    painter->translate(0, 40);
    painter->stroke(p, -1);
    painter->translate(400, 0);
    painter->stroke(p, -1);
    painter->translate(-400, 0);

    painter->translate(0, 40);
    painter->stroke(p2, -1);

    // change AA
    // width 10 AA increased to 8
    painter->setAntialias(8);
    painter->translate(0, 40);
    painter->stroke(p, 123);
    painter->translate(400, 0);
    painter->stroke(p, 123);
    painter->translate(-400, 0);

    painter->translate(0, 40);
    painter->stroke(p2, 123);

    // change group for both
    // width 10 default AA
    painter->setAntialias(1);
    painter->translate(0, 40);
    painter->stroke(p, 124);
    painter->translate(400, 0);
    painter->stroke(p, 124);
    painter->translate(-400, 0);

    painter->translate(0, 40);
    painter->stroke(p2, 125);
}

void CanvasPainterLancelotCppTests::testPathFill()
{
    QCanvasPainter::ImageFlags flags = QCanvasPainter::ImageFlag::Repeat;
    QCanvasImage rgbImage = painter->addImage(QImage(":/images/dome_rgb32.png"), flags);
    QCanvasImage argbImage = painter->addImage(QImage(":/images/dome_argb32.png"), flags);

    painter->setStrokeStyle("#afff0000");
    painter->setLineWidth(4);

    QCanvasPath p;
    p.ellipse(10 + 100, 10 + 100, 100, 100);

    painter->setFillStyle(QCanvasImagePattern(rgbImage));
    painter->fill(p);
    painter->stroke(p);

    painter->setFillStyle(QCanvasImagePattern(argbImage));
    painter->save();
    painter->translate(210, 0);
    painter->fill(p);
    painter->stroke(p);
    painter->restore();

    painter->setFillStyle(QCanvasImagePattern(rgbImage));
    painter->save();
    painter->translate(0, 210);
    painter->fill(p);
    painter->restore();

    painter->setFillStyle(QCanvasImagePattern(argbImage));
    painter->save();
    painter->translate(210, 210);
    painter->fill(p);
    painter->restore();

    QCanvasImagePattern rgbpatternOffs(rgbImage);
    rgbpatternOffs.setStartPosition(-30, -30);
    painter->setFillStyle(rgbpatternOffs);
    painter->save();
    painter->translate(420, 0);
    painter->fill(p);
    painter->stroke(p);
    painter->restore();

    QCanvasImagePattern argbpatternOffs(argbImage);
    argbpatternOffs.setStartPosition(-30, -30);
    painter->setFillStyle(argbpatternOffs);
    painter->save();
    painter->translate(630, 0);
    painter->fill(p);
    painter->stroke(p);
    painter->restore();

    painter->setFillStyle(rgbpatternOffs);
    painter->save();
    painter->translate(420, 210);
    painter->fill(p);
    painter->restore();

    painter->setFillStyle(argbpatternOffs);
    painter->save();
    painter->translate(630, 210);
    painter->fill(p);
    painter->restore();

    painter->setFillStyle(Qt::black);
    painter->fillText("No offset  RGB/ARGB", 150, 450);
    painter->fillText("-30 offset RGB/ARGB", 550, 450);
    painter->fillText("10 offset, 45 degrees rotation", 200, 550);

    rgbpatternOffs.setRotation(qDegreesToRadians(45));
    painter->setFillStyle(rgbpatternOffs);
    painter->save();
    painter->translate(0, 550);
    painter->fill(p);
    painter->stroke(p);
    painter->restore();

    argbpatternOffs.setRotation(qDegreesToRadians(45));
    painter->setFillStyle(argbpatternOffs);
    painter->save();
    painter->translate(210, 550);
    painter->fill(p);
    painter->stroke(p);
    painter->restore();
}

void CanvasPainterLancelotCppTests::testCurveImpl(int pathGroup)
{
    QCanvasPath star;
    star.moveTo(50, 0);
    star.lineTo(30, 90);
    star.lineTo(100, 60);
    star.lineTo(0, 20);
    star.lineTo(80, 100);
    star.closePath();

    QCanvasPath rectncircle;
    rectncircle.rect(0, 0, 75, 75);
    rectncircle.circle(50, 50, 25);

    QCanvasPath curve;
    curve.moveTo(100, 0);
    curve.bezierCurveTo(100, 100, 50, 50, 0, 100);
    curve.closePath();

    QCanvasPath curve2;
    curve2.moveTo(200, 0);
    curve2.quadraticCurveTo(200, 200, 250, 250);
    curve2.closePath();

    QCanvasPath arc;
    arc.moveTo(300, 200);
    arc.arcTo(600, 400, 200, 300, 40);
    arc.closePath();

    painter->setStrokeStyle(Qt::black);
    painter->setLineWidth(10);
    painter->setFillStyle("#7f7fff");

    int pg1 = pathGroup;
    int pg2 = pathGroup != -1 ? pathGroup + 1 : -1;
    int pg3 = pathGroup != -1 ? pathGroup + 2 : -1;
    int pg4 = pathGroup != -1 ? pathGroup + 3 : -1;

    auto f = [&] {
        painter->fill(star, pg1);
        painter->stroke(star, pg1);
        painter->translate(100, 0);
        painter->fill(rectncircle, pg2);
        painter->stroke(rectncircle, pg2);
        painter->translate(100, 0);
        painter->fill(curve, pg3);
        painter->stroke(curve, pg3);
        painter->translate(-100, 0);
        painter->fill(curve2, pg4);
        painter->stroke(curve2, pg4);
    };

    f();

    auto g = [&] {
        painter->save();
        painter->translate(50, 60);
        painter->rotate(qDegreesToRadians(10));
        painter->scale(1.0f, 0.7f);
        f();
        painter->restore();
    };

    painter->setLineWidth(0);
    painter->setFillStyle("#ff7f7f");
    painter->translate(0, 300);
    f();
    painter->translate(200, 0);
    QCanvasLinearGradient g1(0, 0, 100, 100);
    g1.setStartColor("#428321");
    g1.setEndColor("#d4d723");
    painter->setFillStyle(g1);
    g();

    painter->resetTransform();
    QCanvasGridPattern gp(10, 10, 40, 40);
    gp.setLineColor("#808080");
    gp.setBackgroundColor("#202020");
    painter->setFillStyle(gp);
    painter->translate(0, 400);
    painter->setLineWidth(4);
    painter->fill(arc, pg1);
    painter->stroke(arc, pg1);
}

void CanvasPainterLancelotCppTests::testCurve()
{
    testCurveImpl(-1);
}

void CanvasPainterLancelotCppTests::testCurveWithPathCaching()
{
    testCurveImpl(99);
}

void CanvasPainterLancelotCppTests::testSomeText()
{

    painter->setFillStyle(Qt::black);
    painter->fillText("Text that is drawn outside the bounds...", -5, 5);

    painter->translate(20, 20);

    auto drawText = [&] {
        painter->save();
        QFont f;
        f.setPointSize(10);
        painter->setFont(f);
        painter->fillText("10 pt, normal", 0, 20);
        f.setPointSize(12);
        painter->setFont(f);
        painter->fillText("12 pt, normal", 0, 40);
        f.setPointSize(10);
        f.setBold(true);
        painter->setFont(f);
        painter->fillText("10 pt, bold", 0, 60);
        f.setBold(false);
        f.setItalic(true);
        painter->setFont(f);
        painter->fillText("10 pt, italic", 0, 80);

        painter->translate(0, 100);
        painter->setFillStyle("#7fff0000");
        f.setPointSize(10);
        f.setItalic(false);
        painter->setFont(f);
        painter->fillText("alpha, 10 pt, normal", 0, 20);
        f.setPointSize(12);
        painter->setFont(f);
        painter->fillText("alpha, 12 pt, normal", 0, 40);
        f.setBold(true);
        painter->setFont(f);
        painter->fillText("alpha, 12 pt, bold", 0, 60);
        f.setItalic(true);
        painter->setFont(f);
        painter->fillText("alpha, 12 pt, bold, italic", 0, 80);

        painter->translate(0, 100);
        painter->setFillStyle(Qt::black);
        painter->save();
        painter->scale(0.9f);
        f = QFont();
        f.setPointSize(10);
        painter->setFont(f);
        painter->fillText("scaled, 10pt, normal", 0, 20);
        f.setPointSize(12);
        painter->setFont(f);
        painter->fillText("scaled, 12pt, normal", 0, 40);
        f.setBold(true);
        painter->setFont(f);
        painter->fillText("scaled, 12 pt, bold", 0, 60);
        f.setItalic(true);
        painter->setFont(f);
        painter->fillText("scaled, 12 pt, bold, italic", 0, 80);
        painter->restore();

        painter->translate(200, 200);
        painter->setFillStyle(Qt::black);
        painter->save();
        painter->scale(-1, -1);
        f = QFont();
        f.setPointSize(10);
        painter->setFont(f);
        painter->fillText("flipped, 10pt, normal", 0, 20);
        f.setPointSize(12);
        painter->setFont(f);
        painter->fillText("flipped, 12pt, normal", 0, 40);
        f.setBold(true);
        painter->setFont(f);
        painter->fillText("flipped, 12 pt, bold", 0, 60);
        f.setItalic(true);
        painter->setFont(f);
        painter->fillText("flipped, 12 pt, bold, italic", 0, 80);
        painter->restore();

        painter->translate(-200, 20);
        painter->setFillStyle(Qt::black);
        painter->save();
        painter->translate(200, 90);
        painter->rotate(qDegreesToRadians(185));
        f = QFont();
        f.setPointSize(10);
        painter->setFont(f);
        painter->fillText("rotated, 10pt, normal", 0, 20);
        f.setPointSize(12);
        painter->setFont(f);
        painter->fillText("rotated, 12pt, normal", 0, 40);
        f.setBold(true);
        painter->setFont(f);
        painter->fillText("rotated, 12 pt, bold", 0, 60);
        f.setItalic(true);
        painter->setFont(f);
        painter->fillText("rotated, 12 pt, bold, italic", 0, 80);
        painter->restore();

        painter->translate(0, 100);
        QCanvasLinearGradient grad(0, 0, 200, 0);
        grad.setColorAt(0, Qt::red);
        grad.setColorAt(0.5f, "#00ff00");
        grad.setColorAt(1, Qt::blue);
        painter->setFillStyle(grad);
        f = QFont();
        f.setPointSize(10);
        painter->setFont(f);
        painter->fillText("gradient, 10 pt, normal", 0, 0);
        f.setPointSize(12);
        painter->setFont(f);
        painter->fillText("gradient, 12pt, normal", 0, 20);
        f.setBold(true);
        painter->setFont(f);
        painter->fillText("gradient, 12 pt, bold", 0, 40);
        f.setItalic(true);
        painter->setFont(f);
        painter->fillText("gradient, 12 pt, bold, italic", 0, 60);

        painter->restore();
    };

    drawText();

    painter->translate(250, 0);
    painter->fillText("clipped to rectangle", 25, 640);
    painter->save();
    painter->setStrokeStyle("#3f000000");
    painter->strokeRect(20, 0, 100, 620);
    painter->setClipRect(20, 0, 100, 620);
    painter->setStrokeStyle(Qt::black);
    drawText();
    painter->restore();

    painter->setFillStyle(Qt::black);
    painter->translate(150, 0);

    painter->translate(0, 75);
    painter->save();
    QFont font;
    font.setPointSize(16);
    painter->setFont(font);
    painter->fillText("e😃m😇o😍j😜i😸!✈️", 0, 40);
    painter->restore();

    painter->translate(0, 75);
    painter->save();
    font.setPointSize(12);
    font.setUnderline(true);
    painter->setFont(font);
    painter->fillText("Underlined text drawing", 0, 20);
    painter->restore();

    painter->translate(0, 35);
    painter->save();
    font.setPointSize(12);
    font.setStrikeOut(true);
    painter->setFont(font);
    painter->fillText("Struck out text drawing", 0, 20);
    painter->restore();

    painter->translate(0, 35);
    painter->save();
    font.setPointSize(12);
    font.setOverline(true);
    painter->setFont(font);
    painter->fillText("Overlined text drawing", 0, 20);
    painter->restore();

    painter->translate(0, 35);
    painter->save();
    font.setPointSize(12);
    font.setUnderline(true);
    font.setOverline(true);
    font.setStrikeOut(true);
    painter->setFont(font);
    painter->fillText("Under+Over+Strike text drawing", 0, 20);
    painter->restore();
}

void CanvasPainterLancelotCppTests::testCanvasPathCommandsWithAndWithoutPathGroup()
{
    QCanvasPath path;

    path.moveTo(10.f, 10.f);
    path.lineTo(100.f, 10.f);
    path.lineTo(100.f, 80.f);
    path.lineTo(10.f, 80.f);
    path.closePath();

    path.moveTo(QPointF(120.f, 10.f));
    path.lineTo(QPointF(200.f, 10.f));
    path.lineTo(QPointF(200.f, 80.f));
    path.closePath();

    path.moveTo(10.f, 120.f);
    path.bezierCurveTo(10.f, 90.f, 80.f, 90.f, 80.f, 120.f);

    path.moveTo(QPointF(100.f, 120.f));
    path.bezierCurveTo(QPointF(100.f, 90.f), QPointF(180.f, 90.f), QPointF(180.f, 120.f));

    path.moveTo(10.f, 160.f);
    path.quadraticCurveTo(55.f, 130.f, 100.f, 160.f);

    path.moveTo(QPointF(120.f, 160.f));
    path.quadraticCurveTo(QPointF(165.f, 130.f), QPointF(210.f, 160.f));

    path.moveTo(10.f, 200.f);
    path.arcTo(10.f, 250.f, 70.f, 250.f, 35.f);

    path.moveTo(QPointF(120.f, 200.f));
    path.arcTo(QPointF(120.f, 250.f), QPointF(180.f, 250.f), 35.f);

    path.moveTo(260.f, 225.f);
    path.arc(230.f, 225.f, 30.f, 0.f, float(M_PI));
    path.arc(330.f, 225.f, 30.f, 0.f, float(M_PI),
             QCanvasPainter::PathWinding::CounterClockWise,
             QCanvasPainter::PathConnection::NotConnected);
    path.arc(QPointF(430.f, 225.f), 30.f, 0.f, 2.f * float(M_PI),
             QCanvasPainter::PathWinding::ClockWise,
             QCanvasPainter::PathConnection::NotConnected);

    path.rect(10.f, 280.f, 80.f, 50.f);
    path.rect(QRectF(110.f, 280.f, 80.f, 50.f));
    path.roundRect(210.f, 280.f, 80.f, 50.f, 10.f);
    path.roundRect(QRectF(310.f, 280.f, 80.f, 50.f), 10.f);
    path.roundRect(10.f, 350.f, 80.f, 50.f, 5.f, 10.f, 15.f, 20.f);
    path.roundRect(QRectF(110.f, 350.f, 80.f, 50.f), 5.f, 10.f, 15.f, 20.f);
    path.ellipse(260.f, 375.f, 40.f, 22.f);
    path.ellipse(QRectF(310.f, 353.f, 80.f, 44.f));
    path.circle(460.f, 375.f, 22.f);
    path.circle(QPointF(530.f, 375.f), 22.f);
    path.setPathWinding(QCanvasPainter::PathWinding::CounterClockWise);
    path.circle(80.f, 460.f, 50.f);
    path.beginHoleSubPath(); // ClockWise
    path.circle(80.f, 460.f, 30.f);
    path.beginSolidSubPath();  // CounterClockWise
    path.circle(80.f, 460.f, 12.f);

    painter->setFillStyle(Qt::green);
    painter->fill(path);
    painter->setStrokeStyle(Qt::red);
    painter->setLineWidth(4);
    painter->stroke(path);

    int group = 5;
    painter->scale(0.5f);

    painter->translate(0, 1050);
    painter->fill(path, group);
    painter->stroke(path, group);

    group = 6;
    painter->translate(1000, -1050);
    painter->fill(path, group);
    painter->stroke(path, group);

    painter->translate(0, 1050);
    painter->fill(path, group);
    painter->stroke(path, group);
}

// ----------- tests adapted from Gallery example start below

void CanvasPainterLancelotCppTests::testRects()
{
    float topMargin = height() * 0.02f;
    float anim = 0.1f;

    {
        int rects = 4;
        float margin = width() * 0.02f;
        float w = width() / (rects+1) - margin;
        float posX = w*0.5 + margin;
        float posY = topMargin;

        QRectF rect1(posX,posY,w,w);
        painter->setFillStyle(QCanvasLinearGradient(0, rect1.y(), 0, rect1.y()+rect1.height()));
        painter->fillRect(rect1);
        posX += w + margin;

        QCanvasLinearGradient g1(0, 0, 0, 0);
        g1.setStartColor(QColor(0,255,0,255));
        g1.setEndColor(QColor(255,0,0,255));
        g1.setStartPosition(posX + (w*0.4f * anim), 0);
        g1.setEndPosition(posX + w - (w*0.4f * anim), 0);
        painter->setFillStyle(g1);
        painter->fillRect(posX,posY,w,w);
        posX += w + margin;

        QRectF rect2(posX,posY,w,w);
        QCanvasLinearGradient g2(rect2.x(), rect2.y(), rect2.x()+rect2.width(), rect2.y()+rect2.height());
        g2.setStartColor(QColor(anim*255, 255, 255));
        g2.setEndColor(QColor(255, anim*255, 255, 255-anim*255));
        painter->setFillStyle(g2);
        painter->fillRect(rect2);
        posX += w + margin;

        QRectF rect3(posX,posY,w,w);
        QCanvasLinearGradient g3(rect3.x(), rect3.y(), rect3.x(), rect3.y()+rect3.height());
        g3.setColorAt(0.0f, QColorConstants::Black);
        g3.setColorAt(0.2f, QColorConstants::Red);
        g3.setColorAt(0.4f, QColorConstants::Green);
        g3.setColorAt(0.6f, QColorConstants::Blue);
        g3.setColorAt(0.8f, QColorConstants::White);
        g3.setColorAt(1.0f, QColorConstants::Transparent);
        painter->setFillStyle(g3);
        painter->fillRect(rect3);
    }

    {
        QCanvasRadialGradient radGrad;
        radGrad.setStops({{ 0.0f, QColor(255, 0, 0, 255) },
                          { 0.1f, QColor(255, 255, 0, 255) },
                          { 0.5f, QColor(0, 255, 0, 255) },
                          { 1.0f, QColor(0, 0, 0, 0) }
                         });

        int rects = 4;
        float margin = width()*0.02f;
        float w = width() / (rects+1) - margin;
        float posX = w*0.5 + margin;
        float posY = topMargin + (w+margin);

        QRectF rect1(posX,posY,w,w);
        painter->setFillStyle(QCanvasRadialGradient(rect1.x(), rect1.y(), rect1.width()));
        painter->fillRect(rect1);
        posX += w + margin;

        QRectF rect2(posX,posY,w,w);
        QCanvasRadialGradient g1;
        g1.setStartColor(QColor(0,255,0,255));
        g1.setEndColor(QColor(255,0,0,255));
        g1.setCenterPosition(rect2.x() + rect2.width()/2, rect2.y() + rect2.height()/2);
        g1.setOuterRadius(0.5*w);
        g1.setInnerRadius(0.4*w*anim);
        painter->setFillStyle(g1);
        painter->fillRect(rect2);
        posX += w + margin;

        QRectF rect3(posX,posY,w,w);
        QCanvasRadialGradient g3(rect3.x() + rect3.width()/2, rect3.y() + rect3.height()/2, w/2, w/4);
        g3.setStartColor(QColor(anim*255, 255, 255));
        g3.setEndColor(QColor(255, anim*255, 255, 255-anim*255));
        painter->setFillStyle(g3);
        painter->fillRect(rect3);
        posX += w + margin;

        QRectF rect4(posX,posY,w,w);
        radGrad.setCenterPosition(rect4.x() + rect4.width()/2, rect4.y() + rect4.height()/2);
        radGrad.setOuterRadius(w/2);
        radGrad.setInnerRadius(w/16);
        auto stops = radGrad.stops();
        stops[2].position = 0.2 + 0.6 * anim;
        stops[0].color = QColor(255, anim*255, 0, 255-anim*255);
        radGrad.setStops(stops);
        painter->setFillStyle(radGrad);
        painter->fillRect(rect4);
    }

    {
        int rects = 4;
        float margin = width()*0.02f;
        float w = width() / (rects+1) - margin;
        float posX = w*0.5 + margin;
        float posY = topMargin + 2*(w+margin);

        QRectF rect1(posX,posY,w,w);
        painter->setFillStyle(QCanvasBoxGradient(rect1, w/2));
        painter->fillRect(rect1);
        posX += w + margin;

        QRectF rect2(posX,posY,w,w);
        QCanvasBoxGradient g1;
        g1.setStartColor(QColor(0,255,0,255));
        g1.setEndColor(QColor(255,0,0,255));
        g1.setRect(rect2);
        g1.setRadius(w/4);
        g1.setFeather(w/2 - anim*w/3);
        painter->setFillStyle(g1);
        painter->fillRect(rect2);
        posX += w + margin;

        QRectF rect3(posX,posY,w,w);
        QCanvasBoxGradient g3(rect3, w/4, w/4);
        g3.setStartColor(QColor(anim*255, 255, 255));
        g3.setEndColor(QColor(255, anim*255, 255, 255-anim*255));
        painter->setFillStyle(g3);
        painter->fillRect(rect3);
        posX += w + margin;

        QRectF rect4(posX,posY,w,w);
        QCanvasBoxGradient g4(rect4, w/4, w/3);
        g4.setColorAt(0.0f, QColor(255, 255, 255, 255));
        g4.setColorAt(0.2f, QColor(0, 0, 0, 255));
        g4.setColorAt(0.4f, QColor(255, anim*255, 0, 255));
        g4.setColorAt(0.6f, QColor(0, 0, 0, 255));
        g4.setColorAt(0.8f, QColor(255, 255, 255, 255));
        g4.setColorAt(1.0f, QColor(0, 0, 0, 0));
        painter->setFillStyle(g4);
        painter->fillRect(rect4);
    }

    {
        int rects = 4;
        float margin = width()*0.02f;
        float w = width() / (rects+1) - margin;
        float posX = w*0.5 + margin;
        float posY = topMargin + 3*(w+margin);

        QRectF rect1(posX,posY,w,w);
        QCanvasConicalGradient g1;
        g1.setCenterPosition(rect1.center());
        painter->setFillStyle(g1);
        painter->fillRect(rect1);
        posX += w + margin;

        QRectF rect2(posX,posY,w,w);
        QCanvasConicalGradient g2;
        g2.setCenterPosition(rect2.center());
        g2.setStartColor(QColor(0,255,0,255));
        g2.setEndColor(QColor(255,0,0,255));
        painter->setFillStyle(g2);
        painter->fillRect(rect2);
        posX += w + margin;

        QRectF rect3(posX,posY,w,w);
        QCanvasConicalGradient g3;
        g3.setCenterPosition(rect3.center());
        g3.setAngle(-0.5f * float(M_PI));
        g3.setStartColor(QColor(anim*255, 255, 255));
        g3.setEndColor(QColor(255, anim*255, 255, 255-anim*255));
        painter->setFillStyle(g3);
        painter->fillRect(rect3);
        posX += w + margin;

        QRectF rect4(posX,posY,w,w);
        QCanvasConicalGradient g4;
        g4.setCenterPosition(rect4.x() + anim * rect4.width(), rect4.y() + rect4.height()/2);
        g4.setAngle(anim);
        g4.setStartColor(QColor(255, 255, 0, 255));
        g4.setColorAt(0.25, QColor(0, 255, 0, 255));
        g4.setColorAt(0.5, QColor(0, 0, 255, 255));
        g4.setColorAt(0.75, QColor(255, 0, 255, 255));
        g4.setEndColor(QColor(255, 255, 0, 255));
        painter->setFillStyle(g4);
        painter->fillRect(rect4);
    }
}

void CanvasPainterLancelotCppTests::testRects2()
{
    float topMargin = height() * 0.02f;
    float anim = 0.1f;

    QCanvasPainter::ImageFlags flags = QCanvasPainter::ImageFlag::Repeat | QCanvasPainter::ImageFlag::GenerateMipmaps;
    QCanvasImage patternImage = painter->addImage(QImage(":/images/pattern1.png"), flags);
    QCanvasImage patternImage2 = painter->addImage(QImage(":/images/pattern2.png"), flags);
    QCanvasImage patternImage3 = painter->addImage(QImage(":/images/pattern3.png"), flags);

    {
        int rects = 3;
        float margin = width()*0.02f;
        float w = width() / (rects+2) - margin;
        float posX = w + margin;
        float posY = topMargin;

        QRectF rect1(posX,posY,w,w);
        QCanvasImagePattern g4(patternImage2, rect1);
        QColor aColor = QColor::fromRgbF(1.0f,
                                        0.5f + 0.5f * sin(anim),
                                        0.5f + 0.5f * sin(anim * 2 + M_PI),
                                        1.0f);
        g4.setTintColor(aColor);
        painter->setFillStyle(g4);
        painter->fillRect(rect1);
        posX += w + margin;

        QRectF rect2(posX,posY,w,w);
        QCanvasImagePattern g1;
        g1.setImage(patternImage);
        g1.setStartPosition(rect2.x()+w/2, rect2.y()+w/2);
        g1.setImageSize(w/5, w/3);
        g1.setRotation(anim);
        QColor opacityTint(255, 255, 255, 127 + anim * 128);
        g1.setTintColor(opacityTint);
        painter->setFillStyle(g1);
        painter->fillRect(rect2);
        posX += w + margin;

        QRectF rect3(posX,posY,w,w);
        QCanvasImagePattern g3(patternImage);
        g3.setStartPosition(rect3.x(), rect3.y());
        g3.setImageSize(w*0.1 + anim * w*0.2f, w*0.15f + anim * w*0.3f);
        painter->setFillStyle(g3);
        painter->fillRect(rect3);
    }

    {
        int rects = 3;
        float margin = width()*0.02f;
        float border = margin + margin * anim;
        float w = width() / (rects+2) - margin;
        float w2 = w - border;
        float posX = w + margin + border/2;
        float posY = topMargin + 2*(w+margin) + border/2;

        QRectF rect1(posX,posY,w2,w2);
        painter->setLineWidth(border);
        painter->setFillStyle(QCanvasImagePattern(patternImage3));
        QCanvasLinearGradient g1(posX, posY, posX+w2, posY+w2);
        g1.setStartColor("#ffffff");
        g1.setEndColor("#000000");
        painter->setStrokeStyle(g1);
        painter->beginPath();
        painter->roundRect(rect1, border);
        painter->fill();
        painter->stroke();
        posX += w + margin;

        QRectF rect2(posX,posY,w2,w2);
        g1.setStartPosition(posX, posY);
        g1.setEndPosition(posX+w2, posY+w2);
        painter->setFillStyle(g1);
        painter->setStrokeStyle(QCanvasImagePattern(patternImage3));
        painter->beginPath();
        painter->roundRect(rect2, border);
        painter->fill();
        painter->stroke();
        posX += w + margin;

        QRectF rect3(posX,posY,w2,w2);
        QCanvasRadialGradient g2(posX+w2/4, posY+w2/4, w2);
        g2.setStartColor("#900000ff");
        g2.setEndColor("#90ff0000");
        painter->setStrokeStyle(g2);
        QCanvasImagePattern p1 = QCanvasImagePattern(patternImage3);
        p1.setImageSize(16, 16);
        p1.setStartPosition(anim*40, 0);
        painter->setFillStyle(p1);
        painter->beginPath();
        painter->roundRect(rect3, border);
        painter->fill();
        painter->stroke();
    }
}

void CanvasPainterLancelotCppTests::testPaths()
{
    float topMargin = height() * 0.02f;
    float anim = 0.1f;

    {
        int rects = 3;
        float margin = width()*0.1f;
        float w = width() / (rects) - margin;
        float lineWidth = width()*0.02f;
        float posX = margin/2;
        float posY = topMargin;

        painter->setFillStyle(0xFF948979);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->setLineWidth(lineWidth);
        painter->setLineCap(QCanvasPainter::LineCap::Round);
        painter->setLineJoin(QCanvasPainter::LineJoin::Round);

        painter->beginPath();
        painter->rect(posX, posY, w, w);
        posX += w + margin;
        painter->roundRect(posX, posY, w, w, w/2*anim);
        posX += w + margin;
        painter->roundRect(posX, posY, w, w, w/2*anim, 0, w/2*(cos(anim)*0.5+0.5), w/4*(sin(anim*10)*0.5+0.5));

        posY += w+margin;
        posX = margin/2;
        painter->ellipse(posX+w/2, posY+w/2, w/3+w/4*anim, w/3+w/4*(1-anim));
        posX += w + margin;
        painter->moveTo(posX+w/2, posY+w/2);
        painter->arc(posX+w/2, posY+w/2, w/2, 0, 0.5f + anim*5);
        painter->closePath();
        posX += w + margin;
        QRectF r(posX, posY, w, w);
        painter->ellipse(r);
        float a = w*0.1f*anim;
        float a2 = w*0.1f*(1-anim);
        r.adjust(w*0.1f+a, w*0.1f+a2, -w*0.1f-a, -w*0.1f-a2);
        painter->beginHoleSubPath();
        painter->ellipse(r);
        r.adjust(w*0.2f, w*0.2f, -w*0.2f, -w*0.2f);
        painter->beginSolidSubPath();
        painter->rect(r);

        painter->stroke();
        painter->fill();

        posY += w+margin;
        posX = margin/2;
        painter->beginPath();
        painter->moveTo(posX, posY);
        painter->bezierCurveTo(posX+w*0.2f*anim, posY+w*0.8f*anim, posX+w*0.8f*anim, posY+w*0.2f*anim, posX+w, posY+w);
        posX += w + margin;
        painter->moveTo(posX, posY);
        painter->quadraticCurveTo(posX+w*0.8f*anim, posY+w*0.2f*anim, posX+w, posY+w);
        posX += w + margin;
        painter->moveTo(posX, posY);
        painter->arcTo(posX, posY+w, posX+w, posY+w, w*anim);

        painter->stroke();
    }
}

void CanvasPainterLancelotCppTests::testPaths2()
{
    float topMargin = height() * 0.02f;
    float anim = 0.5f;

    {
        int rects = 3;
        float margin = width()*0.1f;
        float w = width() / (rects) - margin;
        float posX = margin/2;
        float posY = topMargin;
        float lineWidth = width()*0.02f;

        painter->setFillStyle(0x00941979);
        painter->setStrokeStyle(0x0031D0B8);
        painter->setLineWidth(lineWidth);
        painter->setLineCap(QCanvasPainter::LineCap::Round);
        painter->setLineJoin(QCanvasPainter::LineJoin::Round);

        painter->setLineCap(QCanvasPainter::LineCap::Round);
        painter->setLineJoin(QCanvasPainter::LineJoin::Round);
        painter->beginPath();
        painter->moveTo(posX, posY);
        painter->lineTo(posX, posY+w);
        painter->lineTo(posX+w*anim, posY);
        painter->stroke();
        posX += w + margin;
        painter->setLineCap(QCanvasPainter::LineCap::Butt);
        painter->setLineJoin(QCanvasPainter::LineJoin::Miter);
        painter->beginPath();
        painter->moveTo(posX, posY);
        painter->lineTo(posX, posY+w);
        painter->lineTo(posX+w*anim, posY);
        painter->stroke();
        posX += w + margin;
        painter->setLineCap(QCanvasPainter::LineCap::Square);
        painter->setLineJoin(QCanvasPainter::LineJoin::Bevel);
        painter->beginPath();
        painter->moveTo(posX, posY);
        painter->lineTo(posX, posY+w);
        painter->lineTo(posX+w*anim, posY);
        painter->stroke();
    }
}

void CanvasPainterLancelotCppTests::testTransforms()
{
    float topMargin = height() * 0.02f;
    float anim = 0.5f;

    {
        int rects = 3;
        float margin = width()*0.1f;
        float w = width() / (rects) - margin;
        float posX = margin/2;
        float lineWidth = width()*0.01f;
        float posY = topMargin;

        painter->setFillStyle(0xFFFFFFFF);
        painter->setStrokeStyle(0xFF000000);
        painter->setLineWidth(lineWidth);
        painter->fillRect(posX, posY, w, w);
        painter->strokeRect(posX, posY, w, w);
        // Save the painter state
        painter->save();

        // Modify painter state
        posX += w + margin;
        painter->setFillStyle(0xFF900000);
        painter->setStrokeStyle(0xFFE00000);
        painter->setLineWidth(lineWidth*2);
        painter->setLineCap(QCanvasPainter::LineCap::Round);
        painter->setLineJoin(QCanvasPainter::LineJoin::Round);
        painter->translate(posX+w/2, posY+w/2);
        painter->rotate(1.0 - anim);
        painter->scale(0.2f + 0.8f*anim);
        painter->translate(-(posX+w/2), -(posY+w/2));
        painter->fillRect(posX, posY, w, w);
        painter->strokeRect(posX, posY, w, w);

        posX += w + margin;
        // And restore painter state back
        painter->restore();
        painter->fillRect(posX, posY, w, w);
        painter->strokeRect(posX, posY, w, w);

        posY += w+margin;
        posX = margin/2;
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->save();
        painter->translate(w/8 * anim, w/4 * anim);
        painter->strokeRect(posX, posY, w/2, w/2);
        posX += w + margin;
        painter->translate(w/8 * anim, w/4 * anim);
        painter->strokeRect(posX, posY, w/2, w/2);
        posX += w + margin;
        painter->translate(w/8 * anim, w/4 * anim);
        painter->strokeRect(posX, posY, w/2, w/2);
        painter->restore();

        posY += w+margin;
        posX = margin/2;
        painter->save();
        painter->translate(posX+w/2, posY+w/2);
        painter->skew(0.4f * anim);
        painter->translate(-(posX+w/2), -(posY+w/2));
        painter->strokeRect(posX, posY, w, w);
        painter->restore();

        posX += w + margin;
        painter->save();
        painter->translate(posX+w/2, posY+w/2);
        painter->skew(0.0f, 0.4f * anim);
        painter->translate(-(posX+w/2), -(posY+w/2));
        painter->strokeRect(posX, posY, w, w);
        painter->restore();

        posX += w + margin;
        QTransform t;
        t.translate(posX+w/2, posY+w/2);
        t.rotate(anim*20);
        float scale = 0.5f + 0.5f*anim;
        t.scale(scale, scale);
        t.translate(-(posX+w/2), -(posY+w/2));
        painter->setTransform(t);
        painter->strokeRect(posX, posY, w, w);
        painter->setClipRect(posX, posY, w, w);
        // Duplicate transformation
        QTransform t2 = painter->getTransform();
        painter->transform(t2);

        painter->setStrokeStyle(0xFF948979);
        painter->strokeRect(posX, posY, w, w);
        painter->resetClipping();

        posY += w+margin;
        posX = margin/2;
        painter->resetTransform();
        painter->setLineWidth(lineWidth);
        QRectF r(posX, posY, w, w);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(r);
        r.adjust(lineWidth/2, lineWidth/2, -lineWidth/2, -lineWidth/2);
        painter->setClipRect(r);
        painter->beginPath();
        painter->circle(posX + anim*w, posY + w/2, w/2);
        painter->fill();
        painter->resetClipping();

        posX += w + margin;
        painter->setFillStyle(0xFFDFD0B8);
        painter->fillRect(posX, posY, w, w);
        painter->setClipRect(int(posX + w/2), int(posY + w/2), int(w/2)+2, int(w/2)+2);
        painter->clearRect(posX, posY, w, w);
        painter->resetClipping();

        posX += w + margin;
        painter->setFillStyle(0xFFDFD0B8);
        painter->setStrokeStyle(0xFF948979);
        painter->beginPath();
        painter->roundRect(posX, posY, w, w, w / 4);
        painter->fill();
        painter->stroke();
        float cRectSize = (0.6 + 0.4 * sin(anim)) * w;
        QRectF cRect(posX + anim*w, posY + w/2 - cRectSize/2, cRectSize, cRectSize);
        painter->translate(cRect.center());
        painter->rotate(anim);
        painter->translate(-cRect.center());
        painter->clearRect(cRect);
    }
}

void CanvasPainterLancelotCppTests::testTransforms2()
{
    float topMargin = height() * 0.02f;
    float anim = 0.5f;

    {
        int rects = 3;
        float margin = width()*0.1f;
        float w = width() / (rects) - margin;
        float posX = margin/2;
        float lineWidth = width()*0.01f;
        float posY = topMargin;

        painter->setFillStyle(0xFFFFFFFF);
        painter->resetClipping();
        posY += w+margin;
        posX = margin/2;
        painter->resetTransform();
        painter->setLineWidth(lineWidth);
        QRectF r(posX, posY, w, w);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(r);
        r.adjust(lineWidth/2, lineWidth/2, -lineWidth/2, -lineWidth/2);
        painter->setClipRect(r);
        painter->beginPath();
        painter->circle(posX + anim*w, posY + w/2, w/2);
        painter->fill();
        painter->resetClipping();

        posX += w + margin;
        painter->setFillStyle(0xFFDFD0B8);
        painter->fillRect(posX, posY, w, w);
        painter->setClipRect(int(posX + w/2), int(posY + w/2), int(w/2)+2, int(w/2)+2);
        painter->clearRect(posX, posY, w, w);
        painter->resetClipping();

        posX += w + margin;
        painter->setFillStyle(0xFFDFD0B8);
        painter->setStrokeStyle(0xFF948979);
        painter->beginPath();
        painter->roundRect(posX, posY, w, w, w / 4);
        painter->fill();
        painter->stroke();
        float cRectSize = (0.6 + 0.4 * sin(anim)) * w;
        QRectF cRect(posX + anim*w, posY + w/2 - cRectSize/2, cRectSize, cRectSize);
        painter->translate(cRect.center());
        painter->rotate(anim);
        painter->translate(-cRect.center());
        painter->clearRect(cRect);
    }
}

void CanvasPainterLancelotCppTests::testGridPatterns()
{
    float topMargin = height() * 0.02f;
    float anim = 0.2f;

    {
        int rects = 2;
        float margin = width() * 0.05f;
        float w = width() / rects - margin;
        float h = height() * 0.25;
        float posX = margin/2;
        float posY = topMargin;

        QRectF rect1(posX, posY, w, h);
        float cellZoom = 1.0 + 0.95 * sin(0.5 * anim);
        QPointF cp1 = rect1.center();
        // Minor grid
        QCanvasGridPattern gp1(cp1.x(), cp1.y(), w * 0.1 * cellZoom, h * 0.1 * cellZoom);
        gp1.setLineColor("#404040");
        gp1.setBackgroundColor("#202020");
        painter->setFillStyle(gp1);
        painter->fillRect(rect1);
        // Major grid
        QCanvasGridPattern gp2(cp1.x(), cp1.y(), w * cellZoom, h * cellZoom);
        gp2.setLineColor("#d0d0d0");
        gp2.setBackgroundColor(Qt::transparent);
        painter->setFillStyle(gp2);
        painter->fillRect(rect1);

        posX += w + margin;
        QRectF rect2(posX, posY, w, h);
        QPointF cp2 = rect2.center();
        float g1 = w * 0.1f;
        QCanvasGridPattern gp3(cp2.x(), cp2.y(), g1, g1);
        QColor color1(anim * 150, 80, 40);
        gp3.setLineColor(color1);
        gp3.setBackgroundColor("#202020");
        gp3.setLineWidth(g1 * 0.5f);
        gp3.setFeather(g1 * 0.5f);
        gp3.setRotation(anim);
        painter->setFillStyle(gp3);
        painter->fillRect(rect2);

        posY += h + margin;
        posX = margin/2;
        QRectF rect3(posX, posY, w, h);
        QCanvasGridPattern gp4;
        float bar = w * 0.1f;
        gp4.setStartPosition(15 * anim, 0);
        gp4.setCellSize(bar, 0);
        gp4.setLineWidth(bar * 0.5f);
        gp4.setRotation(float(M_PI) / 4);
        QCanvasGridPattern gp5;
        gp5.setCellSize(0, 4);
        gp5.setBackgroundColor("#202020");
        gp5.setLineColor("#404040");
        painter->setStrokeStyle(gp4);
        painter->setFillStyle(gp5);
        painter->setLineWidth(0.5f * bar);
        painter->beginPath();
        painter->roundRect(rect3, bar);
        painter->fill();
        painter->stroke();

        posX += w + margin;
        QRectF rect4(posX, posY, w, h);
        QPointF cp4 = rect4.center();
        QCanvasGridPattern gp6;
        float strokeW = 10 * anim;
        gp6.setLineColor(Qt::transparent);
        gp6.setBackgroundColor(Qt::white);
        gp6.setStartPosition(cp4.x(), cp4.y());
        gp6.setCellSize(rect4.width() / 5, rect4.height() / 5);
        gp6.setLineWidth(strokeW);
        painter->setStrokeStyle(gp6);
        painter->setLineWidth(4);
        painter->beginPath();
        painter->roundRect(rect4, 10);
        painter->stroke();
    }
}

void CanvasPainterLancelotCppTests::testShadows()
{
    float topMargin = height() * 0.02f;
    float anim = 0.3f;

    {
        int rects = 3;
        float margin = width() * 0.15f;
        float w = width() / rects - margin;
        float h = w;
        float posX = margin/2;
        float posY = topMargin + margin * 0.5;

        QRectF rect1(posX, posY, w, h);
        QCanvasBoxShadow bs1(rect1);
        painter->drawBoxShadow(bs1);
        rect1.translate(w + margin, 0);
        bs1.setRect(rect1);
        bs1.setBlur(0.15 * w);
        painter->drawBoxShadow(bs1);
        rect1.translate(w + margin, 0);
        bs1.setRect(rect1);
        bs1.setBlur(0.3 * w);
        painter->drawBoxShadow(bs1);

        posY += h + margin * 0.5;
        QRectF rect2(posX, posY, w, h);
        QCanvasBoxShadow bs2(rect2);
        bs2.setRadius(h/2);
        painter->drawBoxShadow(bs2);
        rect2.translate(w + margin, 0);
        bs2.setRect(rect2);
        bs2.setBlur(0.15 * w);
        painter->drawBoxShadow(bs2);
        rect2.translate(w + margin, 0);
        bs2.setRect(rect2);
        bs2.setBlur(0.3 * w);
        painter->drawBoxShadow(bs2);

        posY += h + margin * 0.5;
        QRectF rect3(posX, posY, w, h);
        QCanvasBoxShadow bs3(rect3);
        bs3.setBlur(h/8);
        bs3.setColor("#393E46");
        bs3.setTopLeftRadius(h/4 + h/4 * sin(anim));
        bs3.setTopRightRadius(h/4 + h/4 * sin(anim + 0.5 * M_PI));
        bs3.setBottomLeftRadius(h/4 + h/4 * sin(anim + 1.0 * M_PI));
        bs3.setBottomRightRadius(h/4 + h/4 * sin(anim + 1.5 * M_PI));
        painter->drawBoxShadow(bs3);
        rect3.translate(w + margin, 0);
        bs3.setColor("#ffffff");
        bs3.setRect(rect3);
        painter->drawBoxShadow(bs3);
        rect3.translate(w + margin, 0);
        bs3.setColor("#40ffff00");
        bs3.setRect(rect3);
        painter->drawBoxShadow(bs3);
    }
}

void CanvasPainterLancelotCppTests::testShadows2()
{
    float topMargin = height() * 0.02f;
    float anim = 0.3f;

    {
        int rects = 3;
        float margin = width() * 0.15f;
        float w = width() / rects - margin;
        float h = w;
        float posX = margin/2;
        float posY = topMargin + margin * 0.5;

        posY += h + margin * 0.5;
        float blur = w * 0.3;
        float radius = 5 + anim * (h / 2 - 5);
        float shadowOffsetX = blur * 0.3;
        float shadowOffsetY = blur * 0.3;
        QRectF buttonRect(posX, posY, w, h);
        // Double shadows
        QRectF shadow1Rect = buttonRect.translated(-shadowOffsetX, shadowOffsetY);
        QCanvasBoxShadow shadow;
        QColor c("#222831");
        QColor cl = c.lighter(200);
        QColor cd = c.darker(250);
        cl.setAlphaF(0.3f);
        cd.setAlphaF(0.8f);
        shadow.setBlur(blur);
        shadow.setRadius(radius);
        shadow.setRect(shadow1Rect);
        shadow.setColor(cd);
        painter->drawBoxShadow(shadow);
        QRectF shadow2Rect = buttonRect.translated(shadowOffsetX, -shadowOffsetY);
        shadow.setRect(shadow2Rect);
        shadow.setColor(cl);
        painter->drawBoxShadow(shadow);
        // Button on top of shadows
        painter->beginPath();
        painter->roundRect(buttonRect, radius);
        painter->setFillStyle(c);
        painter->fill();

        buttonRect.translate(w + margin, 0);
        shadow1Rect.translate(w + margin, 0);
        shadow2Rect.translate(w + margin, 0);
        radius = h / 8;
        shadow.setRadius(radius);
        shadow.setBlur(h/16 + anim * h/4);
        // Double shadows
        shadow.setRect(shadow1Rect);
        shadow.setColor(cd);
        painter->drawBoxShadow(shadow);
        shadow.setRect(shadow2Rect);
        shadow.setColor(cl);
        painter->drawBoxShadow(shadow);
        // Button on top of shadows
        painter->beginPath();
        painter->roundRect(buttonRect, radius);
        painter->setFillStyle(c);
        painter->fill();

        buttonRect.translate(w + margin, 0);
        shadow1Rect.translate(w + margin, 0);
        shadow2Rect.translate(w + margin, 0);
        radius = h / 8;
        shadow.setRadius(radius);
        shadow.setBlur(h * 0.2);
        // Double shadows
        float t1 = shadowOffsetX * sin(anim);
        shadow1Rect.translate(t1, -t1);
        shadow.setRect(shadow1Rect);
        shadow.setColor(cd);
        painter->drawBoxShadow(shadow);
        float t2 = shadowOffsetX * sin(anim + M_PI);
        shadow2Rect.translate(t2, -t2);
        shadow.setRect(shadow2Rect);
        shadow.setColor(cl);
        painter->drawBoxShadow(shadow);
        // Button on top of shadows
        painter->beginPath();
        painter->roundRect(buttonRect, radius);
        painter->setFillStyle(c);
        painter->fill();
    }
}

void CanvasPainterLancelotCppTests::drawCompositeItem1(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode)
{
    float anim = 0.5f;

    painter->setGlobalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver);
    // First item
    painter->beginPath();
    painter->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter->setFillStyle("#DFD0B8");
    painter->fill();

    painter->setGlobalCompositeOperation(mode);

    // Second item
    painter->beginPath();
    painter->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * anim), w * 0.4f);
    painter->setFillStyle("#948979");
    painter->fill();
}

void CanvasPainterLancelotCppTests::drawCompositeItem2(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode)
{
    float anim = 0.5f;

    painter->setGlobalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver);
    painter->setLineWidth(w * 0.05f);
    // First item
    painter->beginPath();
    painter->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter->setFillStyle("#DFD0B8");
    painter->fill();
    painter->setStrokeStyle("#ffffff");
    painter->stroke();

    painter->setGlobalCompositeOperation(mode);

    // Second item
    painter->beginPath();
    painter->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * anim), w * 0.4f);
    painter->setFillStyle("#948979");
    painter->fill();
    painter->setStrokeStyle("#000000");
    painter->stroke();
}

void CanvasPainterLancelotCppTests::drawCompositeItem3(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode)
{
    float anim = 0.5f;

    painter->setGlobalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver);
    painter->setLineWidth(w * 0.05f);
    painter->setAntialias(10.0f);
    // First item
    painter->beginPath();
    painter->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter->setFillStyle("#DFD0B8");
    painter->fill();
    painter->setStrokeStyle("#ffffff");
    painter->stroke();

    painter->setGlobalCompositeOperation(mode);

    // Second item
    painter->setGlobalAlpha(0.5);
    painter->beginPath();
    painter->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * anim), w * 0.4f);
    painter->setFillStyle("#948979");
    painter->fill();
    painter->setStrokeStyle("#000000");
    painter->stroke();
    painter->setGlobalAlpha(1.0f);
}

void CanvasPainterLancelotCppTests::testCompositeModes()
{
    float topMargin = height() * 0.02f;

    painter->clearRect(0, 0, width(), height());
    painter->setGlobalAlpha(0.7f);

    {
        int rects = 3;
        float margin = width() * 0.1f;
        float w = width() / rects - margin;
        float h = w * 1.2;
        float posX = margin/2;
        float posY = topMargin;

        drawCompositeItem1(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceOver);
        posX += w + margin;
        drawCompositeItem1(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceAtop);
        posX += w + margin;
        drawCompositeItem1(posX, posY, w, h, QCanvasPainter::CompositeOperation::DestinationOut);

        posY += h + margin;
        posX = margin/2;
        drawCompositeItem2(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceOver);
        posX += w + margin;
        drawCompositeItem2(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceAtop);
        posX += w + margin;
        drawCompositeItem2(posX, posY, w, h, QCanvasPainter::CompositeOperation::DestinationOut);

        posY += h + margin;
        posX = margin/2;
        drawCompositeItem3(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceOver);
        posX += w + margin;
        drawCompositeItem3(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceAtop);
        posX += w + margin;
        drawCompositeItem3(posX, posY, w, h, QCanvasPainter::CompositeOperation::DestinationOut);
    }

    painter->setGlobalAlpha(1.0f);
}

void CanvasPainterLancelotCppTests::testImages()
{
    float topMargin = height() * 0.02f;
    float anim = 0.1f;

    QCanvasImage patternImage2 = painter->addImage(QImage(":/images/pattern2.png"),
        QCanvasPainter::ImageFlag::Repeat | QCanvasPainter::ImageFlag::GenerateMipmaps);
    QCanvasImage testImage = painter->addImage(QImage(":/images/qt_development_white.png"));
    QCanvasImage image3Gray = painter->addImage(QImage(":/images/face-smile-bw.png"));

    {
        float margin = height()*0.05f;
        float posY = topMargin;

        QRectF rect1(0, 0, width(), height());
        auto pattern = QCanvasImagePattern(patternImage2);
        pattern.setTintColor(QColor(10, 10, 10));
        painter->setFillStyle(pattern);
        painter->fillRect(rect1);

        // Draw image scaled to view width
        float imageRatio = float(testImage.height()) / testImage.width();
        float imgX = margin;
        float imgW = width() - 2 * margin;
        QRectF imgRect(imgX, posY, imgW, imgW * imageRatio);
        painter->drawImage(testImage, imgRect);

        // Draw just the "Development" part of the image
        posY += imgRect.height() + margin;
        float cropW = 105;
        QRectF sourceArea(cropW, 0, testImage.width() - cropW, testImage.height());
        float cx = width() * 0.5f;
        float r = width() * 0.1f + (anim * width() * 0.2f);
        QRectF targetArea(cx-r, posY, r*2, width()*0.1);
        painter->drawImage(testImage, sourceArea, targetArea);

        // Draw rotating & scaling & tinted image
        posY += targetArea.height() + height() * 0.15;
        QRectF rect(cx-r * 0.5f, posY - r * 0.5f, r, r);
        QPointF c(rect.x()+rect.width() * 0.5f, rect.y()+rect.height() * 0.5f);
        painter->save();
        painter->translate(c);
        painter->rotate(anim);
        painter->translate(-c);
        QColor tintColor(255, 255, 255 - anim * 200);
        image3Gray.setTintColor(tintColor);
        painter->drawImage(image3Gray,rect);
        painter->restore();
    }
}

void CanvasPainterLancelotCppTests::testImages2()
{
    float topMargin = height() * 0.02f;

    QCanvasImage image3Plain = painter->addImage(QImage(":/images/pattern2.png"));
    QCanvasImage image3Nearest = painter->addImage(QImage(":/images/pattern2.png"), QCanvasPainter::ImageFlag::Nearest);
    QCanvasImage image3Mips = painter->addImage(QImage(":/images/pattern2.png"), QCanvasPainter::ImageFlag::GenerateMipmaps);
    QCanvasImage image3NearestMips = painter->addImage(QImage(":/images/pattern2.png"),
        QCanvasPainter::ImageFlag::Nearest | QCanvasPainter::ImageFlag::GenerateMipmaps);

    {
        float posY = topMargin;
        float cx = width() * 0.5f;

        QRectF rect1(0, 0, width(), height());
        painter->setFillStyle(Qt::gray);
        painter->fillRect(rect1);

        // Scaled images with and without mipmapping
        posY = height() * 0.15f;
        float posY2 = posY + (height() * 0.3f);
        bool useNearest = false;
        {
            float posX = width()*0.1f;
            float sizeDiff = width() * 0.025f;
            float centerY = 4 * sizeDiff;
            for (int i = 0; i < 7; ++i) {
                float size = (i + 1) * sizeDiff;
                QRectF rect2(posX, posY+centerY, size, size);
                painter->drawImage(useNearest ? image3Nearest : image3Plain, rect2);
                QRectF rect3(posX, posY2+centerY, size, size);
                painter->drawImage(useNearest ? image3NearestMips : image3Mips, rect3);
                posX += size + 2;
            }
            QFont font;
            float fontSize = width() * 0.04f;
            font.setPixelSize(fontSize);
            painter->setTextAlign(QCanvasPainter::TextAlign::Center);
            painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
            QString scaling = useNearest ? "NEAREST" : "LINEAR";
            QString offString = QString("MIPMAPS: OFF, TEXTURE: %1").arg(scaling);
            QString onString = QString("MIPMAPS: ON, TEXTURE: %1").arg(scaling);
            float text1PosY = posY + 0.5f * centerY;
            float text2PosY = posY2 + 0.5f * centerY;
            painter->setFillStyle(QColorConstants::White);
            painter->setFont(font);
            painter->fillText(offString, cx, text1PosY);
            painter->fillText(onString, cx, text2PosY);
        }
    }
}

void CanvasPainterLancelotCppTests::testImages3()
{
    float topMargin = height() * 0.02f;

    QCanvasImage image3Plain = painter->addImage(QImage(":/images/pattern2.png"));
    QCanvasImage image3Nearest = painter->addImage(QImage(":/images/pattern2.png"), QCanvasPainter::ImageFlag::Nearest);
    QCanvasImage image3Mips = painter->addImage(QImage(":/images/pattern2.png"), QCanvasPainter::ImageFlag::GenerateMipmaps);
    QCanvasImage image3NearestMips = painter->addImage(QImage(":/images/pattern2.png"),
        QCanvasPainter::ImageFlag::Nearest | QCanvasPainter::ImageFlag::GenerateMipmaps);

    {
        float posY = topMargin;
        float cx = width() * 0.5f;

        QRectF rect1(0, 0, width(), height());
        painter->setFillStyle(Qt::gray);
        painter->fillRect(rect1);

        // Scaled images with and without mipmapping
        posY = height() * 0.15f;
        float posY2 = posY + (height() * 0.3f);
        bool useNearest = true;
        {
            float posX = width()*0.1f;
            float sizeDiff = width() * 0.025f;
            float centerY = 4 * sizeDiff;
            for (int i = 0; i < 7; ++i) {
                float size = (i + 1) * sizeDiff;
                QRectF rect2(posX, posY+centerY, size, size);
                painter->drawImage(useNearest ? image3Nearest : image3Plain, rect2);
                QRectF rect3(posX, posY2+centerY, size, size);
                painter->drawImage(useNearest ? image3NearestMips : image3Mips, rect3);
                posX += size + 2;
            }
            QFont font;
            float fontSize = width() * 0.04f;
            font.setPixelSize(fontSize);
            painter->setTextAlign(QCanvasPainter::TextAlign::Center);
            painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
            QString scaling = useNearest ? "NEAREST" : "LINEAR";
            QString offString = QString("MIPMAPS: OFF, TEXTURE: %1").arg(scaling);
            QString onString = QString("MIPMAPS: ON, TEXTURE: %1").arg(scaling);
            float text1PosY = posY + 0.5f * centerY;
            float text2PosY = posY2 + 0.5f * centerY;
            painter->setFillStyle(QColorConstants::White);
            painter->setFont(font);
            painter->fillText(offString, cx, text1PosY);
            painter->fillText(onString, cx, text2PosY);
        }
    }
}

void CanvasPainterLancelotCppTests::testAntialiasing()
{
    float topMargin = height() * 0.02f;
    float anim = 0.1f;

    {
        int rects = 3;
        float margin = std::floor(width() * 0.1f);
        float w = std::floor(width() / (rects) - margin);
        float posX = margin/2;
        float lineWidth = width()*0.01f;
        float posY = std::floor(topMargin);

        painter->setLineCap(QCanvasPainter::LineCap::Round);
        painter->setStrokeStyle(Qt::black);
        painter->setLineWidth(lineWidth);
        painter->setAntialias(0);
        painter->beginPath();
        painter->roundRect(posX, posY, w, w, w/4);
        painter->stroke();
        posX += w + margin;
        painter->setAntialias(1.0f);
        painter->beginPath();
        painter->roundRect(posX, posY, w, w, w/4);
        painter->stroke();
        posX += w + margin;
        painter->setAntialias(1.0f + anim*8);
        painter->beginPath();
        painter->roundRect(posX, posY, w, w, w/4);
        painter->stroke();

        posY += w+margin;
        posX = margin/2;
        int lines = 12;
        painter->setAntialias(0);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            // Align to improve thin lines painting
            painter->moveTo(int(posX), int(posY+i*w/lines));
            painter->lineTo(int(posX+w), int(posY+i*w/lines));
            painter->stroke();
        }
        posX += w + margin;
        painter->setAntialias(1.0f);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(posX, posY+i*w/lines);
            painter->lineTo(posX+w, posY+i*w/lines);
            painter->stroke();
        }
        posX += w + margin;
        painter->setAntialias(1.0f + anim*5);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(posX, posY+i*w/lines);
            painter->lineTo(posX+w, posY+i*w/lines);
            painter->stroke();
        }

        posY += w+margin;
        posX = margin/2;
        painter->setAntialias(0);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(int(posX), int(posY+i*w/lines));
            painter->lineTo(int(posX+w), int(posY+i*w/lines) + 5);
            painter->stroke();
        }
        posX += w + margin;
        painter->setAntialias(1.0);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(posX, posY+i*w/lines);
            painter->lineTo(posX+w, posY+i*w/lines+5);
            painter->stroke();
        }
        posX += w + margin;
        painter->setAntialias(1.0f + anim*5);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(posX, posY+i*w/lines);
            painter->lineTo(posX+w, posY+i*w/lines+5);
            painter->stroke();
        }
    }
}

void CanvasPainterLancelotCppTests::testAntialiasing2()
{
    float topMargin = height() * 0.02f;
    float anim = 0.1f;

    {
        int rects = 3;
        float margin = std::floor(width() * 0.1f);
        float w = std::floor(width() / (rects) - margin);
        float posX = margin/2;
        float posY = std::floor(topMargin);
        int lines = 12;

        posY += w+margin;
        posX = margin/2;
        posY += anim*4;
        painter->setAntialias(0);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(int(posX), int(posY+i*w/lines));
            painter->lineTo(int(posX+w), int(posY+i*w/lines) + 5);
            painter->stroke();
        }
        posX += w + margin;
        painter->setAntialias(1.0f);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(posX, posY+i*w/lines);
            painter->lineTo(posX+w, posY+i*w/lines+5);
            painter->stroke();
        }
        posX += w + margin;
        painter->setAntialias(1.0f + anim*5);
        for (int i=0 ; i<lines ; i++) {
            painter->beginPath();
            painter->setLineWidth((i + 1) * 0.5f);
            painter->moveTo(posX, posY+i*w/lines);
            painter->lineTo(posX+w, posY+i*w/lines+5);
            painter->stroke();
        }
    }
}

void CanvasPainterLancelotCppTests::testTextAlignment()
{
    float topMargin = height() * 0.02f;
    float anim = 0.7f;

    {
        float margin = height() * 0.15f;
        float fontSize = height() * 0.03f;
        float posX = margin/2;
        float posY = topMargin + margin/4;

        QFont f3;
        f3.setPixelSize(fontSize);
        painter->setFont(f3);

        // Test horizontal alignments
        posX = width() * 0.5f;
        painter->setFillStyle(Qt::black);
        painter->fillRect(posX-1, posY-fontSize, 1, fontSize*3);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
        painter->setFillStyle(Qt::gray);
        painter->setTextAlign(QCanvasPainter::TextAlign::Left);
        painter->fillText("Left", posX, posY);
        painter->setFillStyle(Qt::green);
        painter->setTextAlign(QCanvasPainter::TextAlign::Center);
        painter->fillText("Center", posX, posY+fontSize);
        painter->setFillStyle(Qt::blue);
        painter->setTextAlign(QCanvasPainter::TextAlign::Right);
        painter->fillText("Right", posX, posY+fontSize*2);

        // Test vertical (baseline) alignments
        posY += margin;
        posX = width() * 0.1f;
        const float spacing = width() * 0.19;
        painter->setFillStyle(Qt::cyan);
        painter->fillRect(posX, posY-1, width()*0.8f, 1);
        painter->setTextAlign(QCanvasPainter::TextAlign::Center);
        painter->setFillStyle(Qt::gray);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
        painter->fillText("Top", posX, posY);
        painter->setFillStyle(Qt::red);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Hanging);
        painter->fillText("Hanging", posX+spacing, posY);
        painter->setFillStyle(Qt::green);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
        painter->fillText("Middle", posX + 2 * spacing, posY);
        painter->setFillStyle(Qt::blue);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Alphabetic);
        painter->fillText("Alphabetic", posX + 3 * spacing, posY);
        painter->setFillStyle(Qt::darkYellow);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
        painter->fillText("Bottom", posX + 4 * spacing, posY);

        // Test horizontal alignments with rect
        posY += margin;
        QRectF r1(width() * 0.5f, posY, 0, fontSize);
        float animX = width() * 0.4f * anim;
        r1.adjust(-animX, 0, animX, 0);
        painter->setStrokeStyle("#549221");
        painter->strokeRect(r1);

        painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
        painter->setFillStyle(Qt::black);
        painter->setTextAlign(QCanvasPainter::TextAlign::Left);
        painter->fillText("Left", r1);
        painter->setTextAlign(QCanvasPainter::TextAlign::Center);
        painter->fillText("Center", r1);
        painter->setTextAlign(QCanvasPainter::TextAlign::Right);
        painter->fillText("Right", r1);

        // Test vertical (baseline) alignments with rect
        posY += margin;
        posX = width() * 0.1f;
        float rectW = width() * 0.8f;
        QRectF r2(posX, posY, rectW, margin * anim);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(r2);

        painter->setTextAlign(QCanvasPainter::TextAlign::Left);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Alphabetic);
        painter->fillText("Left & Alphabetic", r2);
        painter->setTextAlign(QCanvasPainter::TextAlign::Right);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
        painter->fillText("Right & Bottom", r2);
        painter->setTextAlign(QCanvasPainter::TextAlign::Center);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
        painter->fillText("Center & Middle", r2);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
        painter->fillText("Center & Top", r2);

        posY += margin;
        painter->strokeRect(posX, posY, rectW, 8.5 * fontSize);
        painter->setTextDirection(QCanvasPainter::TextDirection::LeftToRight);
        QRectF r3(posX, posY, rectW, fontSize);
        painter->setTextAlign(QCanvasPainter::TextAlign::Start);
        painter->fillText("LeftToRight: Start-aligned", r3);
        posY += fontSize;
        QRectF r4(posX, posY, rectW, fontSize);
        painter->setTextAlign(QCanvasPainter::TextAlign::End);
        painter->fillText("LeftToRight: End-aligned", r4);
        painter->setTextDirection(QCanvasPainter::TextDirection::RightToLeft);
        posY += fontSize*2;
        QRectF r5(posX, posY, rectW, fontSize);
        painter->setTextAlign(QCanvasPainter::TextAlign::Start);
        painter->fillText("RightToLeft: Start-aligned", r5);
        posY += fontSize;
        QRectF r6(posX, posY, rectW, fontSize);
        painter->setTextAlign(QCanvasPainter::TextAlign::End);
        painter->fillText("RightToLeft: End-aligned", r6);
        // Autodetect direction from the text string.
        painter->setTextDirection(QCanvasPainter::TextDirection::Auto);
        posY += fontSize*2;
        QRectF r7(posX, posY, rectW, fontSize);
        painter->setTextAlign(QCanvasPainter::TextAlign::Start);
        painter->fillText("English", r7);
        posY += fontSize;
        QRectF r8(posX, posY, rectW, fontSize);
        painter->setTextAlign(QCanvasPainter::TextAlign::Start);
        painter->fillText("خامل", r8);
        painter->setTextDirection(QCanvasPainter::TextDirection::LeftToRight);
    }
}

void CanvasPainterLancelotCppTests::testTextWrapping()
{
    float topMargin = height() * 0.02f;
    float anim = 0.5f;

    {
        float margin = height() * 0.02f;
        float fontSize = height() * 0.03f;
        float posX = margin + (anim * width() * 0.2f);
        float posY = topMargin;
        float rectW = (width() - 2 * margin) - (anim * width() * 0.4f);
        float rectH = height() * 0.18;

        QFont f3;
        f3.setPixelSize(fontSize);
        painter->setFont(f3);

        // Test text alignment and line wrapping
        QString s("This is a test string which is slightly longer one. Yes, we want long string for this.");
        painter->setFillStyle(Qt::black);
        painter->setTextWrapMode(QCanvasPainter::WrapMode::WordWrap);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
        painter->setTextAlign(QCanvasPainter::TextAlign::Left);
        painter->setTextLineHeight(-10);
        QRectF rect1(posX, posY, rectW, rectH);
        painter->setStrokeStyle(0xFF393E46);
        painter->strokeRect(rect1);
        painter->fillText(s, rect1);
        QRectF boundingRect1 = painter->textBoundingBox(s, rect1);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(boundingRect1);

        posY += rectH + margin;
        painter->setTextAlign(QCanvasPainter::TextAlign::Center);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
        painter->setTextLineHeight(0);
        QRectF rect2(posX, posY, rectW, rectH);
        painter->setStrokeStyle(0xFF393E46);
        painter->strokeRect(rect2);
        painter->fillText(s, rect2);
        QRectF boundingRect2 = painter->textBoundingBox(s, rect2);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(boundingRect2);

        posY += rectH + margin;
        painter->setTextAlign(QCanvasPainter::TextAlign::Right);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
        painter->setTextLineHeight(10);
        QRectF rect3(posX, posY, rectW, rectH);
        painter->setStrokeStyle(0xFF393E46);
        painter->strokeRect(rect3);
        painter->fillText(s, rect3);
        QRectF boundingRect3 = painter->textBoundingBox(s, rect3);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(boundingRect3);

        posY += 2.0 * rectH + margin;
        posX = 0.5f * width();
        QFont f4;
        f4.setPixelSize(width() * 0.04f);
        painter->setFont(f4);
        painter->setTextLineHeight(0);
        painter->setTextWrapMode(QCanvasPainter::WrapMode::WordWrap);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
        painter->setTextAlign(QCanvasPainter::TextAlign::Center);
        float circleRadius = width() * 0.1f * (1.0 + anim);
        float textMargin = width() * 0.04f;
        float textW = 2 * (circleRadius - textMargin);
        QString text = QStringLiteral("Circle radius: %1").arg(qRound(circleRadius));
        painter->fillText(text, posX, posY, textW);
        painter->beginPath();
        painter->setLineWidth(1);
        painter->circle(posX, posY, circleRadius);
        painter->setStrokeStyle(0xFF393E46);
        painter->stroke();
        QRectF boundingRect4 = painter->textBoundingBox(text, posX, posY, textW);
        painter->setStrokeStyle(0xFFDFD0B8);
        painter->strokeRect(boundingRect4);
    }
}

void CanvasPainterLancelotCppTests::testTextBrushes()
{
    float topMargin = height() * 0.02f;
    float anim = 0.5f;

    QCanvasPainter::ImageFlags flags = QCanvasPainter::ImageFlag::Repeat | QCanvasPainter::ImageFlag::GenerateMipmaps;
    QCanvasImage patternImage2 = painter->addImage(QImage(":/images/pattern2.png"), flags);

    {
        float margin = height() * 0.04f;
        float posX = margin * 0.5f;
        float posY = topMargin + margin + 0.1f;
        float fontSize1 = width() * 0.09f;
        float fontSize2 = width() * 0.13f;
        QFont f;
        f.setPixelSize(fontSize1);
        painter->setFont(f);
        float w = width();
        QCanvasLinearGradient g1(0, 0, 0, 0);
        g1.setStartColor(QColor(100,150,100));
        g1.setEndColor(QColor(0,0,0));
        g1.setStartPosition(posX + (w*0.5f * anim), 0);
        g1.setEndPosition(posX + w + 1 - (w*0.5f * anim), 0);
        painter->setFillStyle(g1);
        painter->fillText("Text with linear gradient", posX, posY);

        posY += margin + fontSize1;
        float h = 80;
        QCanvasRadialGradient g2;
        g2.setStartColor(QColor(100, 100, 20, 255));
        g2.setColorAt(0.5, QColor(140, 50, 20, 255));
        g2.setEndColor(QColor(0,0,0,0));
        g2.setCenterPosition(posX + w/2, posY + h/2);
        g2.setOuterRadius(0.6*w*anim);
        g2.setInnerRadius(0.1*w*anim);
        painter->setFillStyle(g2);
        painter->fillText("Text with radial gradient", posX, posY);

        posY += margin + fontSize2;
        QCanvasConicalGradient g4;
        g4.setCenterPosition(width() * 0.5, posY - fontSize2 * 0.5);
        g4.setAngle(anim);
        g4.setStartColor(QColor(255, 255, 0, 255));
        g4.setColorAt(0.25, QColor(0, 255, 0, 255));
        g4.setColorAt(0.5, QColor(0, 0, 255, 255));
        g4.setColorAt(0.75, QColor(255, 0, 255, 255));
        g4.setEndColor(QColor(255, 255, 0, 255));
        painter->setFillStyle(g4);
        f.setPixelSize(fontSize2);
        painter->setFont(f);
        painter->fillText("MULTIGRADIENT", posX, posY);

        painter->setFillStyle(Qt::black);
        painter->fillRect(0, posY, width(), height() - posY);
        posY += margin + fontSize2;
        QCanvasImagePattern p1 = QCanvasImagePattern(patternImage2);
        p1.setImageSize(64, 64);
        p1.setStartPosition(0, anim * 10);
        painter->setFillStyle(p1);
        painter->fillText("IMAGE PATTERN", posX, posY);

        posY += margin + fontSize2;
        QCanvasGridPattern p2;
        p2.setStartPosition(anim * 10, 0);
        p2.setLineWidth(2);
        p2.setCellSize(4, 4);
        p2.setLineColor("#00414A");
        p2.setBackgroundColor("#2CDE85");
        painter->setFillStyle(p2);
        painter->fillText("GRID PATTERN", posX, posY);
    }
}

void CanvasPainterLancelotCppTests::testTextFonts()
{
    float topMargin = height() * 0.02f;
    float anim = 0.5f;

    {
        float margin = height() * 0.04f;
        float posX = margin/2;
        float posY = topMargin;
        painter->setFillStyle("#123321");
        painter->fillText("This is text without setting any font", posX, posY);

        posY += margin;
        QFont f;
        painter->setFillStyle("#13dc6d");
        f.setPixelSize(20);
        painter->setFont(f);
        painter->fillText("Size 20px", posX, posY);
        posY += margin;
        f.setPointSize(20);
        painter->setFont(f);
        painter->fillText("Size 20pt", posX, posY);

        posY += margin;
        painter->setFillStyle("#17367e");
        painter->setTextAntialias(1.0 + anim * 5.0);
        f.setPixelSize(14);
        painter->setFont(f);
        painter->fillText("Size 14px, blurring", posX, posY);
        posY += margin;
        f.setPixelSize(24);
        painter->setFont(f);
        painter->fillText("Size 24px, blurring", posX, posY);
        posY += 1.3 * margin;
        f.setPixelSize(34);
        painter->setFont(f);
        painter->fillText("Size 34px, blurring", posX, posY);
        painter->setTextAntialias(1.0);

        posY += margin;
        painter->setFillStyle("#8c1cb8");
        f.setPixelSize(margin * 0.8);
        f.setWeight(QFont::Weight::Light);
        f.setItalic(false);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: Light", posX, posY);
        f.setItalic(true);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: Light Italic", posX, posY);

        f.setWeight(QFont::Weight::Normal);
        f.setItalic(false);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: Normal", posX, posY);
        f.setItalic(true);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: Normal Italic", posX, posY);

        f.setWeight(QFont::Weight::Bold);
        f.setItalic(false);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: Bold", posX, posY);
        f.setItalic(true);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: Bold Italic", posX, posY);

        f.setItalic(false);
        f.setCapitalization(QFont::SmallCaps);
        painter->setFont(f);
        posY += margin;
        painter->fillText("Style: SmallCaps", posX, posY);

        posY += 2*margin;
        QFont f2("Pacifico", margin);
        painter->setFont(f2);
        painter->fillText("This is text with custom font", posX, posY);

        posY += 2*margin;
        QFont f5;
        f5.setPixelSize(margin*0.8f);
        f5.setLetterSpacing(QFont::AbsoluteSpacing, 6 * anim - 2);
        painter->setFont(f5);
        painter->setFillStyle("#c11b63");
        painter->fillText("This is text with varying letter spacing", posX, posY);

        posY += 2*margin;
        f5.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        f5.setWordSpacing(20 * anim - 5);
        painter->setFont(f5);
        painter->fillText("This is text with varying word spacing", posX, posY);
    }
}

void CanvasPainterLancelotCppTests::testCanvasPathWithAddPath()
{
    const int personPathGroup = 5;
    const int heartPathGroup = 5;
    const float d = std::min(width(), height()) * 0.1f;

    QCanvasPath personPath;
    personPath.addPath(QStringLiteral(
        "M152,84a36,36,0,1,0-36-36A36.04061,36.04061,0,0,0,152,84Zm0-48Z"
        "m64,112a12.00028,12.00028,0,0,1-12,12c-37.20215,0-55.50781-19.66406"
        "-70.21729-35.46484-.45459-.48828-.89794-.96192-1.34667-1.44239l-8.02393,"
        "18.45484,34.5625,24.68774A11.999,11.999,0,0,1,164,176v56a12,12,0,0,1-24,"
        "0V182.17578l-25.37207-18.12353L83.00488,236.78516a12.00021,12.00021,0,0,"
        "1-22.00976-9.57032l37.55322-86.37255.02881-.06592,13.74463-31.61377c-"
        "8.09571-.96167-18.24219,2.0072-30.35889,8.94482a159.5463,159.5463,0,0,0-"
        "29.47754,22.37793,12.0001,12.0001,0,0,1-16.9707-16.9707A183.31075,"
        "183.31075,0,0,1,70.03711,97.28027c36.06689-20.65234,58.03027-11.69433,"
        "70.104-.54589,3.93116,3.63085,7.63037,7.60449,11.208,11.44726C164.667,"
        "122.4873,177.24561,136,204,136A12.00028,12.00028,0,0,1,216,148Z"
    ));

    painter->setLineWidth(d * 0.1f);
    painter->setFillStyle(Qt::white);
    painter->setStrokeStyle(Qt::black);
    {
        const float cx = 120.0f;
        const float cy = 120.0f;
        painter->translate(width() * 0.2f - cx, height() * 0.5f - cy);
        painter->fill(personPath, personPathGroup);
        painter->stroke(personPath, personPathGroup);
        painter->resetTransform();
    }

    painter->setFillStyle(QColor(0xE0, 0x20, 0x20));
    painter->setStrokeStyle(Qt::black);
    painter->setLineWidth(d * 0.05f);

    QCanvasPath heartPath;
    heartPath.moveTo(0, d / 4);
    heartPath.quadraticCurveTo(0, 0, d / 4, 0);
    heartPath.quadraticCurveTo(d / 2, 0, d / 2, d / 4);
    heartPath.quadraticCurveTo(d / 2, 0, d * 3 / 4, 0);
    heartPath.quadraticCurveTo(d, 0, d, d / 4);
    heartPath.quadraticCurveTo(d, d / 2, d * 3 / 4, d * 3 / 4);
    heartPath.lineTo(d / 2, d);
    heartPath.lineTo(d / 4, d * 3 / 4);
    heartPath.quadraticCurveTo(0, d / 2, 0, d / 4);

    const int items = 20;
    for (int i = 0; i < items; i++) {
        float posX = 0.4f * width() + 0.5f * (width() - d) * (float(i) / items);
        float posY = (height() - d) * 0.5f + (height() - d) * 0.3f * std::sin(float(i) / items * 10.0f);
        painter->resetTransform();
        painter->translate(posX, posY);
        painter->stroke(heartPath, heartPathGroup);
        painter->fill(heartPath, heartPathGroup);
    }
    painter->resetTransform();
}
