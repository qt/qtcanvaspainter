// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DCOMMANDBUFFER_P_H
#define QCANVAS2DCOMMANDBUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtcanvas2dglobal_p.h"
#include "qcanvas2dcontext_p.h"

QT_BEGIN_NAMESPACE

class Q_CANVAS2D_EXPORT QCanvas2DCommandBuffer
{
public:
    QCanvas2DCommandBuffer();
    ~QCanvas2DCommandBuffer();

    void clearBuffers();

    inline void setGlobalAlpha(qreal alpha)
    {
        commands << QCanvas2DContext::GlobalAlpha;
        reals << alpha;
    }

    inline void setGlobalBrightness(qreal brightness)
    {
        commands << QCanvas2DContext::GlobalBrightness;
        reals << brightness;
    }

    inline void setGlobalContrast(qreal contrast)
    {
        commands << QCanvas2DContext::GlobalContrast;
        reals << contrast;
    }

    inline void setGlobalSaturate(qreal saturate)
    {
        commands << QCanvas2DContext::GlobalSaturate;
        reals << saturate;
    }

    inline void setGlobalCompositeOperation(QCanvasPainter::CompositeOperation co)
    {
        commands << QCanvas2DContext::GlobalCompositeOperation;
        ints << int(co);
    }

    inline void setStrokeStyle(const QCanvasBrush &style)
    {
        commands << QCanvas2DContext::StrokeStyle;
        brushes << style;
    }

    inline void setStrokeColor(const QColor &color)
    {
        commands << QCanvas2DContext::StrokeColor;
        colors << color;
    }

    inline void drawPixmap(QQmlRefPointer<QCanvas2DPixmap> pixmap, const QString &filename, const QRectF &sr, const QRectF &dr)
    {
        commands << QCanvas2DContext::DrawPixmap;
        strings << filename;
        pixmaps << pixmap;
        rects << sr << dr;
    }

    inline void scale(qreal x, qreal y)
    {
        commands << QCanvas2DContext::Scale;
        reals << x;
        reals << y;
    }

    inline void rotate(qreal angle)
    {
        commands << QCanvas2DContext::Rotate;
        reals << angle;
    }

    inline void shear(qreal h, qreal v)
    {
        commands << QCanvas2DContext::Shear;
        reals << h;
        reals << v;
    }

    inline void translate(qreal x, qreal y)
    {
        commands << QCanvas2DContext::Translate;
        reals << x;
        reals << y;
    }

    inline void transform(const QTransform &transform)
    {
        commands << QCanvas2DContext::Transform;
        matrixes << transform;
    }

    inline void setTransform(const QTransform &transform)
    {
        commands << QCanvas2DContext::SetTransform;
        matrixes << transform;
    }

    inline void beginPath()
    {
        commands << QCanvas2DContext::BeginPath;
    }

    inline void beginSolidSubPath()
    {
        commands << QCanvas2DContext::BeginSolidSubPath;
    }

    inline void beginHoleSubPath()
    {
        commands << QCanvas2DContext::BeginHoleSubPath;
    }

    inline void closePath()
    {
        commands << QCanvas2DContext::ClosePath;
    }

    inline void moveTo(qreal x, qreal y)
    {
        commands << QCanvas2DContext::MoveTo;
        reals << x;
        reals << y;
    }

    inline void lineTo(qreal x, qreal y)
    {
        commands << QCanvas2DContext::LineTo;
        reals << x;
        reals << y;
    }

    inline void quadraticCurveTo(qreal cpx, qreal cpy, qreal x, qreal y)
    {
        commands << QCanvas2DContext::QuadraticCurveTo;
        reals << cpx;
        reals << cpy;
        reals << x;
        reals << y;
    }

    inline void bezierCurveTo(qreal cp1x, qreal cp1y, qreal cp2x, qreal cp2y, qreal x, qreal y)
    {
        commands << QCanvas2DContext::BezierCurveTo;
        reals << cp1x;
        reals << cp1y;
        reals << cp2x;
        reals << cp2y;
        reals << x;
        reals << y;
    }

