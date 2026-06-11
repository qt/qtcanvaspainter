// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWidgets>
#include <QtCanvasPainter>
#include "../shared/floppy_xpm"
#include <QCommandLineParser>
#include <private/qvectorpath_p.h>
#include <private/qpainterpath_p.h>

using CanvasPainterTestFunction = void (*)(QCanvasPainter *p);
using PainterTestFunction = void (*)(QPainter *p);

struct TestDescription {
    QString name;
    CanvasPainterTestFunction cFunc;
    PainterTestFunction qFunc;
};

static void testGradients(QCanvasPainter *p)
{
    QRectF rect1(50, 50, 50, 50);
    QCanvasLinearGradient linearGrad(rect1.topLeft(), rect1.bottomRight());
    linearGrad.setColorAt(0.0, "#1a2a3c");
    linearGrad.setColorAt(0.5, "#fdbb7d");
    linearGrad.setColorAt(1.0, "#1a2a3c");
    p->setFillStyle(linearGrad);
    p->fillRect(rect1);
    p->fillRect(120,20,50,50);
    p->translate(50,60);
    p->fillRect(rect1);

    QRectF rect2(200, 50, 50, 50);
    QCanvasRadialGradient radialGradOffset;
    radialGradOffset.setStartColor(QColor(0,255,0,255));
    radialGradOffset.setEndColor(QColor(255,0,0,255));
    radialGradOffset.setCenterPosition(rect2.x() + rect2.width()/2, rect2.y() + rect2.height()/2);
    radialGradOffset.setOuterRadius(25);
    radialGradOffset.setInnerRadius(0);
    p->setFillStyle(radialGradOffset);
    p->fillRect(rect2);

    p->save();
    QCanvasRadialGradient radialGradCentered(200, 100, 50);
    radialGradCentered.setColorAt(0, Qt::red);
    radialGradCentered.setColorAt(0.35, QColor("pink"));
    radialGradCentered.setColorAt(1, Qt::white);
    p->setFillStyle(radialGradCentered);

    p->fillRect(150, 50, 100, 100);
    p->strokeRect(150, 50, 100, 100);

    QTransform bt;
    bt.translate(-25, -25);
    p->setBrushTransform(bt);

    p->translate(150, 10);
    p->fillRect(150, 50, 100, 100);
    p->strokeRect(150, 50, 100, 100);
    p->restore();

    QRectF objRect{10, 200, 100, 150};
    QCanvasRadialGradient radialGradObjRect(0.5, 0.5, 0.5);
    radialGradObjRect.setColorAt(0, Qt::blue);
    radialGradObjRect.setColorAt(0.33, Qt::white);
    radialGradObjRect.setColorAt(0.66, Qt::red);
    radialGradObjRect.setColorAt(0.99, Qt::white);
    radialGradObjRect.setColorAt(1, Qt::green);
    p->setFillStyle(radialGradObjRect);

    QTransform bt2;
    bt2.translate(objRect.left(), objRect.top());
    bt2.scale(objRect.width(), objRect.height());
    p->setBrushTransform(bt2);

    p->fillRect(objRect);
    p->strokeRect(objRect);

    QRectF objRect3{210, 200, 100, 150};
    QCanvasLinearGradient linearGradObjRect(0, 0, 0.2, 1);
    linearGradObjRect.setColorAt(0, Qt::red);
    linearGradObjRect.setColorAt(0.45, Qt::yellow);
    linearGradObjRect.setColorAt(0.50, Qt::green);
    linearGradObjRect.setColorAt(0.55, Qt::white);
    linearGradObjRect.setColorAt(1, Qt::blue);
    p->setFillStyle(linearGradObjRect);

    QTransform bt3;
    bt3.translate(objRect3.left(), objRect3.top());
    bt3.scale(objRect3.width(), objRect3.height());
    p->setBrushTransform(bt3);

    p->fillRect(objRect3);
    p->strokeRect(objRect3);
}

static void testGradients_reference(QPainter *p)
{
    // ── Linear gradient in logical coordinates ────────────────────────────────
    // Gradient coords are absolute logical points → LogicalMode (the default)
    QRectF rect1(50, 50, 50, 50);
    QLinearGradient linearGrad(rect1.topLeft(), rect1.bottomRight());
    linearGrad.setColorAt(0.0, QColor("#1a2a3c"));
    linearGrad.setColorAt(0.5, QColor("#fdbb7d"));
    linearGrad.setColorAt(1.0, QColor("#1a2a3c"));
    // LogicalMode is the default, but be explicit for clarity
    linearGrad.setCoordinateMode(QGradient::LogicalMode);
    p->setPen(Qt::NoPen);
    p->fillRect(rect1, linearGrad);
    p->fillRect(QRectF(120, 20, 50, 50), linearGrad);
    p->translate(50, 60);
    p->fillRect(rect1, linearGrad);

    // ── Radial gradient with explicit center in logical coordinates ───────────
    // setCenterPosition + setOuterRadius → absolute logical coords → LogicalMode
    QRectF rect2(200, 50, 50, 50);
    QRadialGradient radialGradOffset(
        rect2.x() + rect2.width() / 2,
        rect2.y() + rect2.height() / 2,
        25);   // outerRadius; innerRadius=0 means no focal offset
    radialGradOffset.setColorAt(0, QColor(0, 255, 0, 255));
    radialGradOffset.setColorAt(1, QColor(255, 0, 0, 255));
    radialGradOffset.setCoordinateMode(QGradient::LogicalMode);
    p->fillRect(rect2, radialGradOffset);

    // ── Radial gradient centered at (200,100) r=50, with brushTransform ───────
    // The canvas version uses absolute logical coords, then a brushTransform
    // that offsets by (-25,-25) and a painter translate of (150,10).
    // In QPainter terms the gradient simply stays in logical space.
    p->save();
    QRadialGradient radialGradCentered(200, 100, 50);
    radialGradCentered.setColorAt(0,    Qt::red);
    radialGradCentered.setColorAt(0.35, QColor("pink"));
    radialGradCentered.setColorAt(1,    Qt::white);
    radialGradCentered.setCoordinateMode(QGradient::LogicalMode);

    p->fillRect(QRectF(150, 50, 100, 100), radialGradCentered);
    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(QRectF(150, 50, 100, 100));

    // Apply the brushTransform offset via a gradient whose center is shifted
    // by (-25,-25), then translate painter by (150,10) to match canvas version
    QRadialGradient radialGradCenteredShifted(200 - 25, 100 - 25, 50);
    radialGradCenteredShifted.setColorAt(0,    Qt::red);
    radialGradCenteredShifted.setColorAt(0.35, QColor("pink"));
    radialGradCenteredShifted.setColorAt(1,    Qt::white);
    radialGradCenteredShifted.setCoordinateMode(QGradient::LogicalMode);
    p->translate(150, 10);
    p->fillRect(QRectF(150, 50, 100, 100), radialGradCenteredShifted);
    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(QRectF(150, 50, 100, 100));
    p->restore();

    // ── Radial gradient in object bounding-box coordinates ───────────────────
    // Canvas emulates ObjectMode via brushTransform: translate(left,top) scale(w,h)
    // → QPainter ObjectMode maps (0,0)..(1,1) onto the bounding rect directly
    QRectF objRect{10, 200, 100, 150};
    QRadialGradient radialGradObjRect(0.5, 0.5, 0.5);
    radialGradObjRect.setColorAt(0,    Qt::blue);
    radialGradObjRect.setColorAt(0.33, Qt::white);
    radialGradObjRect.setColorAt(0.66, Qt::red);
    radialGradObjRect.setColorAt(0.99, Qt::white);
    radialGradObjRect.setColorAt(1,    Qt::green);
    radialGradObjRect.setCoordinateMode(QGradient::ObjectMode);
    p->setPen(Qt::NoPen);
    p->fillRect(objRect, radialGradObjRect);
    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(objRect);

    // ── Linear gradient in object bounding-box coordinates ───────────────────
    // Same pattern: canvas uses brushTransform translate+scale → ObjectMode
    QRectF objRect3{210, 200, 100, 150};
    QLinearGradient linearGradObjRect(0, 0, 0.2, 1);
    linearGradObjRect.setColorAt(0,    Qt::red);
    linearGradObjRect.setColorAt(0.45, Qt::yellow);
    linearGradObjRect.setColorAt(0.50, Qt::green);
    linearGradObjRect.setColorAt(0.55, Qt::white);
    linearGradObjRect.setColorAt(1,    Qt::blue);
    linearGradObjRect.setCoordinateMode(QGradient::ObjectMode);
    p->setPen(Qt::NoPen);
    p->fillRect(objRect3, linearGradObjRect);
    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(objRect3);
}

