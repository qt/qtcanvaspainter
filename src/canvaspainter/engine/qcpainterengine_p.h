// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCPAINTERENGINE_P_H
#define QCPAINTERENGINE_P_H

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

#include <QtGui/qcolor.h>
#include <QtGui/qtransform.h>
#include <QtGui/qpainterpath.h>
#include <QtCore/qrect.h>
#include <QtCore/qlist.h>
#include "qcpainter.h"
#include "qcpainterpath.h"
#include "qcpainterengineutils_p.h"

QT_BEGIN_NAMESPACE

class QCPainterRhiRenderer;

class QCPainterEngine
{
public:
    enum QCPointFlags : quint8 {
        PointCorner = 1 << 0,
        PointLeft = 1 << 1,
        PointBevel = 1 << 2,
        PointInnerBevel = 1 << 3,
    };

    QCPainterEngine();

    // State
    void beginPaint(float logicalWidth, float logicalHeight, float dpr = 1.0f);
    void endPaint();
    void save();
    void restore();
    void reset();

    // Painting styles
    void setStrokeColor(const QColor &color);
    void setStrokePaint(const QCPaint &paint);
    void setFillColor(const QColor &color);
    void setFillPaint(const QCPaint &paint);
    void setLineWidth(float width);
    void setLineCap(QCPainter::LineCap lineCap);
    void setLineJoin(QCPainter::LineJoin lineJoin);
    void setGlobalAlpha(float alpha);
    void setGlobalBrightness(float value);
    void setGlobalContrast(float value);
    void setGlobalSaturate(float value);

    // Custom paints
    void setCustomStrokeBrush(QCCustomBrush *brush);
    void setCustomFillBrush(QCCustomBrush *brush);

    // Transform
    QTransform currentTransform() const;
    void transform(const QTransform &transform);
    void setTransform(const QTransform &transform);
    void resetTransform();
    void translate(float x, float y);
    void scale(float x, float y);
    void rotate(float angle);
    void skew(float angleX, float angleY = 0.0f);

    void setBrushTransform(const QTransform &transform);

    // Images
    int createImage(int width, int height,
                    QCPainter::ImageFlags flags,
                    const uchar* data);
    bool deleteImage(int imageId);
    void drawImageId(int imageId, float x, float y, float width, float height,
                     const QColor &tintColor);
    QCPaint createImagePattern(float x, float y, float width, float height,
                               int imageId, float angle, const QColor &tintColor);

    // Paths
    void beginPath();
    void closePath();
    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void bezierTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y);
    void quadTo(float cpX, float cpY, float x, float y);
    void arcTo(float x1, float y1, float x2, float y2, float radius);
    void addArc(float x, float y, float radius,
                float a0, float a1, QCPainter::PathWinding direction,
                bool isConnected);
    void addRect(float x, float y, float width, float height);
    void addRoundRect(float x, float y, float width, float height, float radius);
    void addRoundRect(float x, float y, float width, float height,
                      float radiusTopLeft, float radiusTopRight,
                      float radiusBottomRight, float radiusBottomLeft);
    // TODO: Have also startAngle, endAngle and direction (winding)?
    // See https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/ellipse
    void addEllipse(float x, float y, float radiusX, float radiusY);
    void addCircle(float x, float y, float radius);
    void addPath(const QPainterPath &path);
    void addPath(const QCPainterPath &path, const QTransform &transform = QTransform());
    void addPath(const QCPainterPath &path, qsizetype start, qsizetype count, const QTransform &transform = QTransform());
    void setPathWinding(QCPainter::PathWinding winding);
    void fill();
    void fillForClear();
    void stroke();
    void fill(const QCPainterPath &path, int pathGroup);
    void stroke(const QCPainterPath &path, int pathGroup);

    // Blending
    void setGlobalCompositeOperation(QCPainter::CompositeOperation op);

    // Clipping
    void resetClipRect();
    void setClipRect(const QRectF &rect);

    // Text
    void setTextWrapMode(QCPainter::WrapMode wrapMode);
    void setTextLineHeight(float height);
    void setTextAntialias(float antialias);
    void setTextAlignment(QCPainter::TextAlign align);
    void setTextBaseline(QCPainter::TextBaseline baseline);
    void setTextDirection(QCPainter::TextDirection direction);
    void fillText(const QString &text, const QRectF &rect);
    QRectF textBoundingBox(const QString &text, const QRectF &rect);

    // Other
    QCDrawDebug drawDebug() const;
    void setAntialias(float antialias);
    void setMiterLimit(float limit);
    void removePathGroup(int pathGroup);
    void setRenderHints(QCPainter::RenderHints hints, bool on = true);
    QCPainter::RenderHints renderHints() const;

    // Internal
    QCContext* initialize(QCPainterRhiRenderer *renderer);
    void cleanup();