    inline void arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal radius)
    {
        commands << QCanvas2DContext::ArcTo;
        reals << x1;
        reals << y1;
        reals << x2;
        reals << y2;
        reals << radius;
    }

    inline void rect(qreal x, qreal y, qreal w, qreal h)
    {
        commands << QCanvas2DContext::Rect;
        reals << x;
        reals << y;
        reals << w;
        reals << h;
    }

    inline void roundRect(qreal x, qreal y, qreal w, qreal h,
                          qreal radiusTopLeft, qreal radiusTopRight,
                          qreal radiusBottomRight, qreal radiusBottomLeft)
    {
        commands << QCanvas2DContext::RoundRect;
        reals << x;
        reals << y;
        reals << w;
        reals << h;
        reals << radiusTopLeft;
        reals << radiusTopRight;
        reals << radiusBottomRight;
        reals << radiusBottomLeft;
    }

    inline void ellipse(qreal cX, qreal cY, qreal rX, qreal rY)
    {
        commands << QCanvas2DContext::Ellipse;
        reals << cX;
        reals << cY;
        reals << rX;
        reals << rY;
    }

    inline void ellipseRect(qreal x, qreal y, qreal w, qreal h)
    {
        commands << QCanvas2DContext::EllipseRect;
        reals << x;
        reals << y;
        reals << w;
        reals << h;
    }

    inline void circle(qreal cX, qreal cY, qreal r)
    {
        commands << QCanvas2DContext::Circle;
        reals << cX;
        reals << cY;
        reals << r;
    }

    inline void arc(qreal xc, qreal yc, qreal radius, qreal sar, qreal ear, bool antiClockWise)
    {
        commands << QCanvas2DContext::Arc;
        reals << xc;
        reals << yc;
        reals << radius;
        reals << sar;
        reals << ear;
        bools << antiClockWise;
    }

    inline void clearRect(const QRectF &r)
    {
        commands << QCanvas2DContext::ClearRect;
        rects << r;
    }

    inline void fillRect(const QRectF &r)
    {
        commands << QCanvas2DContext::FillRect;
        rects << r;
    }

    inline void strokeRect(const QRectF &r)
    {
        commands << QCanvas2DContext::StrokeRect;
        rects << r;
    }


    inline void fillPath(const QPainterPath &path)
    {
        commands << QCanvas2DContext::FillPath;
        paths << path;
    }
    inline void fillPath(const QCanvasPath &path, int pathGroup)
    {
        commands << QCanvas2DContext::FillCanvasPath;
        canvasPaths << path;
        ints << pathGroup;
    }
    inline void fill()
    {
        commands << QCanvas2DContext::Fill;
    }

    inline void strokePath(const QPainterPath &path)
    {
        commands << QCanvas2DContext::StrokePath;
        paths << path;
    }
    inline void strokePath(const QCanvasPath &path, int pathGroup)
    {
        commands << QCanvas2DContext::StrokeCanvasPath;
        canvasPaths << path;
        ints << pathGroup;
    }
    inline void stroke()
    {
        commands << QCanvas2DContext::Stroke;
    }

    inline void addPath(const QCanvasPath &path, const QTransform &transform)
    {
        commands << QCanvas2DContext::AddCanvasPath;
        canvasPaths << path;
        matrixes << transform;
    }

    inline void addPath(const QCanvasPath &path, int start, int count, const QTransform &transform)
    {
        commands << QCanvas2DContext::AddCanvasPathRange;
        canvasPaths << path;
        ints << start << count;
        matrixes << transform;
    }

    inline void clip(bool enabled, const QPainterPath &path)
    {
        commands << QCanvas2DContext::Clip;
        bools << enabled;
        paths << path;
    }
    inline void clipRect(const QRectF &r)
    {
        commands << QCanvas2DContext::ClipRect;
        rects << r;
    }
    inline void resetClipping()
    {
        commands << QCanvas2DContext::ResetClipping;
    }
    inline void setFillStyle(const QCanvasBrush &style)
    {
        commands << QCanvas2DContext::FillStyle;
        brushes << style;
    }

    inline void setFillColor(const QColor &color)
    {
        commands << QCanvas2DContext::FillColor;
        colors << color;
    }

    inline void setLineWidth(qreal w)
    {
        commands << QCanvas2DContext::LineWidth;
        reals << w;
    }

    inline void setAntialias(qreal w)
    {
        commands << QCanvas2DContext::Antialias;
        reals << w;
    }

    inline void setLineCap(QCanvasPainter::LineCap cap)
    {
        commands << QCanvas2DContext::LineCap;
        ints << int(cap);
    }

    inline void setLineJoin(QCanvasPainter::LineJoin join)
    {
        commands << QCanvas2DContext::LineJoin;
        ints << int(join);
    }

    inline void setMiterLimit(qreal limit)
    {
        commands << QCanvas2DContext::MiterLimit;
        reals << limit;
    }

    inline void setFont(const QFont &font)
    {
        commands << QCanvas2DContext::Font;
        fonts << font;
    }

    inline void drawText(const QString &text, qreal x, qreal y)
    {
        commands << QCanvas2DContext::FillText;
        strings << text;
        reals << x << y;
    }

    inline void drawBoxShadow(QCanvasBoxShadow *shadow)
    {
        commands << QCanvas2DContext::DrawBoxShadow;
        brushes << QCanvasBrush(*shadow);
    }

    inline void setTextAlign(QCanvasPainter::TextAlign ta)
    {
        commands << QCanvas2DContext::TextAlign;
        ints << int(ta);
    }
    inline void setTextBaseline(QCanvasPainter::TextBaseline tb)
    {
        commands << QCanvas2DContext::TextBaseline;
        ints << int(tb);
    }
    inline void save()
    {
        commands << QCanvas2DContext::Save;
    }
    inline void restore()
    {
        commands << QCanvas2DContext::Restore;
    }
    inline void reset()
    {
        commands << QCanvas2DContext::Reset;
    }

private:
    friend class QCanvas2DItemRenderer;

    QList<QCanvas2DContext::PaintCommand> commands;
    QList<int> ints;
    QList<bool> bools;
    QList<qreal> reals;
    QList<QRectF> rects;
    QList<QColor> colors;
    QList<QTransform> matrixes;
    QList<QCanvasBrush> brushes;
    QList<QPainterPath> paths;
    QList<QCanvasPath> canvasPaths;
    QList<QImage> images;
    QList<QFont> fonts;
    QList<QQmlRefPointer<QCanvas2DPixmap> > pixmaps;
    QList<QString> strings;
};

QT_END_NAMESPACE

#endif // QCANVAS2DCOMMANDBUFFER_P_H