static void testPen(QCanvasPainter *p)
{
    p->setLineWidth(1);
    p->strokeRect(50 - 15, 10, 200 + 30, 10);

    p->setFillStyle("lightblue");
    p->setStrokeStyle(Qt::red);
    p->setLineWidth(30);
    QRectF r(50, 50, 200, 200);
    p->fillRect(r);
    p->strokeRect(r);
    p->setFillStyle(Qt::black);
    p->fillRect(50 - 15, 50 - 15,30,30);

    p->translate(250, 0);
    QCanvasLinearGradient grad(0.50, 0, 2.50, 0);
    grad.setColorAt(0, Qt::yellow);
    grad.setColorAt(0.01, Qt::blue);
    grad.setColorAt(0.33, Qt::white);
    grad.setColorAt(0.66, Qt::red);
    grad.setColorAt(0.99, Qt::white);
    grad.setColorAt(1, Qt::green);
    p->setFillStyle(grad);
    p->fillRect(50, 10, 200, 10);
    p->setFillStyle(Qt::gray);
    p->fillRect(r);

    p->setStrokeStyle(grad);
    QTransform t;
    t.scale(100, 100);
    p->setBrushTransform(t);

    p->strokeRect(r);
}

static void testPen_reference(QPainter *p)
{
    p->drawRect(50 - 15, 10, 200 + 30, 10);

    p->setBrush("lightblue");
    p->setPen(QPen(Qt::red, 30, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    QRectF r(50, 50, 200, 200);
    p->drawRect(r);
    p->fillRect(50 - 15, 50 - 15,30,30, Qt::black);

    p->translate(250, 0);
    QLinearGradient grad(0.50, 0, 2.50, 0);
    grad.setColorAt(0, Qt::yellow);
    grad.setColorAt(0.01, Qt::blue);
    grad.setColorAt(0.33, Qt::white);
    grad.setColorAt(0.66, Qt::red);
    grad.setColorAt(0.99, Qt::white);
    grad.setColorAt(1, Qt::green);

    p->fillRect(50, 10, 200, 10, grad);

    p->setBrush(Qt::gray);
    QBrush b(grad);
    QTransform t;
    t.scale(100, 100);
    b.setTransform(t);
    p->setPen(QPen(b, 30, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));

    p->drawRect(r);
}

static void testImage(QCanvasPainter *p)
{
    QImage img(floppy);
    auto ci = p->addImage(img, QCanvasPainter::ImageFlag::Repeat);
    //p->drawImage(ci, 10, 10, 200, 200);
    QCanvasImagePattern ip(ci);

    p->setFillStyle(ip);

    p->beginPath();
    p->ellipse(QRectF(30, 30, 500, 300));
    p->fill();
}

static void testImage_reference(QPainter *p)
{
    QImage img(floppy);
    QBrush brush(img);
    p->setBrush(brush);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QRectF(30, 30, 500, 300));
}

static void testText(QCanvasPainter *p)
{
    QString s = QStringLiteral("Test text");
    p->setFillStyle(Qt::black);

    auto drawTextWithRect = [&](const QString &s, const QRectF &r) {
        p->fillText(s, r);
        p->strokeRect(p->textBoundingBox(s, r));
    };

    QFont f1;
    f1.setPointSize(18);
    p->setFont(f1);
    drawTextWithRect(s, {100, 20, 100, 30});

    f1.setPointSize(28);
    p->setFont(f1);
    p->setFillStyle(Qt::red);
    p->setStrokeStyle(Qt::red);
    drawTextWithRect(s, {100, 100, 100, 30});

    p->setFillStyle(Qt::darkGreen);
    p->setStrokeStyle(Qt::darkGreen);

    const QStringList bidiStrings{
        "ABC 123",
        "123 مرحبًا", // numbers first in string
        "مرحبًا 123", // numbers last in string
        "ABC 123 مرحبًا"};

    auto drawBidiStrings = [&](const QRectF &firstRect) {
        auto r = firstRect;
        for (auto &str : bidiStrings) {
            drawTextWithRect(str, r);
            r.translate(100, 10);
        }
    };

    QFont f2(QStringLiteral("Noto Sans Arabic UI"), 16);
    p->setFont(f2);
    drawBidiStrings({100, 200, 90, 30});

    QFont f3("FreeSans", 16);
    p->setFont(f3);
    p->setTextDirection(QCanvasPainter::TextDirection::Inherit);
    drawBidiStrings({100, 250, 90, 30});

    p->setTextDirection(QCanvasPainter::TextDirection::Auto);
    drawBidiStrings({100, 300, 90, 30});

    p->setTextDirection(QCanvasPainter::TextDirection::LeftToRight);
    drawBidiStrings({100, 350, 90, 30});

    p->setTextDirection(QCanvasPainter::TextDirection::RightToLeft);
    drawBidiStrings({100, 400, 90, 30});


}

static void testText_reference(QPainter *p)
{
    QString s = QStringLiteral("Test text");
    p->setPen(Qt::black);

    auto drawTextWithRect = [&](const QString &s, const QRectF &r) {
        p->drawText(r, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip, s);
        QRectF bb1 = p->fontMetrics().boundingRect(r.toRect(), Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip, s);
        p->drawRect(bb1);
    };

    QFont f1;
    f1.setPointSize(18);
    p->setFont(f1);
    drawTextWithRect(s, {100, 20, 100, 30});

    f1.setPointSize(28);
    p->setFont(f1);
    p->setPen(Qt::red);
    drawTextWithRect(s, {100, 100, 100, 30});

    p->setPen(Qt::darkGreen);

    const QStringList bidiStrings{
        "ABC 123",
        "123 مرحبًا", // numbers first in string
        "مرحبًا 123", // numbers last in string
        "ABC 123 مرحبًا"};

    auto drawBidiStrings = [&](const QRectF &firstRect) {
        auto r = firstRect;
        for (auto &str : bidiStrings) {
            drawTextWithRect(str, r);
            r.translate(100, 10);
        }
    };

    QFont f2(QStringLiteral("Noto Sans Arabic UI"), 16);
    p->setFont(f2);
    drawBidiStrings({100, 200, 90, 30});

    QFont f3("FreeSans", 16);
    p->setFont(f3);
    drawBidiStrings({100, 250, 90, 30});

    p->setLayoutDirection(Qt::LayoutDirectionAuto);
    drawBidiStrings({100, 300, 90, 30});

    p->setLayoutDirection(Qt::LeftToRight);
    drawBidiStrings({100, 350, 90, 30});

    p->setLayoutDirection(Qt::RightToLeft);
    drawBidiStrings({100, 400, 90, 30});
}

static void testAntialiasing(QCanvasPainter *p)
{
    QPainterPath path;
    path.moveTo(50, 200);
    path.lineTo(50, 250);
    path.quadTo(100, 250, 100, 200);
    path.lineTo(50, 200);

    p->setFillStyle(Qt::red);

    p->setAntialias(0);
    p->beginPath();
    p->addPath(path);
    p->fill();

    p->translate(100, 0);
    p->setAntialias(1.0f);
    p->beginPath();
    p->addPath(path);
    p->fill();

    p->translate(100, 0);
    p->setAntialias(0);
    p->beginPath();
    p->addPath(path);
    p->fill();

    p->translate(100, 0);
    p->setAntialias(1.0f);
    p->beginPath();
    p->addPath(path);
    p->fill();
}

static void testAntialiasing_reference(QPainter *p)
{
    QPainterPath path;
    path.moveTo(50, 200);
    path.lineTo(50, 250);
    path.quadTo(100, 250, 100, 200);
    path.closeSubpath();

    p->setPen(Qt::NoPen);
    p->setBrush(Qt::red);

    p->setRenderHint(QPainter::Antialiasing, false);
    p->drawPath(path);

    p->translate(100, 0);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->drawPath(path);

    p->translate(100, 0);
    p->setRenderHint(QPainter::Antialiasing, false);
    p->drawPath(path);

    p->translate(100, 0);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->drawPath(path);
}

static void testTransform(QCanvasPainter *p)
{
    p->setFillStyle(Qt::red);
    p->fillRect(10, 10, 200, 75);

    p->setFillStyle(Qt::green);
    p->translate(0, 100);
    p->fillRect(10, 10, 200, 75);

    p->setFillStyle(Qt::blue);
    p->rotate(M_PI/180 * 15);
    p->fillRect(10, 10, 200, 75);
}

static void testTransform_reference(QPainter *p)
{
    p->setPen(Qt::NoPen);

    p->setBrush(Qt::red);
    p->drawRect(QRectF(10, 10, 200, 75));

    p->setBrush(Qt::green);
    p->translate(0, 100);
    p->drawRect(QRectF(10, 10, 200, 75));

    p->setBrush(Qt::blue);
    p->rotate(15.0);   // QPainter::rotate takes degrees
    p->drawRect(QRectF(10, 10, 200, 75));
}

static void testPath(QCanvasPainter *p)
{
    QCanvasPath path;
    path.moveTo(10, 10);
    path.lineTo(100, 40);
    path.lineTo(100, 100);
    path.quadraticCurveTo(50, 50, 10, 100);
    path.closePath();

    QCanvasPath path2;
    path2.moveTo(10, 10);
    path2.lineTo(100, 40);
    path2.lineTo(100, 100);
    path2.lineTo(10, 100);
    path2.closePath();

    QCanvasPath path3;
    path3.moveTo(10, 10);
    path3.lineTo(100, 40);
    path3.lineTo(100, 100);
    path3.quadraticCurveTo(50, 150, 10, 100);
    path3.closePath();

    QCanvasPath path4;
    path4.moveTo(10, 10);
    path4.quadraticCurveTo(150, 150, 10, 100);

    p->setFillStyle(Qt::red);

    p->fill(path);

    p->setFillStyle(Qt::green);
    p->translate(0, 150);
    p->fill(path2);

    p->setFillStyle(Qt::blue);
    p->translate(0, 150);
    p->fill(path3);

    p->setFillStyle(Qt::yellow);
    p->setLineWidth(3);
    p->translate(150, 0);
    p->fill(path4);
    p->stroke(path4);
}

static void testPath_reference(QPainter *p)
{
    QPainterPath path;
    path.moveTo(10, 10);
    path.lineTo(100, 40);
    path.lineTo(100, 100);
    path.quadTo(50, 50, 10, 100);
    path.closeSubpath();

    QPainterPath path2;
    path2.moveTo(10, 10);
    path2.lineTo(100, 40);
    path2.lineTo(100, 100);
    path2.lineTo(10, 100);
    path2.closeSubpath();

    QPainterPath path3;
    path3.moveTo(10, 10);
    path3.lineTo(100, 40);
    path3.lineTo(100, 100);
    path3.quadTo(50, 150, 10, 100);
    path3.closeSubpath();

    QPainterPath path4;
    path4.moveTo(10, 10);
    path4.quadTo(150, 150, 10, 100);

    p->setPen(Qt::NoPen);

    p->setBrush(Qt::red);
    p->drawPath(path);

    p->setBrush(Qt::green);
    p->translate(0, 150);
    p->drawPath(path2);

    p->setBrush(Qt::blue);
    p->translate(0, 150);
    p->drawPath(path3);

    p->setBrush(Qt::yellow);
    p->setPen(QPen(Qt::black, 3));
    p->translate(150, 0);
    p->drawPath(path4);
}

static QList<QPainterPath> createPainterPaths()
{
    int x = 0;
    QPainterPath path;
    path.moveTo(x + 10, 10);
    path.lineTo(x + 100, 40);
    path.lineTo(x + 100, 100);
    path.quadTo(x + 50, 50, x +10, 100);
    path.closeSubpath();

    x += 150;
    QPainterPath path2;
    path2.moveTo(x + 10, 10);
    path2.lineTo(x + 100, 40);
    path2.lineTo(x + 100, 100);
    path2.lineTo(x + 10, 100);

    x += 150;
    QPainterPath path3;
    path3.moveTo(x + 10, 10);
    path3.lineTo(x + 100, 40);
    path3.lineTo(x + 100, 100);
    path3.quadTo(x + 50, 150, x + 10, 100);

    x += 150;

    QPainterPath path4;
    path4.moveTo(x + 10, 10);
    path4.quadTo(x + 150, 150, x + 10, 100);

    return {path, path2, path3, path4};
}

static void testPainterPath(QCanvasPainter *p)
{
    p->setFillStyle(Qt::red);
    p->setLineWidth(3);
    p->setStrokeStyle(Qt::blue);
    const auto paths = createPainterPaths();
    for (auto &path : paths) {
        p->beginPath();
        p->addPath(path);
        p->fill();
        p->stroke();
    }
}

static void testPainterPath_reference(QPainter *p)
{
    p->setBrush(Qt::red);
    p->setPen(QPen(Qt::blue, 3));
    const auto paths = createPainterPaths();
    for (auto &path : paths) {
        p->drawPath(path);
    }
}

static void testClip(QCanvasPainter *p)
{
    p->rotate(M_PI / 10);

    p->strokeRect(9, 9, 102, 102);
    p->setClipRect(10, 10, 100, 100);
    p->setFillStyle(Qt::red);
    p->fillRect(0, 0, 300, 300);

    p->rotate(M_PI / 20);
    p->translate(50, 0);
    p->resetClipping();
    p->strokeRect(9, 149, 252, 52);
    p->setClipRect(10, 150, 250, 50);
    p->setFillStyle(Qt::green);
    p->beginPath();
    p->ellipse(QRectF(10, 150, 300, 100));
    p->fill();
}

static void testClip_reference(QPainter *p)
{
    p->rotate(180.0 / M_PI * (M_PI / 10));  // same angle as canvas version

    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(QRectF(9, 9, 102, 102));
    p->setClipRect(QRectF(10, 10, 100, 100));
    p->setBrush(Qt::red);
    p->setPen(Qt::NoPen);
    p->drawRect(0, 0, 300, 300);

    p->rotate(180.0 / M_PI * (M_PI / 20));
    p->translate(50, 0);
    p->setClipping(false);

    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(QRectF(9, 149, 252, 52));

    p->setClipRect(QRectF(10, 150, 250, 50));
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::green);
    p->drawEllipse(QRectF(10, 150, 300, 100));
}

