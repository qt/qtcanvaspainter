// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DCONTEXT_P_H
#define QCANVAS2DCONTEXT_P_H

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
#include "qcanvas2ditem_p.h"

#include <QtCanvasPainter/qcanvaspainter.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtCanvasPainter/qcanvaspath.h>
#include <QtCanvasPainter/qcanvasboxshadow.h>

#include <QtQml/qqml.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/private/qv4persistent_p.h>

#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>

#include <QtCore/qstring.h>
#include <QtCore/qstack.h>
#include <QtCore/qqueue.h>


QT_BEGIN_NAMESPACE

namespace QV4 {
    struct ExecutionEngine;
}

class QCanvas2DCommandBuffer;
class QQuickPixmap;
class QSGTexture;

class Q_CANVAS2D_EXPORT QCanvas2DContext : public QObject
{
    Q_OBJECT

public:
    Q_DISABLE_COPY(QCanvas2DContext)

    enum PaintCommand : quint8 {
        BeginPath,
        BeginSolidSubPath,
        BeginHoleSubPath,
        ClosePath,
        MoveTo,
        LineTo,
        QuadraticCurveTo,
        BezierCurveTo,
        ArcTo,
        Rect,
        RoundRect,
        EllipseRect,
        Ellipse,
        Circle,
        Arc,
        FillRect,
        StrokeRect,
        ClearRect,
        StrokeColor,
        FillColor,
        StrokeStyle,
        FillStyle,
        Fill,
        FillPath,
        FillCanvasPath,
        Stroke,
        StrokePath,
        StrokeCanvasPath,
        AddCanvasPath,
        Clip,
        ClipRect,
        ResetClipping,
        GlobalAlpha,
        GlobalBrightness,
        GlobalContrast,
        GlobalSaturate,
        GlobalCompositeOperation,
        LineWidth,
        LineCap,
        LineJoin,
        MiterLimit,
        TextAlign,
        TextBaseline,
        Font,
        FillText,
        DrawPixmap,
        Scale,
        Rotate,
        Shear,
        Translate,
        Transform,
        SetTransform,
        Save,
        Restore,
        Reset,
        DrawImage, // TODO: Used in putImageData, check support.
        DrawBoxShadow,
        Antialias,
    };

    struct State {
        State()
            : strokeColor(QColor(Qt::black))
            , fillColor(QColor(Qt::black))
            , clip(false)
            , fillRule(Qt::WindingFill)
            , globalAlpha(1.0)
            , globalBrightness(1.0)
            , globalContrast(1.0)
            , globalSaturate(1.0)
            , lineWidth(1)
            , antialias(1)
            , lineCap(QCanvasPainter::LineCap::Butt)
            , lineJoin(QCanvasPainter::LineJoin::Miter)
            , miterLimit(10)
            , globalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver)
            , font(QFont(QStringLiteral("sans-serif")))
            , textAlign(QCanvasPainter::TextAlign::Start)
            , textBaseline(QCanvasPainter::TextBaseline::Alphabetic)
        {
            font.setPixelSize(10);
        }

        QPainterPath clipPath;
        QCanvasBrush strokeStyle;
        QCanvasBrush fillStyle;
        QTransform transform;
        QColor strokeColor;
        QColor fillColor;
        bool clip:1;
        Qt::FillRule fillRule;
        qreal globalAlpha;
        qreal globalBrightness;
        qreal globalContrast;
        qreal globalSaturate;
        qreal lineWidth;
        qreal antialias;
        QCanvasPainter::LineCap lineCap;
        QCanvasPainter::LineJoin lineJoin;
        qreal miterLimit;
        QCanvasPainter::CompositeOperation globalCompositeOperation;
        QFont font;
        QCanvasPainter::TextAlign textAlign;
        QCanvasPainter::TextBaseline textBaseline;
    };

    QCanvas2DContext(QObject *parent = nullptr);
    ~QCanvas2DContext();

    QStringList contextNames() const;
    void init(QCanvas2DItem *canvasItem, const QVariantMap &args);
    void flush();
    void sync();

    QV4::ReturnedValue v4value() const;
    QV4::ExecutionEngine *v4Engine() const;
    void setV4Engine(QV4::ExecutionEngine *eng);

    QCanvas2DItem* canvas() const { return m_canvas; }
    QCanvas2DCommandBuffer* buffer() const { return m_buffer; }

    bool bufferValid() const { return m_buffer != nullptr; }
    void popState();
    void pushState();
    void reset();

    // ***** transformations *****
    void rotate(qreal angle);
    void scale(qreal x,  qreal y);
    void shear(qreal h, qreal v);
    void translate(qreal x, qreal y);
    void transform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);
    void setTransform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);

    // ***** direct rect methods *****
    void clearRect(qreal x, qreal y, qreal w, qreal h);
    void fillRect(qreal x, qreal y, qreal w, qreal h);
    void strokeRect(qreal x, qreal y, qreal w, qreal h);

    // ***** path handling *****
    void beginPath();
    void clip();
    void clipRect(qreal x, qreal y, qreal w, qreal h);
    void resetClipping();
    void fill();
    void stroke();
    void fillPath(const QCanvasPath &path, int pathGroup = -1);
    void strokePath(const QCanvasPath &path, int pathGroup = -1);

    // ***** other *****
    void drawText(const QString& text, qreal x, qreal y, bool fill);
    void drawBoxShadow(QCanvasBoxShadow *shadow);
    QPainterPath createTextGlyphs(qreal x, qreal y, const QString& text);
    QQmlRefPointer<QCanvas2DPixmap> createPixmap(const QUrl& url, QSizeF sourceSize = QSizeF());
    void setGrabbedImage(const QImage& grab);

    State state;
    QStack<QCanvas2DContext::State> m_stateStack;
    QCanvas2DItem *m_canvas = nullptr;
    QCanvas2DCommandBuffer *m_buffer = nullptr;
    QPainterPath m_path; // TODO: Remove or make QCanvasPath?
    QV4::PersistentValue m_fillStyle;
    QV4::PersistentValue m_strokeStyle;
    QV4::PersistentValue m_v4path;
    QV4::ExecutionEngine *m_v4engine;
    QV4::PersistentValue m_v4value;
    QImage m_grabbedImage;
    bool m_grabbed:1;
};

QT_END_NAMESPACE

#endif // QCANVAS2DCONTEXT_P_H