#ifdef QCPAINTER_PERF_DEBUG
    QCPerfLogging *perfLogger() { return &perf; }
#endif

private:
    friend class QCPainterPrivate;
    friend class QCPainterRhiRenderer;

    void setDevicePixelRatio(float ratio);
    void appendCommand(QCCommand command);
    void appendCommands(const QCCommand commands[], int cCount);
    void appendCommandsData(const float commandsData[], int dCount, bool ignoreTransform = false);
    void handleSetPathWinding(QCPainter::PathWinding winding);
    void handleClosePath();
    void handleMoveTo();
    void handleAddPoint(float x, float y, QCPointFlags flags);
    void commandsToPaths();
    void tesselateBezier(float x1, float y1, float x2, float y2,
                         float x3, float y3, float x4, float y4,
                         int level, QCPointFlags flags);
    void enforceWinding(int pointsOffset, int pointsCount, QCPainter::PathWinding winding);
    void expandFill();
    void expandStroke(float w, QCPainter::LineCap cap, QCPainter::LineJoin join, float miterLimit);
    void calculateJoins(float w, QCPainter::LineJoin join, float miterLimit);
    void addBevelJoin(const QCPoint &p0, const QCPoint &p1,
                      float lw, float rw, float lu, float ru);
    void addRoundJoin(const QCPoint &p0, const QCPoint &p1,
                      float lw, float rw, float lu, float ru, int capDivs);
    void addButtCapStart(const QCPoint &p,
                         float dx, float dy, float w, float d,
                         float aa, float u0, float u1);
    void addRoundCapStart(const QCPoint &p,
                          float dx, float dy, float w, int capDivs,
                          float u0, float u1);
    void addButtCapEnd(const QCPoint &p,
                       float dx, float dy, float w, float d,
                       float aa, float u0, float u1);
    void addRoundCapEnd(const QCPoint &p,
                        float dx, float dy, float w, int capDivs,
                        float u0, float u1);
    inline void addVert(float x, float y, float u, float v) noexcept;
    void ensureVertices(int count);
    void preparePainterPath(const QCPainterPath &path,
                            const QTransform &transform = QTransform());
    void appendPainterPath(const QCPainterPath &path,
                           const QTransform &transform = QTransform());
    void appendPainterPath(const QCPainterPath &path,
                           qsizetype start,
                           qsizetype count,
                           const QTransform &transform = QTransform());
    bool fillPathUpdateRequired(QCPainterPath *path, int pathGroup);
    bool strokePathUpdateRequired(QCPainterPath *path, int pathGroup);
    QCPainter::TextAlign effectiveTextAlign(QStringView text) const;

    QCPaint getFillPaint(bool ignoreTransform = false);
    QCPaint getStrokePaint(float *strokeWidth, bool ignoreTransform = false);
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    void updateStateFontVars();
#endif
    QCContext ctx;
    QCState state;
    QCPainterRhiRenderer *m_renderer = nullptr;

#ifdef QCPAINTER_PERF_DEBUG
    QCPerfLogging perf;
#endif
};

QT_END_NAMESPACE

#endif // QCPAINTERENGINE_P_H