static void testGradientBug(QCanvasPainter *p)
{
    QCanvasGradientStops stops = {
                             {0.0, "green"},
                             {0.2, "black"},
                             {0.4, "blue"},
                             {0.6, "white"},
                             {0.8, "yellow"},
                             {1.0, "red"},
                             };
    QRectF rect1(50, 50, 100, 100);
    QCanvasLinearGradient lg;
    for (auto &stop: stops)
        lg.setColorAt(stop.position, stop.color);
    lg.setStartPosition(rect1.topLeft());
    lg.setEndPosition(rect1.topRight());

    QRectF rect2(200, 50, 100, 100);
    QCanvasConicalGradient cg(rect2.center(), 0);

    for (auto &stop: stops)
        cg.setColorAt(1.0 - stop.position, stop.color);

    if (1) { // trigger bug
        p->setFillStyle(lg);
        p->fillRect(rect1);
    }

    p->setFillStyle(cg);
    p->fillRect(rect2);
}

static void testGradientBug_reference(QPainter *p)
{
    QCanvasGradientStops stops = {
                             {0.0, "green"},
                             {0.2, "black"},
                             {0.4, "blue"},
                             {0.6, "white"},
                             {0.8, "yellow"},
                             {1.0, "red"},
                             };

    QRectF rect1(50, 50, 100, 100);
    QLinearGradient lg(rect1.topLeft(), rect1.topRight());
    for (auto &stop : stops)
        lg.setColorAt(stop.position, stop.color);

    QRectF rect2(200, 50, 100, 100);
    QConicalGradient cg(rect2.center(), 0);
    for (auto &stop : stops)
        cg.setColorAt(stop.position, stop.color);

    p->setPen(Qt::NoPen);
    p->fillRect(rect1, lg);
    p->fillRect(rect2, cg);
}

static void testGradientPad(QCanvasPainter *p)
{
    QCanvasGradientStops stops = {
                             {0.0, "transparent"},
                             {0.001, "red"},
                             {0.5, "lightblue"},
                             {0.999, "red"},
                             {1.0, "transparent"},
                             };
    QRectF rect1(50, 50, 200, 100);
    QCanvasLinearGradient lg;
    lg.setStops(stops);
    lg.setStartPosition(QPointF(100, 50));
    lg.setEndPosition(QPointF(200, 50));

    p->setFillStyle(lg);
    p->fillRect(rect1);
    p->strokeRect(rect1);
}

static void testGradientPad_reference(QPainter *p)
{
    QRectF rect1(50, 50, 200, 100);
    QLinearGradient lg(QPointF(100, 50), QPointF(200, 50));
    lg.setSpread(QGradient::PadSpread);
    lg.setColorAt(0.0,   Qt::transparent);
    lg.setColorAt(0.001, Qt::red);
    lg.setColorAt(0.5,   QColor("lightblue"));
    lg.setColorAt(0.999, Qt::red);
    lg.setColorAt(1.0,   Qt::transparent);

    p->fillRect(rect1, lg);
    p->setPen(QPen(Qt::black, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(rect1);
}

static void testPathWinding(QCanvasPainter *p)
{
    p->setFillStyle(Qt::blue);
    p->beginPath();
    p->moveTo(20,20);
    p->lineTo(70, 30);
    p->lineTo(120, 20);
    p->lineTo(120,120);
    p->lineTo(20, 120);
    p->closePath();
    p->rect(60, 60, 100, 100);
    p->fill();

    p->setFillStyle(Qt::red);
    p->beginPath();
    p->rect(200, 50, 200, 100);
    p->rect(250, 100, 200, 100);
    p->fill(QCanvasPainter::FillRule::EvenOdd);
}

static void testPathWinding_reference(QPainter *p)
{
    p->setPen(Qt::NoPen);

    // First shape: winding fill (default for QPainterPath)
    QPainterPath path1;
    path1.setFillRule(Qt::WindingFill);
    path1.moveTo(20, 20);
    path1.lineTo(70, 30);
    path1.lineTo(120, 20);
    path1.lineTo(120, 120);
    path1.lineTo(20, 120);
    path1.closeSubpath();
    path1.addRect(QRectF(60, 60, 100, 100));

    p->setBrush(Qt::blue);
    p->drawPath(path1);

    // Second shape: even-odd fill with two overlapping rects
    QPainterPath path2;
    path2.setFillRule(Qt::OddEvenFill);
    path2.addRect(QRectF(200, 50, 200, 100));
    path2.addRect(QRectF(250, 100, 200, 100));

    p->setBrush(Qt::red);
    p->drawPath(path2);
}

static void testClipRectBug(QCanvasPainter *p)
{
    p->setFillStyle(Qt::red);
    p->fillRect(10, 10, 300, 100);

    p->setClipRect(100, 10, 100, 500);

    p->setFillStyle(Qt::green);
    p->fillRect(10, 150, 300, 100);

    p->resetClipping();

    p->setFillStyle(Qt::blue);
    p->fillRect(10, 300, 300, 100);
}

static void testClipRectBug_reference(QPainter *p)
{
    p->fillRect(10, 10, 300, 100, Qt::red);

    p->setClipRect(100, 10, 100, 500);

    p->fillRect(10, 150, 300, 100, Qt::green);

    p->setClipping(false);

    p->fillRect(10, 300, 300, 100, Qt::blue);
}

static void testSaveRestore(QCanvasPainter *p)
{
    p->setClipRect(0, 0, 150, 500);
    p->beginPath();

    p->ellipse(QRectF(10, 10, 300, 100));

    p->setFillStyle(Qt::red);

    p->fill();

    p->save();
    p->setClipRect(150, 10, 300, 500);

    p->beginPath();
    p->roundRect(10, 150, 300, 100, 20);

    p->setFillStyle(Qt::green);
    p->fill();

    p->restore();

    p->setFillStyle(Qt::blue);
    p->fillRect(10, 300, 300, 100);
}

static void testSaveRestore_reference(QPainter *p)
{
    p->setClipRect(QRectF(0, 0, 150, 500));
    p->setBrush(Qt::red);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QRectF(10, 10, 300, 100));

    p->save();
    p->setClipRect(QRectF(150, 10, 300, 500));
    QPainterPath rr;
    rr.addRoundedRect(QRectF(10, 150, 300, 100), 20, 20);
    p->setBrush(Qt::green);
    p->drawPath(rr);
    p->restore();

    p->setBrush(Qt::blue);
    p->drawRect(QRectF(10, 300, 300, 100));
}

static void testPathFillRule(QCanvasPainter *p)
{
    p->setFillStyle(QColor(255, 0, 0, 100));
    p->setStrokeStyle(Qt::blue);

    p->translate(0, 50);

    p->beginPath();

    p->moveTo(100, 0);
    p->lineTo(60, 100);
    p->lineTo(160, 40);
    p->lineTo(40, 40);
    p->lineTo(140, 100);
    p->lineTo(100, 0);

    p->fill();
    p->stroke();

    p->translate(0,150);

    p->beginPath();

    p->moveTo(100, 0);
    p->lineTo(60, 100);
    p->lineTo(160, 40);
    p->lineTo(40, 40);
    p->lineTo(140, 100);
    p->lineTo(100, 0);
    p->setFillRule(QCanvasPainter::FillRule::EvenOdd);

    p->fill();
    p->stroke();

    p->translate(0,150);
    p->beginPath();

    p->moveTo(100, 0);
    p->lineTo(160, 40);
    p->lineTo(140, 100);
    p->lineTo(60, 100);
    p->lineTo(40, 40);
    p->lineTo(100, 0);

    p->fill();
    p->stroke();
}

static void testPathFillRule_reference(QPainter *p)
{
    p->translate(0, 50);

    QPainterPath star1;
    star1.setFillRule(Qt::WindingFill);
    star1.moveTo(100, 0);
    star1.lineTo(60, 100);
    star1.lineTo(160, 40);
    star1.lineTo(40, 40);
    star1.lineTo(140, 100);
    star1.lineTo(100, 0);

    p->setBrush(QColor(255, 0, 0, 100));
    p->setPen(QPen(Qt::blue, 1));
    p->drawPath(star1);

    p->translate(0, 150);
    star1.setFillRule(Qt::OddEvenFill);
    p->drawPath(star1);

    p->translate(0, 150);

    QPainterPath star2;
    star2.setFillRule(Qt::WindingFill);
    star2.moveTo(100, 0);
    star2.lineTo(160, 40);
    star2.lineTo(140, 100);
    star2.lineTo(60, 100);
    star2.lineTo(40, 40);
    star2.lineTo(100, 0);

    p->drawPath(star2);
}

/* XPM */
static const char *xman[] = {
    /* width height ncolors chars_per_pixel */
    "8 8 3 1",
    /* colors */
    "e g4 black c pale turquoise 4",
    "f m white c light golden rod yellow g4 grey",
    "g g white c lemon chiffon m black",
    /* pixels */
    "eeeeeeee",
    "ffffffff",
    "gggggggg",
    "gggggggg",
    "gggggggg",
    "gggggggg",
    "gggggggg",
    "gggggggg"
};

static void testClipRegion(QCanvasPainter *p)
{
    p->setStrokeStyle(Qt::red);

    QList<QRectF> clipRegion;

    clipRegion << QRectF(100, 100, 200, 100);
    clipRegion << QRectF(50, 10, 150, 50);

    for (const auto &rect : clipRegion) {
        p->strokeRect(rect);
    }
    p->setStencilClip(clipRegion);

    p->setFillStyle(QColor(0,0,255,160));
    p->setStrokeStyle(Qt::green);
    p->beginPath();
    p->ellipse(QRectF(10, 10, 400, 150));

    p->fill();
    p->stroke();

    if (1) { // image
        QImage img(floppy);
        auto ci = p->addImage(img);
        p->drawImage(ci, 200, 150, 200, 200);
    }

    if (1) { // text
        QString s = QStringLiteral("TEXTabcdefghijkl");
        p->setFillStyle(Qt::green);

        QFont f1;
        f1.setPointSize(36);
        p->setFont(f1);
        QRectF r(50, 100, 200, 50);
        p->fillText(s, r);
    }

    if (1) { // non-convex
        p->save();
        p->setFillStyle(QColor(255, 128, 0));
        p->setStrokeStyle(Qt::black);
        p->scale(2);

        p->beginPath();

        p->moveTo(100, 0);
        p->lineTo(60, 100);
        p->lineTo(160, 40);
        p->lineTo(40, 40);
        p->lineTo(140, 100);
        p->lineTo(100, 0);

        p->fill();
        p->restore();
    }

    if (1) { // custom fill
        QImage img(xman);
        auto ci = p->addImage(img, QCanvasPainter::ImageFlag::Repeat);
        QCanvasImagePattern ip(ci);

        p->setFillStyle(ip);

        p->beginPath();
        p->ellipse(QRectF(70, 150, 50, 60));
        p->fill();
    }

    // Clear clip
    p->setStencilClip(QList<QRectF>{});
    p->setFillStyle(Qt::magenta);
    p->beginPath();
    p->ellipse(QRectF(100, 250, 100, 50));
    p->fill();
    p->stroke();
}

static void testClipRegion_reference(QPainter *p)
{
    p->setPen(Qt::red);
    p->setBrush(Qt::NoBrush);

    QList<QRectF> clipRegion;
    clipRegion << QRectF(100, 100, 200, 100);
    clipRegion << QRectF(50, 10, 150, 50);

    for (const auto &rect : clipRegion)
        p->drawRect(rect);

    QRegion region;
    for (const auto &rect : clipRegion)
        region |= rect.toRect();
    p->setClipRegion(region);

    p->setBrush(QColor(0, 0, 255, 160));
    p->setPen(QPen(Qt::green, 1));
    p->drawEllipse(QRectF(10, 10, 400, 150));

    if (1) { // image
        QImage img(floppy);
        p->drawImage(QRectF(200, 150, 200, 200), img);
    }

    if (1) { // text
        QString s = QStringLiteral("TEXTabcdefghijkl");
        p->setPen(Qt::green);
        QFont f1;
        f1.setPointSize(36);
        p->setFont(f1);
        p->drawText(QRectF(50, 100, 200, 50), Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip, s);
    }

    if (1) { // non-convex
        p->save();
        p->setBrush(QColor(255, 128, 0));
        p->setPen(Qt::NoPen);
        p->scale(2, 2);

        QPainterPath star;
        star.moveTo(100, 0);
        star.lineTo(60, 100);
        star.lineTo(160, 40);
        star.lineTo(40, 40);
        star.lineTo(140, 100);
        star.lineTo(100, 0);
        star.setFillRule(Qt::WindingFill); // default for QCanvasPainter
        p->drawPath(star);
        p->restore();
    }

    if (1) { // custom fill (xman image pattern)
        QImage img(xman);
        QBrush brush(img);
        p->save();
        p->setBrush(brush);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QRectF(70, 150, 50, 60));
        p->restore();
    }

    // Clear clip
    p->setClipping(false);
    p->setBrush(Qt::magenta);
    p->drawEllipse(QRectF(100, 250, 100, 50));
}

static void testClipRegionTransform(QCanvasPainter *p)
{
    p->setStrokeStyle(Qt::red);
    p->rotate(M_PI/180 * 15);

    QList<QRectF> clipRegion{{100, 100, 200, 100}};
    //clipRegion << QRectF(50, 10, 150, 50);

    for (const auto &rect : clipRegion) {
        p->strokeRect(float(rect.x()),
                      float(rect.y()),
                      float(rect.width()),
                      float(rect.height()));
    }
    p->setStencilClip(clipRegion);

    p->rotate(-M_PI/180 * 15);
    p->setFillStyle(QColor(255, 255, 0, 192));
    p->fillRect(0, 0, 500, 400);
}

static void testClipRegionTransform_reference(QPainter *p)
{
    p->setPen(Qt::red);
    p->setBrush(Qt::NoBrush);
    p->rotate(180.0 / M_PI * (M_PI / 180 * 15));  // 15 degrees

    QRectF rect(100, 100, 200, 100);
    p->drawRect(rect);

    p->setClipRegion(QRegion(rect.toRect()));

    p->rotate(-(180.0 / M_PI * (M_PI / 180 * 15)));
    p->setBrush(QColor(255, 255, 0, 192));
    p->setPen(Qt::NoPen);
    p->drawRect(0, 0, 500, 400);
}

static void testClipRegionIntersect(QCanvasPainter *p)
{
    p->setStrokeStyle(Qt::red);

    QRectF rect(50, 50, 200, 100);

    p->strokeRect(rect);
    p->save();
    p->rotate(M_PI/180 * 15);
    p->strokeRect(rect);
    p->restore();

    p->setStencilClip({rect});

    p->setFillStyle(QColor(255, 255, 0, 192)); // Yellow in unrotated
    p->fillRect(0, 0, 500, 400);

    p->rotate(M_PI/180 * 15);

    p->setStencilClip({rect});

    p->rotate(-M_PI/180 * 15);
    p->setFillStyle(QColor(0, 0, 255, 128)); // blue in intersection
    p->fillRect(0, 0, 500, 400);
}

static void testClipRegionIntersect_reference(QPainter *p)
{
    QRectF rect(50, 50, 200, 100);

    p->setPen(Qt::red);
    p->setBrush(Qt::NoBrush);
    p->drawRect(rect);
    p->save();
    p->rotate(15.0);
    p->drawRect(rect);
    p->restore();

    p->setClipRegion(QRegion(rect.toRect()));
    p->setBrush(QColor(255, 255, 0, 192));
    p->setPen(Qt::NoPen);
    p->drawRect(0, 0, 500, 400);

    p->rotate(15.0);
    // Intersect with the rotated rect — use IntersectClip to combine
    p->setClipRegion(QRegion(rect.toRect()), Qt::IntersectClip);

    p->rotate(-15.0);
    p->setBrush(QColor(0, 0, 255, 128));
    p->drawRect(0, 0, 500, 400);
}

static void testStroking(QCanvasPainter *p)
{
    p->save();
    p->setStrokeStyle(Qt::darkGreen);
    p->setLineWidth(7);
    p->beginPath();
    p->ellipse(QRectF(10, 10, 400, 100));
    p->stroke();

    p->setStrokeStyle(QColor(192, 0, 0, 127));
    p->translate(0, 150);

    p->beginPath();
    p->moveTo(100, 0);
    p->lineTo(60, 100);
    p->lineTo(160, 40);
    p->lineTo(40, 40);
    p->lineTo(140, 100);
    p->lineTo(100, 0);

    p->stroke();
    p->restore();
}

static void testStroking_reference(QPainter *p)
{
    p->save();
    p->setPen(QPen(Qt::darkGreen, 7));
    p->setBrush(Qt::NoBrush);
    p->drawEllipse(QRectF(10, 10, 400, 100));

    p->setPen(QPen(QColor(192, 0, 0, 127), 7));
    p->translate(0, 150);

    QPainterPath star;
    star.moveTo(100, 0);
    star.lineTo(60, 100);
    star.lineTo(160, 40);
    star.lineTo(40, 40);
    star.lineTo(140, 100);
    star.lineTo(100, 0);

    p->drawPath(star);
    p->restore();
}

static void testClipStroking(QCanvasPainter *p)
{
    p->setStrokeStyle(Qt::red);
    QList<QRectF> clipRegion{{100, 100, 200, 150}};
    clipRegion += QRectF(50, 10, 150, 50);

    for (const auto &rect : clipRegion) {
        p->strokeRect(float(rect.x()),
                      float(rect.y()),
                      float(rect.width()),
                      float(rect.height()));
    }
    p->setStencilClip(clipRegion);

    testStroking(p);

    p->setFillStyle(QColor(255, 255, 0, 192));
    p->fillRect(150, 0, 50, 400);
}

static void testClipHighQualityStroking(QCanvasPainter *p)
{
    p->setHighQualityStroking(true);
    testClipStroking(p);
}

static void testClipStroking_reference(QPainter *p)
{
    QList<QRectF> clipRegion{{100, 100, 200, 150}};
    clipRegion += QRectF(50, 10, 150, 50);

    p->setPen(Qt::red);
    p->setBrush(Qt::NoBrush);
    for (const auto &rect : clipRegion)
        p->drawRect(rect);

    QRegion region;
    for (const auto &rect : clipRegion)
        region |= rect.toRect();
    p->setClipRegion(region);

    testStroking_reference(p);

    p->fillRect(QRectF(150, 0, 50, 400), QColor(255, 255, 0, 192));
}

static void testVectorPath(QCanvasPainter *p)
{
    QRectF clipRegionRect{50, 50, 400, 150};

    const int count = 14;
    qreal points[count * 2] = {
        10, 10,
        10, 200,
        190, 200,
        190, 10,
        10, 10,

        250, 10,
        150, 160,
        350, 50,
        350, 10,
        250, 10,

        60, 180,
        60, 160,
        80, 170,
        60, 180
    };
    QPainterPath::ElementType elements[count] = {
        QPainterPath::MoveToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,

        QPainterPath::MoveToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,

        QPainterPath::MoveToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
    };

    QVectorPath::Hint hints = QVectorPath::WindingFill;
    QVectorPath path(points, count, elements, hints);

    p->strokeRect(clipRegionRect);

    p->setStencilClip({clipRegionRect});
    p->setStencilClip(path);

    p->setFillStyle(Qt::red);
    p->fillRect(QRectF(0, 0, 500, 500));

    p->setStencilClip({});
    p->beginPath();
    p->addPath(path.convertToPainterPath());
    p->setStrokeStyle(Qt::blue);
    p->stroke();
}

static void testVectorPath_reference(QPainter *p)
{
    QRectF clipRegionRect{50, 50, 400, 150};

    const int count = 14;
    qreal points[count * 2] = {
        10, 10,
        10, 200,
        190, 200,
        190, 10,
        10, 10,

        250, 10,
        150, 160,
        350, 50,
        350, 10,
        250, 10,

        60, 180,
        60, 160,
        80, 170,
        60, 180
    };
    QPainterPath::ElementType elements[count] = {
        QPainterPath::MoveToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,

        QPainterPath::MoveToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,

        QPainterPath::MoveToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
        QPainterPath::LineToElement,
    };

    QVectorPath::Hint hints = QVectorPath::WindingFill;
    QVectorPath vectorPath(points, count, elements, hints);
    QPainterPath painterPath = vectorPath.convertToPainterPath();

    p->drawRect(clipRegionRect);

    p->setClipRect(clipRegionRect);
    p->setClipPath(painterPath, Qt::IntersectClip);

    p->fillRect(QRectF(0, 0, 500, 500), Qt::red);

    p->setClipping(false);
    p->setPen(Qt::blue);
    p->drawPath(painterPath);
}

static void testHighQualityStroking(QCanvasPainter *p)
{
    p->setHighQualityStroking(true);
    testStroking(p);
}

static void simpleTest(QCanvasPainter *p)
{
    p->setFillStyle(Qt::red);
    p->fillRect(100,100,100,100);
}

static void simpleTest_reference(QPainter *p)
{
    p->fillRect(100,100,100,100, Qt::red);
}

static void testSetTransform(QCanvasPainter *p)
{
    p->setFillStyle(Qt::red);
    p->translate(100, 100);
    p->strokeRect(0, 0, 100, 100);

    QTransform t;
    t.rotate(-15);
    p->transform(t);
    p->fillRect(0,0,100,100);

    p->setFillStyle(Qt::green);
    p->transform(t);
    p->fillRect(0,0,100,100);
}

static void testSetTransform_reference(QPainter *p)
{
    p->translate(100, 100);
    p->drawRect(0, 0, 100, 100);

    QTransform t;
    t.rotate(-15);
    p->setTransform(t, true);
    p->fillRect(0,0,100,100, Qt::red);

    p->setTransform(t, true);
    p->fillRect(0,0,100,100, Qt::green);
}

static void testMixedQualityStroking(QCanvasPainter *p)
{
    p->setHighQualityStroking(false);
    p->setStrokeStyle(QColor(192, 0, 0, 127));
    p->setLineWidth(20);

    p->beginPath();
    p->moveTo(10, 10);
    p->lineTo(80, 80);
    p->moveTo(10, 80);
    p->lineTo(80, 10);
    p->stroke();

    p->setFillStyle(QColor(0, 192, 0, 127));
    p->fillRect(10, 110, 80, 80);

    p->setHighQualityStroking(true);
    p->beginPath();
    p->moveTo(10, 210);
    p->lineTo(80, 280);
    p->moveTo(10, 280);
    p->lineTo(80, 210);
    p->stroke();
}

static void testMixedQualityStroking_reference(QPainter *p)
{
    QColor penColor(192, 0, 0, 127);
    QPen pen(penColor, 20);
    pen.setCapStyle(Qt::FlatCap);
    p->setPen(pen);

    QPainterPath cross;
    cross.moveTo(10, 10);
    cross.lineTo(80, 80);
    cross.moveTo(10, 80);
    cross.lineTo(80, 10);
    p->drawPath(cross);

    p->fillRect(10, 110, 80, 80, QColor(0, 192, 0, 127));

    cross.translate(0, 200);
    QPainterPathStroker stroker;
    stroker.setWidth(20);
    stroker.setCapStyle(Qt::FlatCap);
    QPainterPath stroke = stroker.createStroke(cross);
    stroke.setFillRule(Qt::WindingFill);
    p->setBrush(penColor);
    p->setPen(Qt::NoPen);
    p->drawPath(stroke);
}

static TestDescription tests[] {
    {"Simple",                   simpleTest,                simpleTest_reference},
    {"Gradients",                testGradients,             testGradients_reference},
    {"Pen",                      testPen,                   testPen_reference},
    {"Image",                    testImage,                 testImage_reference},
    {"Text",                     testText,                  testText_reference},
    {"Paths",                    testPath,                  testPath_reference},
    {"QPainterPath",             testPainterPath,           testPainterPath_reference},
    {"Antialiasing",             testAntialiasing,          testAntialiasing_reference},
    {"Transforms",               testTransform,             testTransform_reference},
    {"setTransform",             testSetTransform,          testSetTransform_reference},
    {"Clipping",                 testClip,                  testClip_reference},
    {"Gradient Cache Bug",       testGradientBug,           testGradientBug_reference},
    {"Gradient Pad Bug",         testGradientPad,           testGradientPad_reference},
    {"Path Winding",             testPathWinding,           testPathWinding_reference},
    {"Clip Rect Bug",            testClipRectBug,           testClipRectBug_reference},
    {"Save/Restore",             testSaveRestore,           testSaveRestore_reference},
    {"Path Fill Rule",           testPathFillRule,          testPathFillRule_reference},
    {"Clip Region",              testClipRegion,            testClipRegion_reference},
    {"Clip Region Transform",    testClipRegionTransform,   testClipRegionTransform_reference},
    {"Clip Region Intersect",    testClipRegionIntersect,   testClipRegionIntersect_reference},
    {"Normal Quality Stroking",  testStroking,              testStroking_reference},
    {"HighQualityStroking",      testHighQualityStroking,   testStroking_reference}, // same reference as normal quality
    {"Clip Normal Quality Stroking",  testClipStroking,         testClipStroking_reference},
    {"Clip HighQualityStroking",      testClipHighQualityStroking, testClipStroking_reference}, // same reference as normal quality
    {"Clip Vector Path",              testVectorPath,           testVectorPath_reference},
    {"High and Normal Quality Stroking", testMixedQualityStroking, testMixedQualityStroking_reference},
};

class MyWidget : public QCanvasPainterWidget
{
public:
    MyWidget(int t)
        :test(t)
    {
        setFillColor("#ffffff");
        setWindowTitle(QStringLiteral("QCanvasPainter test - ") + tests[test].name);
        qDebug() << "Testing" << tests[test].name;
    }

    void paint(QCanvasPainter *p) override
    {
        tests[test].cFunc(p);
    }
private:
    int test = 0;
};

class ReferenceWidget : public QWidget
{
public:
    ReferenceWidget(int t)
        : test(t)
    {
        setWindowTitle("QPainter reference");
    }

    bool hasContent() const
    {
        return !!tests[test].qFunc;
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), Qt::white);
        if (tests[test].qFunc)
            tests[test].qFunc(&p);
        else
            p.drawText(10, 100, QLatin1String("No QPainter reference available"));
    }

private:
    int test = 0;
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QRhiWidget::Api graphicsApi;

    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription("Qt Canvas Painter rendering test");
    cmdLineParser.addHelpOption();

    QCommandLineOption glOption({ "g", "opengl" }, QLatin1String("OpenGL"));
    cmdLineParser.addOption(glOption);
    QCommandLineOption vkOption({ "v", "vulkan" }, QLatin1String("Vulkan"));
    cmdLineParser.addOption(vkOption);
    QCommandLineOption d3d11Option({ "d", "d3d11" }, QLatin1String("Direct3D 11"));
    cmdLineParser.addOption(d3d11Option);
    QCommandLineOption d3d12Option({ "D", "d3d12" }, QLatin1String("Direct3D 12"));
    cmdLineParser.addOption(d3d12Option);
    QCommandLineOption mtlOption({ "m", "metal" }, QLatin1String("Metal"));
    cmdLineParser.addOption(mtlOption);

    QCommandLineOption listOption({ "l", "list-tests" }, QLatin1String("Show all tests"));
    cmdLineParser.addOption(listOption);

    QCommandLineOption referenceOption({ "r", "reference" }, QLatin1String("Show QPainter reference"));
    cmdLineParser.addOption(referenceOption);

    cmdLineParser.addPositionalArgument( QLatin1String("test"), QLatin1String("Number of test to run"));

    cmdLineParser.process(a);

    constexpr int nTests = sizeof(tests) / sizeof(TestDescription);
    int test = -1;
    bool testOK = false;
    bool listOptionSet = cmdLineParser.isSet(listOption);
    if (cmdLineParser.positionalArguments().size() > 0)
        test = cmdLineParser.positionalArguments().first().toInt(&testOK);

    if (!testOK && !listOptionSet)
        cmdLineParser.showHelp(-1);

    if (listOptionSet || !testOK || test >= nTests || test < 0) {
        qInfo() << "Available tests:";
        for (int i = 0; i < nTests; ++i)
            qInfo().noquote() << "   " << i << tests[i].name << (tests[i].qFunc ? "" : " [reference not available]");
        return 0;
    }

    // Defaults.
#if defined(Q_OS_WIN)
    graphicsApi = QRhiWidget::Api::Direct3D11;
#elif QT_CONFIG(metal)
    graphicsApi = QRhiWidget::Api::Metal;
#else
    graphicsApi = QRhiWidget::Api::OpenGL;
#endif

    if (cmdLineParser.isSet(glOption))
        graphicsApi = QRhiWidget::Api::OpenGL;
    if (cmdLineParser.isSet(vkOption))
        graphicsApi = QRhiWidget::Api::Vulkan;
    if (cmdLineParser.isSet(d3d11Option))
        graphicsApi = QRhiWidget::Api::Direct3D11;
    if (cmdLineParser.isSet(d3d12Option))
        graphicsApi = QRhiWidget::Api::Direct3D12;
    if (cmdLineParser.isSet(mtlOption))
        graphicsApi = QRhiWidget::Api::Metal;

    MyWidget w(test);
    w.setApi(graphicsApi);
    qDebug() << "API" << w.api();
    w.show();

    ReferenceWidget rw(test);
    if (cmdLineParser.isSet(referenceOption)) {
        if (rw.hasContent())
            rw.show();
        else
            qWarning() << "No QPainter reference for" << tests[test].name;
    }
    return a.exec();
}
