// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2013 Mikko Mononen memon@inside.org
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcpainterengine_p.h"
#include "qcpainterrhirenderer_p.h"
#include "qcanvascustombrush.h"
#include "qcanvascustombrush_p.h"
#include "qcanvaspath_p.h"
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
#include "qctextlayout_p.h"
#include <QtGui/private/qdistancefield_p.h>
#endif
#include <qdebug.h>
#include <QtCore/qscopedvaluerollback.h>
#include <float.h>

#include <QtGui/private/qvectorpath_p.h>

QT_BEGIN_NAMESPACE

// TODO: Measure optimal values for these
// These buffers store amount of a single path, so between beginPath() calls.
#ifndef QCPAINTER_INITIAL_COMMANDS_SIZE
#define QCPAINTER_INITIAL_COMMANDS_SIZE 256
#endif
#ifndef QCPAINTER_INITIAL_COMMANDS_DATA_SIZE
#define QCPAINTER_INITIAL_COMMANDS_DATA_SIZE 512
#endif
#ifndef QCPAINTER_INITIAL_VERTICES_SIZE
#define QCPAINTER_INITIAL_VERTICES_SIZE 4096
#endif
#ifndef QCPAINTER_INITIAL_POINTS_SIZE
#define QCPAINTER_INITIAL_POINTS_SIZE 1024
#endif
#ifndef QCPAINTER_INITIAL_PATHS_SIZE
#define QCPAINTER_INITIAL_PATHS_SIZE 64
#endif
#ifndef QCPAINTER_MAX_STATES
#define QCPAINTER_MAX_STATES 32
#endif
#ifndef QCPAINTER_TEXT_VERTEX_INDEX_LIST_REUSE_CAPACITY
// 1 MB for vertices (16 byte each), 256 KB for indices (4 byte each)
#define QCPAINTER_TEXT_VERTEX_INDEX_LIST_REUSE_CAPACITY (64 * 1024)
#endif

#ifndef QCPAINTER_STRICT_CONVEX_PATH_CHECK
#define QCPAINTER_STRICT_CONVEX_PATH_CHECK 1
#endif

static const float QCPAINTER_MAX_ANTIALIAS_WIDTH = 10.0f;
static const int QCPAINTER_MAX_TESSELLATION_LEVEL = 11;
static const float QCPAINTER_ANTIALIAS_MULTIPLIER = 1.25f;

// Returns value of 'name' env variable as float, or
// defaultValue when not set.
static float qt_qc_envFloat(const char *name, float defaultValue)
{
    if (Q_LIKELY(!qEnvironmentVariableIsSet(name)))
        return defaultValue;
    bool ok = false;
    const float value = qgetenv(name).toFloat(&ok);
    return ok ? value : defaultValue;
}

QCContext* QCPainterEngine::initialize(QCPainterRhiRenderer *renderer)
{
    m_renderer = renderer;
    ctx.commands.resize(QCPAINTER_INITIAL_COMMANDS_SIZE);
    ctx.commandsData.resize(QCPAINTER_INITIAL_COMMANDS_DATA_SIZE);
    ctx.vertices.resize(QCPAINTER_INITIAL_VERTICES_SIZE);
    ctx.paths.resize(QCPAINTER_INITIAL_PATHS_SIZE);
    ctx.points.resize(QCPAINTER_INITIAL_POINTS_SIZE);

    reset();
    setDevicePixelRatio(1.0f);

    return &ctx;
}

void QCPainterEngine::cleanup()
{
    ctx.commands.clear();
    ctx.commandsData.clear();
    ctx.paths.clear();
    ctx.points.clear();
    ctx.dashPaths.clear();
    ctx.dashPoints.clear();
    ctx.currentPath = nullptr;
}

void QCPainterEngine::releaseUnusedResources()
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    textVertices = {};
    textIndices = {};
#endif

    ctx.commands.squeeze();
    ctx.commandsData.squeeze();
    ctx.vertices.squeeze();
    ctx.paths.squeeze();
    ctx.points.squeeze();
    // Only needed between dashPaths() and expandStroke()
    ctx.dashPaths = {};
    ctx.dashPoints = {};
}

QCPainterEngine::QCPainterEngine()
{
#ifdef QCPAINTER_PERF_DEBUG
    perf.start();
#endif
}

// ***** State *****

void QCPainterEngine::beginPaint(float logicalWidth, float logicalHeight, float dpr)
{
    ctx.states.clear();
    reset();

    setDevicePixelRatio(dpr);

    m_renderer->setViewport(0.0f, 0.0f, logicalWidth, logicalHeight);

    ctx.view = { 0.0f, 0.0f, logicalWidth, logicalHeight };
    ctx.dpr = dpr;
    ctx.customMatrixValid = false;
}

void QCPainterEngine::setCustomMatrix(const QMatrix4x4 &matrix)
{
    ctx.customMatrix = matrix;
    ctx.customMatrixValid = true;
}

void QCPainterEngine::endPaint()
{
}

void QCPainterEngine::save()
{
    if (ctx.states.size() >= QCPAINTER_MAX_STATES) {
        qWarning() << "Maximum amount of states reached";
        return;
    }
    ctx.states << state;
}

void QCPainterEngine::restore()
{
    if (ctx.states.isEmpty())
        return;
    state = ctx.states.takeLast();
    applyClipMode();
}

void QCPainterEngine::reset()
{
    state.fill = QCPaint();
    state.stroke = QCPaint();
    state.strokeWidth = 1.0f;
    // Must match what setDevicePixelRatio() and setAntialias(1) would set.
    state.antialias = QCPAINTER_ANTIALIAS_MULTIPLIER / ctx.devicePxRatio;
    state.miterLimit = 10.0f;
    state.lineDash.clear();
    state.lineDashOffset = 0.0;
    state.lineCap = QCanvasPainter::LineCap::Butt;
    state.lineJoin = QCanvasPainter::LineJoin::Miter;
    state.compositeOperation = QCanvasPainter::CompositeOperation::SourceOver;
    state.alpha = 1.0f;
    state.brightness = 1.0f;
    state.contrast = 1.0f;
    state.saturation = 1.0f;
    state.transform.reset();
    state.brushTransform.reset();
    state.clip.extent[0] = -1.0f;
    state.clip.extent[1] = -1.0f;
    state.customFill = {};
    state.customStroke = {};
    state.textWrapMode = QCanvasPainter::WrapMode::NoWrap;
    state.textAlignment = QCanvasPainter::TextAlign::Start;
    state.textBaseline = QCanvasPainter::TextBaseline::Alphabetic;
    state.textDirection = QCanvasPainter::TextDirection::Inherit;
    state.textLineHeight = 0.0f;
    state.textAntialias = 1.0f;
    state.font = QFont();
    state.fillRule = QCanvasPainter::FillRule::NonZero;

    // Blending is almost always enabled, so e.g. antialiasing,
    // non-opaque colors and composition modes work.
    state.blendEnable = true;

    ctx.fontId = 0;
    ctx.colorFontId = 0;
    ctx.fontAlphaMin = -1.0f;
    ctx.fontAlphaMax = -1.0f;
    ctx.pathsCount = 0;
    ctx.pointsCount = 0;
    ctx.verticesCount = 0;
    ctx.commandsCount = 0;
    ctx.commandsDataCount = 0;
    ctx.currentPath = nullptr;
    ctx.preparedPathSerial = 0;

    ctx.states.clear();

    resetClipRect();
}


// ***** Painting styles *****

void QCPainterEngine::setStrokeColor(QColor color)
{
    // Reset other variables
    state.stroke.brushType = BrushColor;
    //state.stroke.transform.reset();
    state.stroke.imageId = 0;
    state.stroke.radius = 0.0f;
    state.stroke.feather = 1.0f;
    // Set inner and outer color
    state.stroke.innerColor = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
    state.stroke.outerColor = state.stroke.innerColor;
    state.customStroke = {};
}

void QCPainterEngine::setStrokePaint(const QCPaint &paint)
{
    state.stroke = paint;
    state.customStroke = {};
}

void QCPainterEngine::setFillColor(QColor color)
{
    // Reset other variables
    state.fill.brushType = BrushColor;
    //state.fill.transform.reset();
    state.fill.imageId = 0;
    state.fill.radius = 0.0f;
    state.fill.feather = 1.0f;
    // Set inner and outer color
    state.fill.innerColor = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
    state.fill.outerColor = state.fill.innerColor;
    state.customFill = {};
}

void QCPainterEngine::setFillPaint(const QCPaint &paint)
{
    state.fill = paint;
    state.customFill = {};
}

void QCPainterEngine::setLineWidth(float width)
{
    state.strokeWidth = width;
}

void QCPainterEngine::setLineCap(QCanvasPainter::LineCap lineCap)
{
    state.lineCap = lineCap;
}

void QCPainterEngine::setLineJoin(QCanvasPainter::LineJoin lineJoin)
{
    state.lineJoin = lineJoin;
}

void QCPainterEngine::setGlobalAlpha(float alpha)
{
    state.alpha = alpha;
}

void QCPainterEngine::setGlobalBrightness(float value)
{
    state.brightness = value;
}

void QCPainterEngine::setGlobalContrast(float value)
{
    state.contrast = value;
}

void QCPainterEngine::setGlobalSaturation(float value)
{
    state.saturation = value;
}

void QCPainterEngine::setFillRule(QCanvasPainter::FillRule fillRule)
{
    state.fillRule = fillRule;
}

QCanvasPainter::FillRule QCPainterEngine::fillRule() const
{
    return state.fillRule;
}

// ***** Custom paints *****

void QCPainterEngine::setCustomStrokeBrush(const QCanvasBrush &brush)
{
    state.customStroke = brush;
}

void QCPainterEngine::setCustomFillBrush(const QCanvasBrush &brush)
{
    state.customFill = brush;
}

// ***** Transform *****

QTransform QCPainterEngine::currentTransform() const
{
    return state.transform;
}

// Returns \a transform without the perspective part (m13, m23 and m33), as
// only affine transforms are supported. Without this, the perspective would
// apply to some of the drawing and not to the rest. Canvas2D ignores those
// components the same way.
static QTransform affineTransform(const QTransform &transform)
{
    return QTransform(transform.m11(), transform.m12(), transform.m21(), transform.m22(),
                      transform.dx(), transform.dy());
}

void QCPainterEngine::transform(const QTransform &transform)
{
    state.transform = affineTransform(transform) * state.transform;
}

void QCPainterEngine::setTransform(const QTransform &transform)
{
    state.transform = affineTransform(transform);
}

void QCPainterEngine::resetTransform()
{
    state.transform.reset();
}

void QCPainterEngine::translate(float x, float y)
{
    state.transform = state.transform.translate(x, y);
}

void QCPainterEngine::scale(float x, float y)
{
    state.transform = state.transform.scale(x, y);
}

void QCPainterEngine::rotate(float angle)
{
    state.transform = state.transform.rotateRadians(angle);
}

void QCPainterEngine::skew(float angleX, float angleY)
{
    state.transform = state.transform.shear(angleX, angleY);
}

void QCPainterEngine::setBrushTransform(const QTransform &transform)
{
    state.brushTransform = transform;
}

// ***** Images *****

int QCPainterEngine::createImage(int width, int height,
                                        QCanvasPainter::ImageFlags flags,
                                        const uchar* data)
{
    return m_renderer->renderCreateTexture(TextureFormatRGBA, width, height, flags, data);
}

bool QCPainterEngine::deleteImage(int imageId)
{
    return m_renderer->renderDeleteTexture(imageId);
}

// Fills a rect (x, y, width, height) with the given paint and
// antialiasing turned off. After painting, the antialiasing
// state and previous fill brush are returned back into use.
void QCPainterEngine::fillPlainRect(const QCPaint &paint, float x, float y, float width, float height)
{
    const QCPaint prevFill = state.fill;
    const auto prevCustomFill = state.customFill;
    const bool prevAA = m_renderer->testFlag(QCPainterRhiRenderer::Antialiasing);
    m_renderer->setFlag(QCPainterRhiRenderer::Antialiasing, false);
    state.fill = paint;
    state.customFill = {};
    beginPath();
    addRect(x, y, width, height);
    fill(QCanvasPainter::FillRule::NonZero);
    m_renderer->setFlag(QCPainterRhiRenderer::Antialiasing, prevAA);
    state.fill = prevFill;
    state.customFill = prevCustomFill;
}

void QCPainterEngine::drawImageId(int imageId, float x, float y, float width, float height, QColor tintColor)
{
    QCPaint ip = createImagePattern(x, y, width, height, imageId, 0.0f, tintColor);
    fillPlainRect(ip, x, y, width, height);
}

void QCPainterEngine::drawImageIdAt(int imageId, float x, float y, float width, float height,
                                    float dX, float dY, float dWidth, float dHeight,
                                    QColor tintColor)
{
    QCPaint ip = createImagePattern(x, y, width, height, imageId, 0.0f, tintColor);
    fillPlainRect(ip, dX, dY, dWidth, dHeight);
}

QCPaint QCPainterEngine::createImagePattern(float x, float y, float width, float height,
                                            int imageId, float angle, QColor tintColor)
{
    QCPaint p;
    p.brushType = BrushImage;
    p.transform = p.transform.translate(x, y);
    if (!qFuzzyIsNull(angle))
        p.transform = p.transform.rotateRadians(angle);

    p.extent[0] = width;
    p.extent[1] = height;

    p.imageId = imageId;

    p.innerColor = { tintColor.redF(), tintColor.greenF(), tintColor.blueF(), tintColor.alphaF() };
    // Not used currently
    //p.outerColor = WHITE_COLOR;

    return p;
}

// ***** Paths *****

void QCPainterEngine::beginPath()
{
    ctx.commandsCount = 0;
    ctx.commandsDataCount = 0;
    ctx.pathsCount = 0;
    ctx.pointsCount = 0;
    ctx.currentPath = nullptr;
    ctx.preparedPathSerial = 0;
    ctx.verticesCount = 0;
}

void QCPainterEngine::closePath()
{
    appendCommand(QCCommand::Close);
}

void QCPainterEngine::moveTo(float x, float y)
{
    float data[] = { x, y };
    appendCommandsData(data, 2);
    appendCommand(QCCommand::MoveTo);
}

void QCPainterEngine::lineTo(float x, float y)
{
    float data[] = { x, y };
    appendCommandsData(data, 2);
    appendCommand(QCCommand::LineTo);
}

void QCPainterEngine::bezierTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y)
{
    float data[] = { cp1X, cp1Y, cp2X, cp2Y, x, y };
    appendCommandsData(data, 6);
    appendCommand(QCCommand::BezierTo);
}

void QCPainterEngine::quadTo(float cpX, float cpY, float x, float y)
{
    // Continue from previous point
    const float prevX = ctx.prevX;
    const float prevY = ctx.prevY;
    static constexpr float m = 2.0f / 3.0f;
    float cp1X = prevX + m * (cpX - prevX);
    float cp1Y = prevY + m * (cpY - prevY);
    float cp2X = x + m * (cpX - x);
    float cp2Y = y + m * (cpY - y);
    float data[] = { cp1X, cp1Y, cp2X, cp2Y, x, y };
    appendCommandsData(data, 6);
    appendCommand(QCCommand::BezierTo);
}

void QCPainterEngine::arcTo(float x1, float y1, float x2, float y2, float radius)
{
    // Continue from previous point
    const float prevX = ctx.prevX;
    const float prevY = ctx.prevY;

#ifdef QCPAINTER_EQUAL_POINTS_CHECKING_ENABLED
    // See if straight line is enough
    if (pointsEquals(prevX, prevY, x1, y1, ctx.distTol) ||
        pointsEquals(x1, y1, x2, y2, ctx.distTol) ||
        pointInSegment(x1, y1, prevX, prevY, x2, y2, ctx.distTol) ||
        radius < ctx.distTol) {
        lineTo(x1, y1);
        return;
    }
#endif

    // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
    float dx0 = prevX - x1;
    float dy0 = prevY - y1;
    float dx1 = x2 - x1;
    float dy1 = y2 - y1;
    normalizePoint(&dx0, &dy0);
    normalizePoint(&dx1, &dy1);
    float a = std::acos(dx0 * dx1 + dy0 * dy1);
    float d = radius / std::tan(a * 0.5f);

    float cx, cy, a0, a1;
    QCanvasPainter::PathWinding direction;
    if (crossProduct(dx0, dy0, dx1, dy1) > 0.0f) {
        direction = QCanvasPainter::PathWinding::ClockWise;
        cx = x1 + dx0 * d + dy0 * radius;
        cy = y1 + dy0 * d - dx0 * radius;
        a0 = std::atan2(dx0, -dy0);
        a1 = std::atan2(-dx1, dy1);
    } else {
        direction = QCanvasPainter::PathWinding::CounterClockWise;
        cx = x1 + dx0 * d - dy0 * radius;
        cy = y1 + dy0 * d + dx0 * radius;
        a0 = std::atan2(-dx0, dy0);
        a1 = std::atan2(dx1, -dy1);
    }

    addArc(cx, cy, radius, a0, a1, direction, true);
}

void QCPainterEngine::addArc(float x, float y, float radius,
                                float a0, float a1, QCanvasPainter::PathWinding direction,
                                bool isConnected)
{
    // Clamp angles
    float da = a1 - a0;
    static constexpr float TWOPI = float(M_PI) * 2;
    if (direction == QCanvasPainter::PathWinding::ClockWise) {
        if (std::abs(da) >= TWOPI) {
            da = TWOPI;
        } else {
            while (da < 0.0f) da += TWOPI;
        }
    } else {
        if (std::abs(da) >= TWOPI) {
            da = -TWOPI;
        } else {
            while (da > 0.0f) da -= TWOPI;
        }
    }

    // Split arc into max 90 degree segments.
    const int divsCount = std::clamp(int(std::abs(da) / M_PI_2 + 0.5f), 1, 5);
    float hda = (da / float(divsCount)) * 0.5f;
    float kappa = std::abs(4.0f / 3.0f * (1.0f - std::cos(hda)) / std::sin(hda));

    if (direction == QCanvasPainter::PathWinding::CounterClockWise)
        kappa = -kappa;

    QCCommand firstCmd = ctx.commandsCount == 0 || !isConnected ? QCCommand::MoveTo : QCCommand::LineTo;
    float prevTanX = 0;
    float prevTanY = 0;
    float prevVX = 0;
    float prevVY = 0;
    // divsCount is max 5 and min 1, meaning dSize is 8..32
    const int dSize = 2 + divsCount * 6;
    QVarLengthArray<QCCommand, 6> commands(1 + divsCount);
    QVarLengthArray<float, 32> data(dSize);
    int cCount = 0;
    int dCount = 0;
    for (int i = 0; i <= divsCount; i++) {
        float a = a0 + da * (i / (float)divsCount);
        float dx = std::cos(a);
        float dy = std::sin(a);
        float vx = x + dx * radius;
        float vy = y + dy * radius;
        float tanx = -dy * radius * kappa;
        float tany = dx * radius * kappa;
        if (i == 0) {
            commands[cCount++] = firstCmd;
            data[dCount++] = vx;
            data[dCount++] = vy;
        } else {
            commands[cCount++] = QCCommand::BezierTo;
            data[dCount++] = prevVX + prevTanX;
            data[dCount++] = prevVY + prevTanY;
            data[dCount++] = vx - tanx;
            data[dCount++] = vy - tany;
            data[dCount++] = vx;
            data[dCount++] = vy;
        }
        prevVX = vx;
        prevVY = vy;
        prevTanX = tanx;
        prevTanY = tany;
    }
    appendCommandsData(data.constData(), dCount);
    appendCommands(commands.constData(), cCount);
}

void QCPainterEngine::addRect(float x, float y, float width, float height)
{
    float data[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };
    appendCommandsData(data, 8);
    static constexpr QCCommand commands[] = {
        QCCommand::MoveTo,
        QCCommand::LineTo,
        QCCommand::LineTo,
        QCCommand::LineTo,
        QCCommand::Close
    };
    appendCommands(commands, 5);
}

void QCPainterEngine::addRoundRect(float x, float y, float width, float height, float radius)
{
    static const float MINR = 0.1f;
    const bool noRadius = (radius < MINR);
    if (noRadius) {
        addRect(x, y, width, height);
    } else {
        // The maximum radius is slightly less than half of smaller side of the rect,
        // to not grow too big even with some float rounding errors.
        const float maxRad = std::min(std::abs(width), std::abs(height)) * 0.4999f;
        const float cornerRad = std::min(radius, maxRad);
        const float rX = cornerRad * sign(width);
        const float rY = cornerRad * sign(height);
        const float xW = x + width;
        const float yH = y + height;
        const float rYCK = rY * COMP_KAPPA90;
        const float rXCK = rX * COMP_KAPPA90;
        float data[] = {
            x + rX, y,
            xW - rX, y,
            xW - rXCK, y, xW, y + rYCK, xW, y + rY,
            xW, yH - rY,
            xW, yH - rYCK, xW - rXCK, yH, xW - rX, yH,
            x + rX, yH,
            x + rXCK, yH, x, yH - rYCK, x, yH - rY,
            x, y + rY,
            x, y + rYCK, x + rXCK, y, x + rX, y
        };

        appendCommandsData(data, 34);
        static constexpr QCCommand commands[] = {
            QCCommand::MoveTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::Close
        };
        appendCommands(commands, 10);
    }
}

void QCPainterEngine::addRoundRect(float x, float y, float width, float height,
                                   float radiusTopLeft, float radiusTopRight,
                                   float radiusBottomRight, float radiusBottomLeft)
{
    static const float MINR = 0.1f;
    const bool noRadius = (radiusTopLeft < MINR) && (radiusTopRight < MINR) &&
                          (radiusBottomRight < MINR) && (radiusBottomLeft < MINR);
    if (noRadius) {
        addRect(x, y, width, height);
    } else {
        const float top = std::max(MINR, radiusTopLeft + radiusTopRight);
        const float right = std::max(MINR, radiusTopRight + radiusBottomRight);
        const float bottom = std::max(MINR, radiusBottomRight + radiusBottomLeft);
        const float left = std::max(MINR, radiusBottomLeft + radiusTopLeft);
        // Find scale, if all radius don't fit. This is how canvas roundRect() behaves:
        // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-roundrect
        const float scale = std::min({1.0f,
                                      qAbs(width / top),
                                      qAbs(height / right),
                                      qAbs(width / bottom),
                                      qAbs(height / left)});
        const float wScale = scale * sign(width);
        const float hScale = scale * sign(height);
        const float rXBL = radiusBottomLeft * wScale;
        const float rYBL = radiusBottomLeft * hScale;
        const float rXBR = radiusBottomRight * wScale;
        const float rYBR = radiusBottomRight * hScale;
        const float rXTR = radiusTopRight * wScale;
        const float rYTR = radiusTopRight * hScale;
        const float rXTL = radiusTopLeft * wScale;
        const float rYTL = radiusTopLeft * hScale;
        const float xW = x + width;
        const float yH = y + height;
        float data[] = {
            x + rXTL, y,
            xW - rXTR, y,
            xW - rXTR * COMP_KAPPA90, y, xW, y + rYTR * COMP_KAPPA90, xW, y + rYTR,
            xW, yH - rYBR,
            xW, yH - rYBR * COMP_KAPPA90, xW - rXBR * COMP_KAPPA90, yH, xW - rXBR, yH,
            x + rXBL, yH,
            x + rXBL * COMP_KAPPA90, yH, x, yH - rYBL * COMP_KAPPA90, x, yH - rYBL,
            x, y + rYTL,
            x, y + rYTL * COMP_KAPPA90, x + rXTL * COMP_KAPPA90, y, x + rXTL, y
        };

        appendCommandsData(data, 34);
        static constexpr QCCommand commands[] = {
            QCCommand::MoveTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::LineTo,
            QCCommand::BezierTo,
            QCCommand::Close
        };
        appendCommands(commands, 10);
    }
}

void QCPainterEngine::addEllipse(float x, float y, float radiusX, float radiusY)
{
    const float radYK = radiusY * KAPPA90;
    const float radXK = radiusX * KAPPA90;
    const float ymRadY = y - radiusY;
    const float ypRadY = y + radiusY;
    const float xmRadX = x - radiusX;
    const float xpRadX = x + radiusX;
    float data[] = {
        xpRadX, y,
        xpRadX, y + radYK, x + radXK, ypRadY, x, ypRadY,
        x - radXK, ypRadY, xmRadX, y + radYK, xmRadX, y,
        xmRadX, y - radYK, x - radXK, ymRadY, x, ymRadY,
        x + radXK, ymRadY, xpRadX, y - radYK, xpRadX, y,
    };
    appendCommandsData(data, 26);
    static constexpr QCCommand commands[] = {
        QCCommand::MoveTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
    };
    appendCommands(commands, 5);
}

void QCPainterEngine::addCircle(float x, float y, float radius)
{
    const float radK = radius * KAPPA90;
    const float ymRad = y - radius;
    const float ypRad = y + radius;
    float data[] = {
        x + radius, y,
        x + radius, y + radK, x + radK, ypRad, x, ypRad,
        x - radK, ypRad, x - radius, y + radK, x - radius, y,
        x - radius, y - radK, x - radK, ymRad, x, ymRad,
        x + radK, ymRad, x + radius, y - radK, x + radius, y,
    };
    appendCommandsData(data, 26);
    static constexpr QCCommand commands[] = {
        QCCommand::MoveTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
    };
    appendCommands(commands, 5);
}

void QCPainterEngine::addPath(const QPainterPath &path)
{
    const int eCount = path.elementCount();
    for (int i = 0; i < eCount; i++) {
        const auto &element = path.elementAt(i);
        switch (element.type) {
        case QPainterPath::MoveToElement:
        {
            moveTo(element.x, element.y);
            break;
        }
        case QPainterPath::LineToElement:
        {
            lineTo(element.x, element.y);
            break;
        }
        case QPainterPath::CurveToElement:
        {
            // 2 CurveToDataElements always follow the CurveToElement
            const auto &data1 = path.elementAt(++i);
            const auto &data2 = path.elementAt(++i);
            bezierTo(element.x, element.y, data1.x, data1.y, data2.x, data2.y);
            break;
        }
        case QPainterPath::CurveToDataElement:
            // Handled in CurveToElement
            break;
        }
    }
}

void QCPainterEngine::addPath(const QCanvasPath &path, const QTransform &transform)
{
    appendPainterPath(path, transform);
}

void QCPainterEngine::addPath(const QCanvasPath &path, qsizetype start, qsizetype count, const QTransform &transform)
{
    appendPainterPath(path, start, count, transform);
}

void QCPainterEngine::setPathWinding(QCanvasPainter::PathWinding winding)
{
    QCCommand c = winding == QCanvasPainter::PathWinding::ClockWise ?
                      QCCommand::WindingCW : QCCommand::WindingCCW;
    appendCommand(c);
}

void QCPainterEngine::fill(QCanvasPainter::FillRule fillRule, QCanvasPath *maybePath, int pathGroup, bool cachedPathUpdateRequired)
{
    const bool usingPathCaching = maybePath && pathGroup != -1;
    if (!usingPathCaching && ctx.commandsCount < 2)
        return;

    // The points of cached paths are not transformed, the vertex shader scales
    // them. So the antialiasing fringe, which is in device pixels, is made in
    // the units of the points for the scale rounded to a power of two, like
    // for the strokes, see stroke().
    float pixelSize = 1.0f;
    if (usingPathCaching) {
        const float scale = getAverageScale(state.transform);
        if (scale < 1.0e-6f)
            return; // collapsed, nothing visible
        pixelSize = 1.0f / quantizedScale(scale);
    }

#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::FILL);
#endif

    if (!usingPathCaching) {
        commandsToPaths();
        expandFill();

        const bool ignoreTransform = false;
        const QCPaint fillPaint = getFillPaint(ignoreTransform);

        QCRhiUncachedPathDrawArgs args { ctx.paths, ctx.pathsCount };
        m_renderer->renderFill(fillPaint, state, ctx.bounds, args, std::nullopt, fillRule);
    } else {
        if (cachedPathUpdateRequired) {
            // When the data is made for a scale, the curves are flattened in
            // device pixels, like those of the uncached paths. Otherwise the
            // data is used at all scales, and they are flattened in the units
            // of the points.
            const bool forScale = m_renderer->fillCacheProps(state).scale != 0.0f;
            commandsToPaths(forScale ? pixelSize : 1.0f);
            expandFill(pixelSize);
        }

        const bool ignoreTransform = true;
        const QCPaint fillPaint = getFillPaint(ignoreTransform);

        if (cachedPathUpdateRequired) {
            QCRhiCachedPathDrawArgs args { maybePath, pathGroup, state.transform, { { ctx.paths, ctx.pathsCount } } };
            m_renderer->renderFill(fillPaint, state, ctx.bounds, std::nullopt, args, fillRule);
        } else {
            QCRhiCachedPathDrawArgs args { maybePath, pathGroup, state.transform, std::nullopt };
            // there was no commandsToPath -> ctx.bounds is stale -> do not pass it, should use the cached one anyway
            m_renderer->renderFill(fillPaint, state, QRectF(), std::nullopt, args, fillRule);
        }
    }

#ifdef QCPAINTER_PERF_DEBUG
    perf.logEnd(QCPerfLogging::FILL);
#endif
}

void QCPainterEngine::fillForClear()
{
    if (ctx.commandsCount < 2)
        return;

#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::CLEAR);
#endif
    commandsToPaths();
    expandFill();

    QCPaint fillPaint;
    fillPaint.innerColor = { 0.0f, 0.0f, 0.0f, 0.0f }; // transparent black
    fillPaint.outerColor = fillPaint.innerColor;

    const bool wasBlendEnabled = state.blendEnable;
    state.blendEnable = false;
    QCRhiUncachedPathDrawArgs args { ctx.paths, ctx.pathsCount };
    m_renderer->renderFill(fillPaint, state, ctx.bounds, args, std::nullopt, QCanvasPainter::FillRule::NonZero);
    state.blendEnable = wasBlendEnabled;

#ifdef QCPAINTER_PERF_DEBUG
    perf.logEnd(QCPerfLogging::CLEAR);
#endif
}

void QCPainterEngine::stroke(QCanvasPath *maybePath, int pathGroup, bool cachedPathUpdateRequired)
{
    const bool usingPathCaching = maybePath && pathGroup != -1;
    if (!usingPathCaching && ctx.commandsCount < 2)
        return;

    // The points of cached paths are not transformed, the vertex shader scales
    // them. So the stroke is expanded in the units of the points, where one
    // device pixel is 1 / scale. The parts of the geometry that are sized in
    // device pixels (the antialiasing, the minimum and maximum width, the
    // segments of round caps and joins, see strokeCacheProps()) are made
    // for the scale rounded to a power of two, so that the cached data is not
    // made again at every zoom step, see quantizedScale(). The points of the
    // other paths are in device pixels already.
    float pixelSize = 1.0f;
    if (usingPathCaching) {
        const float scale = getAverageScale(state.transform);
        if (scale < 1.0e-6f)
            return; // collapsed, nothing visible
        pixelSize = 1.0f / quantizedScale(scale);
    }

#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::STROKE);
#endif

    // The width in device pixels, which the shaders and the antialiasing use
    float strokeWidth;
    const bool ignoreTransform = usingPathCaching;
    const QCPaint strokePaint = getStrokePaint(&strokeWidth, ignoreTransform);

    // Half of the width of the geometry, in the units of the points. The same
    // as strokeWidth in device pixels, except for the cached paths when the
    // clamping of the width is different at the scale the geometry is made for.
    float halfWidth = strokeWidth * 0.5f;
    if (usingPathCaching) {
        const float bakedWidth = std::max(std::clamp(state.strokeWidth / pixelSize, 0.0f,
                                                     QCPAINTER_MAX_STROKE_WIDTH),
                                          state.antialias);
        halfWidth = bakedWidth * 0.5f * pixelSize;
    }

    // With a dash pattern, the dashes are stroked instead of the paths.
    // tolerancePixelSize is the size of a device pixel that the tolerances,
    // the round caps and joins and the antialiasing fringe are made for. For
    // cached data that is used at all scales it is 1, as nothing in the data
    // is then sized in device pixels: there is no fringe and no round caps or
    // joins, and the tolerances don't depend on the scale it is made at first.
    auto expandPaths = [&](float tolerancePixelSize) -> std::pair<QCPaths *, int> {
        commandsToPaths(tolerancePixelSize);
        if (!state.lineDash.isEmpty()) {
            // The points of cached paths are not transformed, the others are
            if (dashPaths(!usingPathCaching, tolerancePixelSize, halfWidth)) {
                expandStroke(ctx.dashPaths, ctx.dashPathsCount, ctx.dashPoints, halfWidth,
                             state.lineCap, state.lineJoin, state.miterLimit, tolerancePixelSize);
                return { &ctx.dashPaths, ctx.dashPathsCount };
            }
        }
        expandStroke(ctx.paths, ctx.pathsCount, ctx.points, halfWidth,
                     state.lineCap, state.lineJoin, state.miterLimit, tolerancePixelSize);
        return { &ctx.paths, ctx.pathsCount };
    };

    if (!usingPathCaching) {
        const auto [paths, pathsCount] = expandPaths(1.0f);
        QCRhiUncachedPathDrawArgs args { *paths, pathsCount };
        m_renderer->renderStroke(strokePaint, state, strokeWidth, args, std::nullopt);
    } else {
        if (cachedPathUpdateRequired) {
            // See fill()
            const bool forScale = m_renderer->strokeCacheProps(state).scale != 0.0f;
            const auto [paths, pathsCount] = expandPaths(forScale ? pixelSize : 1.0f);
            QCRhiCachedPathDrawArgs args { maybePath, pathGroup, state.transform, { { *paths, pathsCount } } };
            m_renderer->renderStroke(strokePaint, state, strokeWidth, std::nullopt, args);
        } else {
            QCRhiCachedPathDrawArgs args { maybePath, pathGroup, state.transform, std::nullopt };
            m_renderer->renderStroke(strokePaint, state, strokeWidth, std::nullopt, args);
        }
    }

#ifdef QCPAINTER_PERF_DEBUG
    perf.logEnd(QCPerfLogging::STROKE);
#endif
}

void QCPainterEngine::fill(const QCanvasPath &path, QCanvasPainter::FillRule fillRule, int pathGroup)
{
    if (path.isEmpty())
        return;

    QCanvasPath *p = const_cast<QCanvasPath *>(&path);
    if (pathGroup == -1) {
        // Not caching, so prepare and fill normally.
        // In this case we apply state transformation into commands data points.
        // So data needs to be prepared again if state transformation has changed.

        const QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(&path);
        if (ctx.preparedPathSerial != pathd->serialNumber
            || ctx.preparedPathTransform != state.transform
            || ctx.preparedPathIterations != pathd->pathIterations
            || ctx.preparedPathCommandsCount != pathd->commandsCount)
        {
            preparePainterPath(path);
            ctx.preparedPathTransform = state.transform;
            ctx.preparedPathCommandsCount = pathd->commandsCount;
            ctx.preparedPathIterations = pathd->pathIterations;
            ctx.preparedPathSerial = pathd->serialNumber;
        }

        fill(fillRule);
    } else {
        // Caching.
        // In this case data points are untransformed, and transformation is applied in vertex shader.

        const bool pathUpdateRequired = fillCachedPathUpdateRequired(p, pathGroup);
        if (pathUpdateRequired) {
            const bool ignoreTransform = true;
            preparePainterPath(path, ignoreTransform);
        }

        fill(fillRule, p, pathGroup, pathUpdateRequired);
    }
}

void QCPainterEngine::stroke(const QCanvasPath &path, int pathGroup)
{
    if (path.isEmpty())
        return;

    QCanvasPath *p = const_cast<QCanvasPath *>(&path);
    if (pathGroup == -1) {
        // Not caching, so prepare and stroke normally.
        // In this case we apply state transformation into commands data points.
        // So data needs to be prepared again if state transformation has changed.

        const QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(&path);
        if (ctx.preparedPathSerial != pathd->serialNumber
            || ctx.preparedPathTransform != state.transform
            || ctx.preparedPathIterations != pathd->pathIterations
            || ctx.preparedPathCommandsCount != pathd->commandsCount)
        {
            preparePainterPath(path);
            ctx.preparedPathTransform = state.transform;
            ctx.preparedPathCommandsCount = pathd->commandsCount;
            ctx.preparedPathIterations = pathd->pathIterations;
            ctx.preparedPathSerial = pathd->serialNumber;
        }

        stroke();
    } else {
        // Caching.
        // In this case data points are untransformed, and transformation is applied in vertex shader.

        const bool pathUpdateRequired = strokeCachedPathUpdateRequired(p, pathGroup);
        if (pathUpdateRequired) {
            const bool ignoreTransform = true;
            preparePainterPath(path, ignoreTransform);
        }

        stroke(p, pathGroup, pathUpdateRequired);
    }
}

void QCPainterEngine::setStencilClip(const QList<QRectF> &rects)
{
    if (rects.isEmpty())
        m_renderer->clearStencil(state);
    else
        m_renderer->renderStencil(state, rects);
}

void QCPainterEngine::setStencilClip(const QVectorPath &clipPath)
{
    if (clipPath.isEmpty())
        m_renderer->clearStencil(state);
    else
        m_renderer->renderStencil(state, clipPath);
}

// ***** Blending *****

void QCPainterEngine::setGlobalCompositeOperation(QCanvasPainter::CompositeOperation op)
{
    state.compositeOperation = op;
}

// ***** Clipping *****

void QCPainterEngine::resetClipRect()
{
    state.clip.transform.reset();
    state.clip.extent[0] = -1.0f;
    state.clip.extent[1] = -1.0f;
    state.clip.rect = {};
    state.clip.mode = QCClip::Mode::None;
    applyClipMode();
}

// Sets the renderer flags of clipping from the state, which save() and
// restore() do not otherwise change.
void QCPainterEngine::applyClipMode()
{
    m_renderer->setFlag(QCPainterRhiRenderer::SimpleClipping,
                        state.clip.mode == QCClip::Mode::Simple);
    m_renderer->setFlag(QCPainterRhiRenderer::TransformedClipping,
                        state.clip.mode == QCClip::Mode::Transformed);
}

void QCPainterEngine::setClipRect(const QRectF &clipRect)
{
    QRectF rect = clipRect;
    if (rect.isEmpty()) {
        resetClipRect();
    } else {
        if (ctx.customMatrixValid) {
            const QMatrix4x4 &m(ctx.customMatrix);
            // Basic local clip rect -> scissor rect logic, matching the Qt
            // Quick scenegraph renderer. Only correct for an orthographic,
            // translation-only customMatrix. Rotations are currently not
            // supported, and full 3D projections are definitely out of scope.
            const float invW = 1 / m(3, 3);
            float fx1 = (rect.left() * m(0, 0) + m(0, 3)) * invW;
            float fy1 = (rect.top() * m(1, 1) + m(1, 3)) * invW;
            float fx2 = (rect.right() * m(0, 0) + m(0, 3)) * invW;
            float fy2 = (rect.bottom() * m(1, 1) + m(1, 3)) * invW;
            if (fx1 > fx2)
                std::swap(fx1, fx2);
            if (fy1 > fy2)
                std::swap(fy1, fy2);
            const int ix1 = qRound((fx1 + 1) * ctx.view.width() * 0.5f);
            const int iy1 = qRound((fy1 + 1) * ctx.view.height() * 0.5f);
            const int ix2 = qRound((fx2 + 1) * ctx.view.width() * 0.5f);
            const int iy2 = qRound((fy2 + 1) * ctx.view.height() * 0.5f);
            if (m_renderer->isYUpInNDC())
                rect = QRect(ix1, ctx.view.height() - iy1 - (iy2 - iy1), ix2 - ix1, iy2 - iy1);
            else
                rect = QRect(ix1, iy1, ix2 - ix1, iy2 - iy1);
        }
        if (state.transform.isIdentity()) {
            // Simple clipping of rectangular area without transform
            state.clip.transform.reset();
            state.clip.extent[0] = -1.0f;
            state.clip.extent[1] = -1.0f;
            // Calculate flipped y and take dpr into account.
            state.clip.rect = { rect.x() * ctx.dpr,
                               (ctx.view.height() - rect.y() - rect.height()) * ctx.dpr,
                               rect.width() * ctx.dpr,
                               rect.height() * ctx.dpr };
            state.clip.mode = QCClip::Mode::Simple;
        } else {
            // Clipping done in fragment shader and taking into use the current transform.
            state.clip.rect = {};
            float width = std::max(0.0f, float(rect.width()));
            float height = std::max(0.0f, float(rect.height()));
            auto &t = state.clip.transform;
            t.reset();
            t = t.translate(rect.x() + (width * 0.5f), rect.y() + (height * 0.5f));
            t *= state.transform;
            state.clip.extent[0] = width * 0.5f;
            state.clip.extent[1] = height * 0.5f;
            state.clip.mode = QCClip::Mode::Transformed;
        }
        applyClipMode();
    }
}


// ***** Text *****

void QCPainterEngine::setTextWrapMode(QCanvasPainter::WrapMode wrapMode)
{
    state.textWrapMode = wrapMode;
}

void QCPainterEngine::setTextLineHeight(float height)
{
    state.textLineHeight = height;
}

void QCPainterEngine::setTextAntialias(float antialias)
{
    state.textAntialias = antialias;
}

void QCPainterEngine::setTextAlignment(QCanvasPainter::TextAlign align)
{
    state.textAlignment = align;
}

void QCPainterEngine::setTextBaseline(QCanvasPainter::TextBaseline baseline)
{
    state.textBaseline = baseline;
}

void QCPainterEngine::setTextDirection(QCanvasPainter::TextDirection direction)
{
    state.textDirection = direction;
}

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT

// Common part of fillText()/fillShapedText(): renders whatever glyph/decoration
// geometry populateFont()/populateFontFromShapedText() left in textVertices/
// textIndices (fontTex is the texture id they returned), then the color-glyph
// pass, then trims the vertex/index buffers back down if they grew large.

void QCPainterEngine::renderFilledText(int fontTex)
{
    // Monochrome (SDF) glyphs and text decorations.
    if (!textVertices.empty()) {
        const QCPaint p = getFillPaint();
        ctx.fontId = fontTex;
        updateStateFontVars();

        // Decoration rects (underline/overline/strikeout) are appended to
        // textVertices/textIndices by the glyph cache with a texCoord pointing
        // into a reserved 0xFF region of the SDF atlas, so they participate in
        // this same draw call and pick up the same shader as the glyphs.
        if (state.customFill.type() != QCanvasBrush::BrushType::Custom) {
            m_renderer->renderTextFill(p, state, textVertices, textIndices);
        } else {
            // Using custom brush. Avoid detaching the brush.
            auto *customPriv = static_cast<QCanvasCustomBrushPrivate *>(
                    QCanvasBrushPrivate::get(std::as_const(state.customFill)));
            m_renderer->renderTextFillCustom(
                    p, state, customPriv, textVertices, textIndices);
        }
    }

    // Color (emoji) glyphs are drawn by samppling the color atlas directly, so
    // they keep their native colors instead of being tinted by the fill.
    auto colorTex = m_renderer->populateColorFont(colorTextVertices, colorTextIndices);
    if (colorTex != 0 && !colorTextVertices.empty()) {
        ctx.colorFontId = colorTex;
        m_renderer->renderTextFill({}, state, colorTextVertices, colorTextIndices, true);
    }

    // simple memory usage cap; drawing huge texts will not reuse the containers (and their allocations)
    if (textVertices.capacity() > QCPAINTER_TEXT_VERTEX_INDEX_LIST_REUSE_CAPACITY)
        textVertices = {};
    if (textIndices.capacity() > QCPAINTER_TEXT_VERTEX_INDEX_LIST_REUSE_CAPACITY)
        textIndices = {};
    if (colorTextVertices.capacity() > QCPAINTER_TEXT_VERTEX_INDEX_LIST_REUSE_CAPACITY)
        colorTextVertices = {};
    if (colorTextIndices.capacity() > QCPAINTER_TEXT_VERTEX_INDEX_LIST_REUSE_CAPACITY)
        colorTextIndices = {};
}
#endif

void QCPainterEngine::fillText(const QString &text, const QRectF &rect)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    auto tex = m_renderer->populateFont(state.font, rect, text, textVertices, textIndices);
    renderFilledText(tex);
#endif
}

void QCPainterEngine::fillShapedText(QFontEngine *fontEngine, const quint32 *glyphIndexes,
                                   const QFixedPoint *glyphPositions, int glyphCount)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    auto tex = m_renderer->populateFontFromShapedText(fontEngine, glyphIndexes, glyphPositions,
                                                     glyphCount, textVertices, textIndices);
    renderFilledText(tex);
#endif
}

QRectF QCPainterEngine::textBoundingBox(const QString &text, const QRectF &rect)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    auto &font = state.font;
    auto metrics = QFontMetricsF{font};

    // TODO: Duplicate code from QCDistanceFieldGlyphCache::generate()
    // We should move this layouting into a shared method.
    // And cache the result so that textBoundingBox() and fillText() with
    // same data don't do double layouting.
    auto option = QTextOption{};
    option.setWrapMode(QCTextLayout::convertToQtWrapMode(state.textWrapMode));
    option.setAlignment(QCTextLayout::convertToQtAlignment(effectiveTextAlign(text)));
    auto layout = QTextLayout();
    layout.clearLayout();
    layout.setTextOption(option);
    layout.setText(QCTextLayout::convertLineBreaks(text));
    layout.setFont(font);
    float leading = metrics.leading();
    float layoutHeight = 0;
    float layoutWidth = 0;
    layout.beginLayout();
    while (true) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(rect.width());
        layoutHeight += leading;
        line.setPosition(QPointF(rect.x(), rect.y() + layoutHeight));
        layoutHeight += line.height() + state.textLineHeight;
        layoutWidth = qMax(layoutWidth, line.naturalTextWidth());
    }
    layout.endLayout();

    // Adjust rect
    float textOffsetY = QCTextLayout::calculateVerticalAlignment(state.textBaseline,
                                                                 rect, metrics,
                                                                 layout.boundingRect());
    std::pair<float, float> textOffsetX =
            QCTextLayout::calculateHorizontalAlignment(effectiveTextAlign(text),
                                                       rect,layoutWidth,
                                                       layout.boundingRect());
    QRectF textRect = layout.boundingRect();
    textRect.adjust(textOffsetX.first, textOffsetY, textOffsetX.second, textOffsetY);
    return textRect;
#else
    return QRectF();
#endif
}

// ***** Other *****

void QCPainterEngine::resetDebugCounters()
{
    m_renderer->resetDebugCounters();
}

void QCPainterEngine::syncDebugCounters()
{
    // Get the numbers from the renderer into ctx.debugCounters. This assumes that
    // we are called after render(), i.e. after endPaint().
    m_renderer->syncDebugCounters();
}

QCDebugCounters QCPainterEngine::debugCounters() const
{
    return ctx.debugCounters;
}

void QCPainterEngine::setAntialias(float antialias)
{
    antialias = std::clamp(antialias, 0.0f, QCPAINTER_MAX_ANTIALIAS_WIDTH);
    state.antialias = QCPAINTER_ANTIALIAS_MULTIPLIER * antialias / ctx.devicePxRatio;
}

// Like the HTML canvas, a pattern with a negative or non-finite length is
// ignored, and an odd pattern is repeated to make the number of lengths even.
void QCPainterEngine::setLineDash(QSpan<const qreal> pattern)
{
    for (qreal length : pattern) {
        if (!(length >= 0.0 && qIsFinite(length)))
            return;
    }
    const qsizetype count = pattern.size() % 2 ? pattern.size() * 2 : pattern.size();
    QList<qreal> dashes;
    dashes.reserve(count);
    for (qsizetype i = 0; i < count; ++i)
        dashes.append(pattern[i % pattern.size()]);
    state.lineDash = std::move(dashes);
}

// Like the HTML canvas, a non-finite offset is ignored
void QCPainterEngine::setLineDashOffset(qreal offset)
{
    if (qIsFinite(offset))
        state.lineDashOffset = offset;
}

void QCPainterEngine::setMiterLimit(float limit)
{
    state.miterLimit = limit;
}

void QCPainterEngine::removePathGroup(int pathGroup)
{
    // Remove from engine side
    erase_if(ctx.cachedStrokePaths, [pathGroup](const QHash<uint, QCCachedPath>::iterator it) {
        return it->pathGroup == pathGroup;
    });
    erase_if(ctx.cachedFillPaths, [pathGroup](const QHash<uint, QCCachedPath>::iterator it) {
        return it->pathGroup == pathGroup;
    });
    // Remove from renderer side
    m_renderer->removePathGroup(pathGroup);
}

void QCPainterEngine::setWindingEnforceEnabled(bool enabled)
{
    m_renderer->setFlag(QCPainterRhiRenderer::WindingEnforce, enabled);
}

void QCPainterEngine::setHighQualityStrokingEnabled(bool enabled)
{
    m_renderer->setFlag(QCPainterRhiRenderer::StencilStrokes, enabled);
}

void QCPainterEngine::setAntialiasingEnabled(bool enabled)
{
    m_renderer->setFlag(QCPainterRhiRenderer::Antialiasing, enabled);
}

// ********** private **********

void QCPainterEngine::setDevicePixelRatio(float ratio)
{
    if (ratio > 0.0f) {
        static const float tessTol = qt_qc_envFloat("QCPAINTER_TESSELLATION_TOLERANCE", 0.25f);
        ctx.tessTol = tessTol / ratio;
        ctx.divsTol = 0.25f / ratio;
        ctx.distTol = 0.01f / ratio;
        // Note: This is not called during the paint operations,
        // so it can set antialias to default value.
        state.antialias = QCPAINTER_ANTIALIAS_MULTIPLIER / ratio;
        ctx.devicePxRatio = ratio;
    }
}

// Append a single \a command.
void QCPainterEngine::appendCommand(QCCommand command)
{
    auto &c = ctx.commands;

    if (ctx.commandsCount + 1 > c.size()) {
        // Increase capacity with the commands amount + 20% of the current size.
        int newSize = (ctx.commandsCount + 1) + c.size() * 0.2;
        c.resize(newSize);
    }

    c[ctx.commandsCount++] = command;
}

// Append \a cCount amount of \a commands.
void QCPainterEngine::appendCommands(const QCCommand commands[], int cCount)
{
    Q_ASSERT(cCount > 0);
    auto &c = ctx.commands;

    if (ctx.commandsCount + cCount > c.size()) {
        // Increase capacity with the commands amount + 20% of the current size.
        int newSize = (ctx.commandsCount + cCount) + c.size() * 0.2;
        c.resize(newSize);
    }

    for (int i = 0; i < cCount; i++)
        c[ctx.commandsCount++] = commands[i];
    //memcpy(&c[ctx.commandsCount], commands, sizeof(QCCommand) * cCount);
    //ctx.commandsCount += cCount;
}

// Append \a dCount amount of \a commands data.
// When \a ignoreTransform is set to true, state tranformation is not applied.
void QCPainterEngine::appendCommandsData(const float commandsData[], int dCount, bool ignoreTransform)
{
    // There are always even amount of data as they are (x, y) points.
    Q_ASSERT(dCount % 2 == 0);
    Q_ASSERT(dCount >= 2);

    // Store previous point, as untransformed.
    ctx.prevX = commandsData[dCount - 2];
    ctx.prevY = commandsData[dCount - 1];

    auto &c = ctx.commandsData;
    if (ctx.commandsDataCount + dCount > c.size()) {
        // Increase capacity with the commands amount + 20% of the current size.
        int newSize = (ctx.commandsDataCount + dCount) + c.size() * 0.2;
        c.resize(newSize);
    }

    if (ignoreTransform || state.transform.isIdentity()) {
        // Add commands directly
        for (int i = 0; i < dCount; i++)
            c[ctx.commandsDataCount++] = commandsData[i];
        //memcpy(&c[ctx.commandsDataCount], commandsData, sizeof(float) * dCount);
        //ctx.commandsDataCount += dCount;
    } else {
        int i = 0;
        while (i < dCount) {
            qreal p1 = commandsData[i++];
            qreal p2 = commandsData[i++];
            state.transform.map(p1, p2, &p1, &p2);
            c[ctx.commandsDataCount++] = float(p1);
            c[ctx.commandsDataCount++] = float(p2);
        }
    }
}

void QCPainterEngine::handleSetPathWinding(QCanvasPainter::PathWinding winding)
{
    ctx.winding = winding;
}

void QCPainterEngine::handleClosePath()
{
    if (auto *path = ctx.currentPath)
        path->isClosed = true;
}

void QCPainterEngine::handleMoveTo()
{
    // Moving starts a new subpath.
    auto &paths = ctx.paths;
    if (ctx.pathsCount + 1 > paths.size()) {
        // Increase capacity with 1 + 20% of the current size.
        int newSize = (ctx.pathsCount + 1) + paths.size() * 0.2;
        paths.resize(newSize);
    }
    QCPath *path = &paths[ctx.pathsCount++];
    *path = {};
    path->pointsOffset = ctx.pointsCount;
    path->winding = ctx.winding;
    ctx.currentPath = path;
}

void QCPainterEngine::handleAddPoint(float x, float y, QCPointFlags flags)
{
    auto *path = ctx.currentPath;
    Q_ASSERT(path); // At this point path always exists.
#ifdef QCPAINTER_EQUAL_POINTS_CHECKING_ENABLED
    if (path->pointsCount > 0 && ctx.pointsCount > 0) {
        QCPoint &pt = ctx.points[ctx.pointsCount - 1];
        if (pointsEquals(pt.x, pt.y, x, y, ctx.distTol)) {
            // Point is close to previous one, just
            // adjust the point flags.
            pt.flags |= flags;
            return;
        }
    }
#endif

    auto &pts = ctx.points;
    if (Q_UNLIKELY(ctx.pointsCount + 1 > pts.size())) {
        // Increase capacity with 1 + 10% of the current size.
        int newSize = (ctx.pointsCount + 1) + pts.size() * 0.1;
        pts.resize(newSize);
    }

    QCPoint &pt = ctx.points[ctx.pointsCount++];
    pt.x = x;
    pt.y = y;
    pt.flags = flags;
    path->pointsCount++;
}

static constexpr float triarea2(float ax, float ay, float bx, float by, float cx, float cy) noexcept
{
    const float abx = bx - ax;
    const float aby = by - ay;
    const float acx = cx - ax;
    const float acy = cy - ay;
    return (acx * aby) - (abx * acy);
}

// Twice the signed area of the points, positive when they go counterclockwise
// on the screen
static float pointsArea(const QCPoints &points, int pointsOffset, int pointsCount)
{
    float area = 0;
    if (pointsCount <= 2)
        return area;
    const QCPoint &a = points.at(pointsOffset);
    for (int i = 2; i < pointsCount; i++) {
        const QCPoint &b = points.at(pointsOffset + i - 1);
        const QCPoint &c = points.at(pointsOffset + i);
        area += triarea2(a.x, a.y, b.x, b.y, c.x, c.y);
    }
    return area;
}

// The first point of a closed path stays first, so that the fills of the paths
// drawn the other way don't change.
static void reversePoints(QCPoints &points, int pointsOffset, int pointsCount, bool isClosed)
{
    int i = isClosed ? 1 : 0;
    int j = pointsCount - 1;
    while (i < j) {
        std::swap(points[pointsOffset + i], points[pointsOffset + j]);
        i++;
        j--;
    }
}

// Returns true when the points are reversed, see reversePoints()
bool QCPainterEngine::enforceWinding(int pointsOffset, int pointsCount, QCanvasPainter::PathWinding winding,
                                     bool isClosed)
{
    if (pointsCount <= 2)
        return false;
    // Calculate if points area is positive or negative
    const float area = pointsArea(ctx.points, pointsOffset, pointsCount);
    // If needed, reverse the points order
    if ((winding == QCanvasPainter::PathWinding::CounterClockWise && area < 0.0f) ||
        (winding == QCanvasPainter::PathWinding::ClockWise && area > 0.0f)) {
        reversePoints(ctx.points, pointsOffset, pointsCount, isClosed);
        return true;
    }
    return false;
}

// Create path elements from commands
// pixelSize is the size of a device pixel in the units of the points. The
// tolerances of flattening the curves are in device pixels.
void QCPainterEngine::commandsToPaths(float pixelSize)
{
    // Paths already created for these commands
    if (ctx.pathsCount > 0)
        return;

    // tessTol is compared to squared distances
    const QScopedValueRollback tessTol(ctx.tessTol, ctx.tessTol * pixelSize * pixelSize);
    const QScopedValueRollback distTol(ctx.distTol, ctx.distTol * pixelSize);

#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::C2P);
#endif

    // The winding set for the subpaths of an earlier path does not carry over
    ctx.winding = QCanvasPainter::PathWinding::CounterClockWise;

    // If the first command is not MoveTo, add it manually
    // and make sure ctx.currentPath exists.
    const QCCommand firstC = ctx.commands.first();
    if (firstC != QCCommand::MoveTo)
        handleMoveTo();

    int dCount = 0;
    const int cCount = ctx.commandsCount;
    for (int i = 0; i < cCount; i++) {
        const QCCommand cmd = ctx.commands.at(i);
        switch (cmd) {
        case QCCommand::MoveTo:
        {
            handleMoveTo();
            Q_FALLTHROUGH();
        }
        case QCCommand::LineTo:
        {
            const float p1 = ctx.commandsData.at(dCount++);
            const float p2 = ctx.commandsData.at(dCount++);
            handleAddPoint(p1, p2, PointCorner);
            break;
        }
        case QCCommand::BezierTo:
        {
            if (ctx.pointsCount > 0) {
                const QCPoint last = ctx.points.at(ctx.pointsCount - 1);
                const float cp1x = ctx.commandsData.at(dCount++);
                const float cp1y = ctx.commandsData.at(dCount++);
                const float cp2x = ctx.commandsData.at(dCount++);
                const float cp2y = ctx.commandsData.at(dCount++);
                const float px = ctx.commandsData.at(dCount++);
                const float py = ctx.commandsData.at(dCount++);
                if (cp1x == last.x && cp1y == last.y && cp2x == last.x && cp2y == last.y
                    && px == last.x && py == last.y) {
                    // A curve of zero size, like that of a circle with the
                    // radius 0, would never get flat. It ends at the last
                    // point, so only that is marked as a corner. Adding the
                    // point again would make a segment of zero length, which
                    // changes the joins of a stroke.
                    ctx.points[ctx.pointsCount - 1].flags |= PointCorner;
                } else {
                    tesselateBezier(last.x, last.y, cp1x, cp1y, cp2x, cp2y, px, py, 0, PointCorner);
                }
            }
            break;
        }
        case QCCommand::Close:
        {
            handleClosePath();
            break;
        }
        case QCCommand::WindingCW:
        case QCCommand::WindingCCW:
        {
            auto w = cmd == QCCommand::WindingCW ? QCanvasPainter::PathWinding::ClockWise :
                         QCanvasPainter::PathWinding::CounterClockWise;
            handleSetPathWinding(w);
            break;
        }
        }
    }

    // Start with max invalid bounds
    float boundX = FLT_MAX;
    float boundY = FLT_MAX;
    float boundW = -FLT_MAX;
    float boundH = -FLT_MAX;
    const int pCount = ctx.pathsCount;
    const bool windingEnforce = m_renderer->testFlag(QCPainterRhiRenderer::WindingEnforce);
    // The signed area of the paths without the winding enforcement
    float area = 0;
    for (int j = 0; j < pCount; j++) {
        QCPath &path = ctx.paths[j];
        if (path.pointsCount == 0)
            continue;
        // If the first and last points are the same, remove the last, mark as closed path.
        // In expandStroke() the closed paths will then add vertices to beginning.
        QCPoint ptFirst = ctx.points.at(path.pointsOffset);
        QCPoint ptLast = ctx.points.at(path.pointsOffset + path.pointsCount - 1);
        if (pointsEquals(ptLast.x, ptLast.y, ptFirst.x, ptFirst.y, ctx.distTol)) {
            path.pointsCount--;
            path.isClosed = true;
        }

        if (windingEnforce)
            path.isReversed = enforceWinding(path.pointsOffset, path.pointsCount, path.winding,
                                             path.isClosed);
        else
            area += pointsArea(ctx.points, path.pointsOffset, path.pointsCount);
    }

    // Without the winding enforcement the paths keep their direction. A fill
    // and its antialiasing fringe are made for the paths going counterclockwise,
    // like the solid ones of the enforcement, so a fill going clockwise would
    // get the fringe on the wrong side of its edges, leaving a gap between it
    // and the fill, and would not be convex. Reversing all the paths doesn't
    // change which pixels a fill covers, with either fill rule, or how a
    // stroke looks, as the dashes follow the order of drawing, see
    // QCPath::isReversed. So they are reversed when they go clockwise in total.
    if (area < 0.0f) {
        for (int j = 0; j < pCount; j++) {
            QCPath &path = ctx.paths[j];
            reversePoints(ctx.points, path.pointsOffset, path.pointsCount, path.isClosed);
            path.isReversed = path.pointsCount > 1;
        }
    }

    // Calculate the direction and length of line segments.
    for (int j = 0; j < pCount; j++) {
        QCPath &path = ctx.paths[j];
        if (path.pointsCount == 0)
            continue;
        int p0Index = path.pointsOffset + path.pointsCount - 1;
        int p1Index = path.pointsOffset;
        for (int i = 0; i < path.pointsCount; i++) {
            QCPoint &p0 = ctx.points[p0Index];
            const QCPoint &p1 = ctx.points[p1Index];
            // Calculate segment direction and length
            p0.dx = p1.x - p0.x;
            p0.dy = p1.y - p0.y;
            p0.len = normalizePoint(&p0.dx, &p0.dy);
            // Update bounds
            boundX = std::min(boundX, p0.x);
            boundY = std::min(boundY, p0.y);
            boundW = std::max(boundW, p0.x);
            boundH = std::max(boundH, p0.y);
            p0Index = path.pointsOffset + i;
            p1Index = p0Index + 1;
        }
    }
    ctx.bounds.setRect(boundX, boundY, boundW, boundH);

#ifdef QCPAINTER_PERF_DEBUG
    perf.logEnd(QCPerfLogging::C2P);
#endif
}

// The maximum number of dashes in a stroke. With more, for example with a very
// short dash pattern on a long path, the stroke is drawn without dashes.
static constexpr int QCPAINTER_MAX_DASH_COUNT = 100000;

// Splits the paths into dashes with the dash pattern of the state, like the
// HTML canvas setLineDash(). The dashes are open paths in ctx.dashPaths and
// ctx.dashPoints, stroked with caps and joins like other paths. The pattern
// starts at the start of each subpath, in the direction it was drawn, shifted
// by the dash offset. On closed subpaths, the dash crossing the start point is
// one dash, but a dash covering the whole subpath has caps at the start point.
// Zero-length dashes are dots with round and square caps.
// The lengths of the pattern are in the user space. When pointsTransformed is
// true, the points are transformed with the state transform, and the lengths
// of the segments are measured in the user space with its inverse, so that the
// dashes follow also a non-uniform scale or a skew. pixelSize is the size of a
// device pixel in the units of the points, for the tolerances. halfWidth is
// half of the stroke width in the units of the points.
// Returns false when the paths are stroked without dashes.
bool QCPainterEngine::dashPaths(bool pointsTransformed, float pixelSize, float halfWidth)
{
    const QList<qreal> &lineDash = state.lineDash;
    if (lineDash.isEmpty())
        return false;
    QVarLengthArray<float, 16> pattern(lineDash.size());
    float patternLength = 0.0f;
    double exactPatternLength = 0.0;
    for (qsizetype i = 0; i < lineDash.size(); ++i) {
        pattern[i] = float(lineDash.at(i));
        patternLength += pattern[i];
        exactPatternLength += lineDash.at(i);
    }
    // All zero lengths draw solid, like in the HTML canvas
    if (!(patternLength > 0.0f && patternLength <= FLT_MAX))
        return false;

    // The inverse of the linear part of the transform, which maps the segment
    // directions to the user space. With a rotation and a uniform scale, all
    // directions have the same scale, userScale.
    float ia = 1.0f;
    float ib = 0.0f;
    float ic = 0.0f;
    float id = 1.0f;
    float userScale = 1.0f;
    bool uniformScale = true;
    if (pointsTransformed) {
        const QTransform &t = state.transform;
        const double det = t.m11() * t.m22() - t.m12() * t.m21();
        // Collapsed, nothing to dash
        if (!(std::abs(det) > 1.0e-12 && qIsFinite(det)))
            return false;
        uniformScale = (t.m11() == t.m22() && t.m12() == -t.m21())
                       || (t.m11() == -t.m22() && t.m12() == t.m21());
        if (uniformScale) {
            userScale = float(1.0 / std::sqrt(std::abs(det)));
        } else {
            ia = float(t.m22() / det);
            ib = float(-t.m12() / det);
            ic = float(-t.m21() / det);
            id = float(t.m11() / det);
        }
    }

    ctx.dashPointsCount = 0;
    ctx.dashPathsCount = 0;
    // Counts the ends of dashes and gaps, to limit the work also when the
    // dashes are too short to add points
    int dashEndCount = 0;
    const bool skipEmptyDashes = state.lineCap == QCanvasPainter::LineCap::Butt;

    auto beginDash = [this]() {
        if (ctx.dashPathsCount + 1 > ctx.dashPaths.size())
            ctx.dashPaths.resize((ctx.dashPathsCount + 1) + ctx.dashPaths.size() / 2);
        QCPath &dash = ctx.dashPaths[ctx.dashPathsCount++];
        dash = {};
        dash.pointsOffset = ctx.dashPointsCount;
    };
    // Adds a point to the current dash, skipping duplicates
    const float distTol = ctx.distTol * pixelSize;
    auto addPoint = [this, distTol](float x, float y, quint8 flags) {
        QCPath &dash = ctx.dashPaths[ctx.dashPathsCount - 1];
        if (dash.pointsCount > 0) {
            QCPoint &last = ctx.dashPoints[ctx.dashPointsCount - 1];
            if (pointsEquals(last.x, last.y, x, y, distTol)) {
                last.flags |= flags;
                return;
            }
        }
        if (ctx.dashPointsCount + 1 > ctx.dashPoints.size())
            ctx.dashPoints.resize((ctx.dashPointsCount + 1) + ctx.dashPoints.size() / 2);
        QCPoint &pt = ctx.dashPoints[ctx.dashPointsCount++];
        pt = {};
        pt.x = x;
        pt.y = y;
        pt.flags = flags;
        dash.pointsCount++;
    };
    // Ends the current dash. A dash of one point is a dot in direction
    // (dx, dy), or removed with butt caps.
    auto endDash = [&](float dx, float dy) {
        QCPath &dash = ctx.dashPaths[ctx.dashPathsCount - 1];
        if (dash.pointsCount >= 2)
            return;
        if (dash.pointsCount == 0 || skipEmptyDashes || (dx == 0.0f && dy == 0.0f)) {
            ctx.dashPointsCount -= dash.pointsCount;
            ctx.dashPathsCount--;
            return;
        }
        // Two points very close to each other give the direction of the caps.
        // Relative to the width, so that the caps are the same at all scales.
        const QCPoint p = ctx.dashPoints[ctx.dashPointsCount - 1];
        const float halfLength = 1.0e-3f * halfWidth;
        ctx.dashPointsCount--;
        dash.pointsCount = 0;
        if (ctx.dashPointsCount + 2 > ctx.dashPoints.size())
            ctx.dashPoints.resize((ctx.dashPointsCount + 2) + ctx.dashPoints.size() / 2);
        for (float sign : { -1.0f, 1.0f }) {
            QCPoint &pt = ctx.dashPoints[ctx.dashPointsCount++];
            pt = {};
            pt.x = p.x + sign * halfLength * dx;
            pt.y = p.y + sign * halfLength * dy;
            pt.flags = PointCorner;
            dash.pointsCount++;
        }
    };

    // The distance into the pattern at the start of each subpath. The offset
    // is wrapped in double precision, so that an offset that keeps growing,
    // for example in an animation, doesn't lose the precision of the dashes.
    double wrappedOffset = std::fmod(state.lineDashOffset, exactPatternLength);
    if (wrappedOffset < 0.0)
        wrappedOffset += exactPatternLength;
    const float startPhase = std::min(float(wrappedOffset), patternLength);

    // Points of the head of a closed subpath, until the first gap, joined
    // to the last dash at the end
    QVarLengthArray<QCPoint, 64> head;

    for (int i = 0; i < ctx.pathsCount; ++i) {
        const QCPath &path = ctx.paths.at(i);
        const int n = path.pointsCount;
        if (n < 2)
            continue;
        // The points in the order they were drawn, before they were reversed
        auto pointAt = [&](int k) -> const QCPoint & {
            const int index = !path.isReversed ? k : (path.isClosed ? (n - k) % n : n - 1 - k);
            return ctx.points.at(path.pointsOffset + index);
        };
        const int segmentCount = path.isClosed ? n : n - 1;

        // Find the dash at the start, with the offset
        float phase = startPhase;
        int index = 0;
        float remaining = 0.0f;
        for (;;) {
            const float length = pattern[index];
            if (phase < length || (phase == 0.0f && length == 0.0f)) {
                remaining = length - phase;
                break;
            }
            phase -= length;
            index = (index + 1) % pattern.size();
        }
        bool on = index % 2 == 0;

        // The head of a closed subpath is collected separately
        const bool collectHead = path.isClosed && on;
        bool inHead = collectHead;
        head.clear();

        const QCPoint &start = pointAt(0);
        if (inHead)
            head.append(start);
        else if (on) {
            beginDash();
            addPoint(start.x, start.y, PointCorner);
        }

        float dx = 0.0f;
        float dy = 0.0f;
        for (int k = 0; k < segmentCount; ++k) {
            const QCPoint &a = pointAt(k);
            const QCPoint &b = pointAt((k + 1) % n);
            // commandsToPaths() has the direction and length of the segment
            // in the point before it in the order of the points
            float length;
            if (!path.isReversed) {
                dx = a.dx;
                dy = a.dy;
                length = a.len;
            } else {
                dx = -b.dx;
                dy = -b.dy;
                length = b.len;
            }
            // The distance of the points for a distance in the user space
            float toPoints = 1.0f / userScale;
            if (!uniformScale) {
                const float ux = ia * dx + ic * dy;
                const float uy = ib * dx + id * dy;
                toPoints = 1.0f / std::sqrt(ux * ux + uy * uy);
            }
            // The length in the user space
            const float segmentLength = length / toPoints;
            float t = 0.0f;
            while (segmentLength - t > remaining) {
                // The current dash or gap ends in this segment
                if (Q_UNLIKELY(++dashEndCount > 2 * QCPAINTER_MAX_DASH_COUNT))
                    return false;
                t += remaining;
                const float x = a.x + dx * (t * toPoints);
                const float y = a.y + dy * (t * toPoints);
                if (on) {
                    if (inHead) {
                        QCPoint pt = {};
                        pt.x = x;
                        pt.y = y;
                        pt.flags = PointCorner;
                        head.append(pt);
                        inHead = false;
                    } else {
                        addPoint(x, y, PointCorner);
                        endDash(dx, dy);
                    }
                } else {
                    beginDash();
                    addPoint(x, y, PointCorner);
                }
                index = (index + 1) % pattern.size();
                on = !on;
                remaining = pattern[index];
            }
            remaining -= segmentLength - t;
            if (on) {
                // The segment end is a point of the dash, keeping its flags
                // for the joins
                const quint8 flags = (k == segmentCount - 1 && !path.isClosed) ? quint8(PointCorner) : b.flags;
                if (inHead)
                    head.append(b);
                else
                    addPoint(b.x, b.y, flags);
            }
        }

        if (inHead) {
            // A dash covers the whole closed subpath. Like in the HTML canvas,
            // it is not joined at the start point, but has caps there.
            beginDash();
            for (const QCPoint &pt : std::as_const(head))
                addPoint(pt.x, pt.y, pt.flags);
            endDash(dx, dy);
            continue;
        }
        if (collectHead) {
            // Join the head to the last dash, or make it a dash of its own
            if (!on)
                beginDash();
            for (qsizetype h = on ? 1 : 0; h < head.size(); ++h)
                addPoint(head.at(h).x, head.at(h).y, head.at(h).flags);
            endDash(dx, dy);
        } else if (on) {
            endDash(dx, dy);
        }
    }

    // Segment directions and lengths, like in commandsToPaths()
    for (int i = 0; i < ctx.dashPathsCount; ++i) {
        const QCPath &dash = ctx.dashPaths.at(i);
        for (int k = 0; k < dash.pointsCount; ++k) {
            QCPoint &p0 = ctx.dashPoints[dash.pointsOffset + k];
            const QCPoint &p1 = ctx.dashPoints[dash.pointsOffset + (k + 1) % dash.pointsCount];
            p0.dx = p1.x - p0.x;
            p0.dy = p1.y - p0.y;
            p0.len = normalizePoint(&p0.dx, &p0.dy);
        }
    }
    return true;
}

// Customized version of NanoVG bezier tesselation (https://github.com/memononen/nanovg)
void QCPainterEngine::tesselateBezier(float x1, float y1, float x2, float y2,
                                         float x3, float y3, float x4, float y4,
                                         int level, QCPointFlags flags)
{
    const float dx = x4 - x1;
    const float dy = y4 - y1;
    const float d2 = std::abs((x2 - x4) * dy - (y2 - y4) * dx);
    const float d3 = std::abs((x3 - x4) * dy - (y3 - y4) * dx);
    const float d23 = d2 + d3;
    // At the deepest level the segment is added even when it is not flat
    // enough, so that the curve still reaches its end point. That should only
    // happen with huge curves, like a circle with a radius of millions of
    // pixels, where the segments are then a bit less accurate. A curve with
    // points that are not finite has d23 that is not finite, and would never
    // get flat, so it is added as one segment. These are checked only when
    // the segment is not flat, so that the common case stays fast.
    if (d23 * d23 < ctx.tessTol * (dx * dx + dy * dy)
        || Q_UNLIKELY(level >= QCPAINTER_MAX_TESSELLATION_LEVEL || !(d23 <= FLT_MAX))) {
        handleAddPoint(x4, y4, flags);
        return;
    }

    const float x12 = (x1 + x2) * 0.5f;
    const float y12 = (y1 + y2) * 0.5f;
    const float x23 = (x2 + x3) * 0.5f;
    const float y23 = (y2 + y3) * 0.5f;
    const float x34 = (x3 + x4) * 0.5f;
    const float y34 = (y3 + y4) * 0.5f;
    const float x234 = (x23 + x34) * 0.5f;
    const float y234 = (y23 + y34) * 0.5f;
    const float x123 = (x12 + x23) * 0.5f;
    const float y123 = (y12 + y23) * 0.5f;
    const float x1234 = (x123 + x234) * 0.5f;
    const float y1234 = (y123 + y234) * 0.5f;

    tesselateBezier(x1, y1, x12, y12, x123, y123, x1234, y1234, (level + 1), {});
    tesselateBezier(x1234, y1234, x234, y234, x34, y34, x4, y4, (level + 1), flags);
}

// pixelSize is the size of a device pixel in the units of the points
void QCPainterEngine::expandFill(float pixelSize)
{
    const float aa = (m_renderer->testFlag(QCPainterRhiRenderer::Antialiasing) ? state.antialias : 0.0f)
                     * pixelSize;
    // Hardcoded miterLimit for fill.
    const float miterLimit = 2.4f;
    // Inner bevels only on the outer side of concave corners. On convex
    // corners the fringe widths are limited below instead.
    calculateJoins(ctx.paths, ctx.pathsCount, ctx.points, aa, QCanvasPainter::LineJoin::Miter, miterLimit,
                   true);
    const int pCount = ctx.pathsCount;
    const bool useEdgeAA = aa > 0.0f;

    // Calculate max vertex usage
    int vertsCount = 0;
    for (int i = 0; i < pCount; i++) {
        const QCPath &path = ctx.paths.at(i);
        vertsCount += path.pointsCount + path.bevelCount + 1;
        if (useEdgeAA)
            vertsCount += (path.pointsCount + path.bevelCount * 5 + 1) * 2; // plus one for loop
    }
    // And ensure we have enough available
    ensureVertices(vertsCount);

    const bool isConvex = (pCount == 1 && ctx.paths.first().isConvex);
    const float halfAA = 0.5f * aa;
    // setAntialias(1) in the units of the points
    const float defaultAA = QCPAINTER_ANTIALIAS_MULTIPLIER / ctx.devicePxRatio * pixelSize;

    // The fringe goes from halfAA inside the edges to halfAA outside. Moving
    // a point along its miter dm moves it towards its neighbors by
    // dot(dm, dir) per unit, so limit the offset on that side to half of the
    // shorter edge. Otherwise the fill and the fringe fold over.
    auto fringeWidths = [halfAA](const QCPoint &p0, const QCPoint &p1,
                                 float *inner, float *outer) {
        const float along = p1.dmx * p1.dx + p1.dmy * p1.dy;
        const float halfLen = 0.5f * std::min(p0.len, p1.len);
        *inner = halfAA;
        *outer = halfAA;
        if (along * halfAA > halfLen)
            *inner = halfLen / along;
        else if (-along * halfAA > halfLen && !(p1.flags & PointInnerBevel))
            *outer = halfLen / -along;
    };

    // Convex paths are drawn without stenciling, so the fill must stay
    // convex: inset all points by the smallest limit, and at most by the
    // distance from the average point to the closest edge. One short edge
    // with a turn then makes the fringe of the whole shape asymmetric, moving
    // its edges outward by up to a quarter of the antialias width, which is
    // accepted to keep drawing it without stenciling.
    float convexInset = halfAA;
    if (isConvex && useEdgeAA) {
        const QCPath &path = ctx.paths.first();
        float cx = 0.0f;
        float cy = 0.0f;
        for (int j = 0; j < path.pointsCount; ++j) {
            const QCPoint &pt = ctx.points[path.pointsOffset + j];
            cx += pt.x;
            cy += pt.y;
        }
        if (path.pointsCount > 0) {
            cx /= path.pointsCount;
            cy /= path.pointsCount;
        }
        int p0Index = path.pointsOffset + path.pointsCount - 1;
        for (int j = 0; j < path.pointsCount; ++j) {
            const int p1Index = path.pointsOffset + j;
            const QCPoint &p0 = ctx.points[p0Index];
            const QCPoint &p1 = ctx.points[p1Index];
            float inner, outer;
            fringeWidths(p0, p1, &inner, &outer);
            // Distance from the center to the line of the edge
            const float dist = std::abs((cx - p1.x) * p1.dy - (cy - p1.y) * p1.dx);
            convexInset = std::min({ convexInset, inner, dist });
            p0Index = p1Index;
        }
    }
    auto fringeWidthsForPath = [&](const QCPoint &p0, const QCPoint &p1,
                                   float *inner, float *outer) {
        if (isConvex) {
            *inner = convexInset;
            *outer = halfAA;
        } else {
            fringeWidths(p0, p1, inner, outer);
        }
    };

    for (int i = 0; i < pCount; i++) {
        QCPath &path = ctx.paths[i];
        path.fillOffset = ctx.verticesCount;
        // Calculate shape vertices.
        if (useEdgeAA) {
            // Looping
            int p0Index = path.pointsOffset + path.pointsCount - 1;
            int p1Index = path.pointsOffset;
            for (int j = 0; j < path.pointsCount; ++j) {
                const QCPoint &p0 = ctx.points[p0Index];
                const QCPoint &p1 = ctx.points[p1Index];
                float inset, outset;
                fringeWidthsForPath(p0, p1, &inset, &outset);
                if (p1.flags & PointBevel) {
                    if (p1.flags & PointLeft) {
                        float lx = p1.x + p1.dmx * inset;
                        float ly = p1.y + p1.dmy * inset;
                        addVert(lx, ly, 0.5f, 1.0f);
                    } else {
                        float lx0 = p1.x + p0.dy * inset;
                        float ly0 = p1.y - p0.dx * inset;
                        addVert(lx0, ly0, 0.5f, 1.0f);
                        float lx1 = p1.x + p1.dy * inset;
                        float ly1 = p1.y - p1.dx * inset;
                        addVert(lx1, ly1, 0.5f, 1.0f);
                    }
                } else {
                    float lx = p1.x + p1.dmx * inset;
                    float ly = p1.y + p1.dmy * inset;
                    addVert(lx, ly, 0.5f, 1.0f);
                }
                p0Index = path.pointsOffset + j;
                p1Index = p0Index + 1;
            }
        } else {
            for (int j = 0; j < path.pointsCount; ++j) {
                QCPoint pts = ctx.points.at(path.pointsOffset + j);
                addVert(pts.x, pts.y, 0.5f, 1.0f);
            }
        }

        path.fillCount = ctx.verticesCount - path.fillOffset;

        if (useEdgeAA) {
            // Calculate antialiasing. The fringe is opaque at the fill inset
            // and transparent outside the shape. For non-convex paths, which
            // are stenciled, it extends further inside to cover thin parts,
            // fading out above the default antialias value.
            const float maxExtension = isConvex
                    ? 0.0f
                    : std::max(0.0f, std::min(aa, 2.0f * defaultAA - aa));
            const float ru = 1.0f;

            const int loopIndex = ctx.verticesCount;
            path.strokeOffset = ctx.verticesCount;
            // Looping
            int p0Index = path.pointsOffset + path.pointsCount - 1;
            int p1Index = path.pointsOffset;

            for (int j = 0; j < path.pointsCount; ++j) {
                const QCPoint &p0 = ctx.points[p0Index];
                const QCPoint &p1 = ctx.points[p1Index];
                float lw, rw;
                fringeWidthsForPath(p0, p1, &lw, &rw);
                // u is 0.5 at the fill inset
                const float extension = std::min(maxExtension, lw + rw);
                const float lu = (0.5f * (lw + rw) - 0.5f * extension) / (lw + rw);
                lw += extension;
                if ((p1.flags & (PointBevel | PointInnerBevel))) {
                    addBevelJoin(p0, p1, lw, rw, lu, ru);
                } else {
                    addVert(p1.x + (p1.dmx * lw), p1.y + (p1.dmy * lw), lu, 1.0f);
                    addVert(p1.x - (p1.dmx * rw), p1.y - (p1.dmy * rw), ru, 1.0f);
                }
                p0Index = path.pointsOffset + j;
                p1Index = p0Index + 1;
            }

            if (path.pointsCount > 0) {
                // Loop the fill
                const auto &verts = ctx.vertices;
                addVert(verts.at(loopIndex).x, verts.at(loopIndex).y, verts.at(loopIndex).u, 1.0f);
                addVert(verts.at(loopIndex + 1).x, verts.at(loopIndex + 1).y, ru, 1.0f);
            }

            path.strokeCount = ctx.verticesCount - path.strokeOffset;
        } else {
            // No antialiasing
            path.strokeOffset = 0;
            path.strokeCount = 0;
        }
    }
}

// Join and cap calculations are customized version based on NanoVG (https://github.com/memononen/nanovg)
void QCPainterEngine::expandStroke(QCPaths &paths, int pathsCount, QCPoints &points,
                                   float w, QCanvasPainter::LineCap cap, QCanvasPainter::LineJoin join,
                                   float miterLimit, float pixelSize)
{
    // On entry w is half of stroke width and on return half of stroke width + aa.
    // Both and pixelSize, the size of a device pixel, are in the units of the points.
    const float aa = (m_renderer->testFlag(QCPainterRhiRenderer::Antialiasing) ? state.antialias : 0.0f)
                     * pixelSize;
    w += aa * 0.5f;
    calculateJoins(paths, pathsCount, points, w, join, miterLimit);
    const int pCount = pathsCount;
    const int roundDivs = (join == QCanvasPainter::LineJoin::Round) || (cap == QCanvasPainter::LineCap::Round)
                              ? curveDivs(w / pixelSize, ctx.divsTol) // the tolerance is in device pixels
                              : 0;

    // Calculate max vertex usage
    int vertsCount = 0;
    for (int i = 0; i < pCount; i++) {
        const QCPath &path = paths.at(i);
        if (join == QCanvasPainter::LineJoin::Round)
            vertsCount += (path.pointsCount + path.bevelCount * (roundDivs + 2) + 1) * 2; // plus one for loop
        else
            vertsCount += (path.pointsCount + path.bevelCount * 5 + 1) * 2; // plus one for loop
        if (!path.isClosed) {
            // Not looping, so need vertices for caps
            if (cap == QCanvasPainter::LineCap::Round) {
                vertsCount += (roundDivs * 2 + 2) * 2;
            } else {
                vertsCount += (3 + 3) * 2;
            }
        }
    }
    // And ensure we have enough available
    ensureVertices(vertsCount);

    float u0 = 0.0f;
    float u1 = 1.0f;
    if (qFuzzyIsNull(aa)) {
        // When not antialiased, disable antialiasing gradient.
        u0 = 0.5f;
        u1 = 0.5f;
    }

    for (int i = 0; i < pCount; i++) {
        QCPath &path = paths[i];
        path.fillOffset = 0;
        path.fillCount = 0;
        path.strokeOffset = ctx.verticesCount;
        // Default to looping
        int startPoint = 0;
        int endPoint = path.pointsCount;
        int p0Index = path.pointsOffset + path.pointsCount - 1;
        int p1Index = path.pointsOffset;
        const int loopIndex = ctx.verticesCount;
        if (!path.isClosed) {
            // Not looping, so add start cap
            startPoint = 1;
            endPoint = path.pointsCount - 1;
            p0Index = path.pointsOffset;
            p1Index = p0Index + 1;
            const QCPoint &p0 = points[p0Index];
            const QCPoint &p1 = points[p1Index];

            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            normalizePoint(&dx, &dy);
            if (cap == QCanvasPainter::LineCap::Butt)
                addButtCapStart(p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (cap == QCanvasPainter::LineCap::Square)
                addButtCapStart(p0, dx, dy, w, w - aa, aa, u0, u1);
            else if (cap == QCanvasPainter::LineCap::Round)
                addRoundCapStart(p0, dx, dy, w, roundDivs, u0, u1);
        }

        for (int j = startPoint; j < endPoint; ++j) {
            // Add joins
            const QCPoint &p0 = points[p0Index];
            const QCPoint &p1 = points[p1Index];
            if (p1.flags & (PointBevel | PointInnerBevel)) {
                if (join == QCanvasPainter::LineJoin::Round) {
                    addRoundJoin(p0, p1, w, w, u0, u1, roundDivs);
                } else {
                    addBevelJoin(p0, p1, w, w, u0, u1);
                }
            } else {
                addVert(p1.x + (p1.dmx * w), p1.y + (p1.dmy * w), u0, 1.0f);
                addVert(p1.x - (p1.dmx * w), p1.y - (p1.dmy * w), u1, 1.0f);
            }
            p0Index = path.pointsOffset + j;
            p1Index = p0Index + 1;
        }

        if (path.isClosed) {
            // Loop the stroke
            const auto &verts = ctx.vertices;
            addVert(verts.at(loopIndex).x, verts.at(loopIndex).y, u0, 1.0f);
            addVert(verts.at(loopIndex + 1).x, verts.at(loopIndex + 1).y, u1, 1.0f);
        } else {
            // Add end cap
            const QCPoint &p0 = points[p0Index];
            const QCPoint &p1 = points[p1Index];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            normalizePoint(&dx, &dy);
            if (cap == QCanvasPainter::LineCap::Butt)
                addButtCapEnd(p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (cap == QCanvasPainter::LineCap::Square)
                addButtCapEnd(p1, dx, dy, w, w - aa, aa, u0, u1);
            else if (cap == QCanvasPainter::LineCap::Round)
                addRoundCapEnd(p1, dx, dy, w, roundDivs, u0, u1);
        }
        path.strokeCount = ctx.verticesCount - path.strokeOffset;
    }
}

void QCPainterEngine::calculateJoins(QCPaths &paths, int pathsCount, QCPoints &points,
                                     float w, QCanvasPainter::LineJoin join, float miterLimit,
                                     bool concaveInnerBevelsOnly)
{
    // Inverse of width
    const float iw = (w > 0.0f) ? 1.0f / w : 0.0f;

    const int pCount = pathsCount;
    // Calculate which joins needs extra vertices to append, and gather vertex count.
    for (int i = 0; i < pCount; i++) {
        QCPath &path = paths[i];
        const int pointsCount = path.pointsCount;
        if (Q_UNLIKELY(pointsCount == 0))
            continue;

        int p0Index = path.pointsOffset + pointsCount - 1;
        int p1Index = path.pointsOffset;
        int leftCount = 0;
        int bevelCount = 0;
#if QCPAINTER_STRICT_CONVEX_PATH_CHECK
        bool prevPosX = points[p0Index].dx > 0;
        bool prevPosY = points[p0Index].dy > 0;
        int xFlips = 0;
        int yFlips = 0;
#endif

        for (int j = 0; j < pointsCount; j++) {
            const QCPoint &p0 = points[p0Index];
            QCPoint &p1 = points[p1Index];
#if QCPAINTER_STRICT_CONVEX_PATH_CHECK
            // Count the number of times the path changes direction
            const bool posX = p1.dx > 0;
            const bool posY = p1.dy > 0;
            if (posX != prevPosX)
                xFlips++;
            if (posY != prevPosY)
                yFlips++;
            prevPosX = posX;
            prevPosY = posY;
#endif
            // Calculate extrusions
            p1.dmx = (p0.dy + p1.dy) * 0.5f;
            p1.dmy = (-p0.dx - p1.dx) * 0.5f;
            float dmr2 = p1.dmx * p1.dmx + p1.dmy * p1.dmy;
            if (dmr2 > 0.001f) {
                float scale = 1.0f / dmr2;
                p1.dmx *= scale;
                p1.dmy *= scale;
            }

            // Clear other flags except the corner.
            p1.flags = (p1.flags & PointCorner) ? PointCorner : 0;

            // Keep track of left turns.
            float cross = p1.dx * p0.dy - p0.dx * p1.dy;
            if (cross > 0.0f) {
                leftCount++;
                p1.flags |= PointLeft;
            }

            // Calculate if we should use bevel or miter for inner join,
            // never with zero width.
            if (w > 0.0f && !(concaveInnerBevelsOnly && (p1.flags & PointLeft))) {
                float limit = std::max(1.01f, std::min(p0.len, p1.len) * iw);
                if ((dmr2 * limit * limit) < 1.0f)
                    p1.flags |= PointInnerBevel;
            }

            // Check if the corner needs to be beveled.
            if (p1.flags & PointCorner) {
                if (join == QCanvasPainter::LineJoin::Bevel || join == QCanvasPainter::LineJoin::Round || (dmr2 * miterLimit * miterLimit) < 1.0f ) {
                    p1.flags |= PointBevel;
                }
            }

            if ((p1.flags & (PointBevel | PointInnerBevel)))
                bevelCount++;

            p0Index = path.pointsOffset + j;
            p1Index = p0Index + 1;
        }
#if QCPAINTER_STRICT_CONVEX_PATH_CHECK
        // A truly convex path with no self-intersections will only change direction
        // twice along the x-axis and twice along the y-axis
        path.isConvex = leftCount == pointsCount && xFlips <= 2 && yFlips <= 2;
#else
        path.isConvex = leftCount == pointsCount;
#endif
        path.bevelCount = bevelCount;
    }
}

static void adjustBevel(bool innerBevel, const QCPoint &p0, const QCPoint &p1, float w,
                        float* x0, float* y0, float* x1, float* y1)
{
    if (innerBevel) {
        *x0 = p1.x + p0.dy * w;
        *y0 = p1.y - p0.dx * w;
        *x1 = p1.x + p1.dy * w;
        *y1 = p1.y - p1.dx * w;
    } else {
        *x0 = p1.x + p1.dmx * w;
        *y0 = p1.y + p1.dmy * w;
        *x1 = p1.x + p1.dmx * w;
        *y1 = p1.y + p1.dmy * w;
    }
}

void QCPainterEngine::addBevelJoin(const QCPoint &p0, const QCPoint &p1,
                                      float lw, float rw, float lu, float ru)
{
    // The u of the point itself, 0.5 unless the sides differ in width
    const float pu = (lw + rw) > 0.0f ? lu + (ru - lu) * lw / (lw + rw) : 0.5f;
    float dlx0 = p0.dy;
    float dly0 = -p0.dx;
    float dlx1 = p1.dy;
    float dly1 = -p1.dx;
    if (p1.flags & PointLeft) {
        float lx0, ly0, lx1, ly1;
        adjustBevel(p1.flags & PointInnerBevel, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);
        addVert(lx0, ly0, lu, 1.0f);
        addVert(p1.x - (dlx0 * rw), p1.y - (dly0 * rw), ru, 1.0f);

        if (p1.flags & PointBevel) {
#ifdef QCPAINTER_FAST_BEVEL_CALCULATIONS
            addVert(lx0, ly0, lu, 1.0f);
            addVert(p1.x - (dlx0 * rw), p1.y - (dly0 * rw), ru, 1.0f);
#else
            float a0 = std::atan2(-dly0, -dlx0);
            float a1 = std::atan2(-dly1, -dlx1);
            if (a1 > a0) a1 -= float(M_PI) * 2;
            for (int i = 0; i < 2; i++) {
                // No extra divs with bevel joins
                float a = a0 + float(i) * (a1 - a0);
                float rx = p1.x + std::cos(a) * rw;
                float ry = p1.y + std::sin(a) * rw;
                addVert(p1.x, p1.y, pu, 1.0f);
                addVert(rx, ry, ru, 1.0f);
            }
#endif
            addVert(lx1, ly1, lu, 1.0f);
            addVert(p1.x - (dlx1 * rw), p1.y - (dly1 * rw), ru, 1.0f);
        } else {
            float rx0 = p1.x - (p1.dmx * rw);
            float ry0 = p1.y - (p1.dmy * rw);
            addVert(p1.x, p1.y, pu, 1.0f);
            addVert(p1.x - (dlx0 * rw), p1.y - (dly0 * rw), ru, 1.0f);
            addVert(rx0, ry0, ru, 1.0f);
            addVert(rx0, ry0, ru, 1.0f);
            addVert(p1.x, p1.y, pu, 1.0f);
            addVert(p1.x - (dlx1 * rw), p1.y - (dly1 * rw), ru, 1.0f);
        }
        addVert(lx1, ly1, lu, 1.0f);
        addVert(p1.x - (dlx1 * rw), p1.y - (dly1 * rw), ru, 1.0f);
    } else {
        float rx0, ry0, rx1, ry1;
        adjustBevel(p1.flags & PointInnerBevel, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);
        addVert(p1.x + (dlx0 * lw), p1.y + (dly0 * lw), lu, 1.0f);
        addVert(rx0, ry0, ru, 1.0f);

        if (p1.flags & PointBevel) {
#ifdef QCPAINTER_FAST_BEVEL_CALCULATIONS
            addVert(p1.x + (dlx0 * lw), p1.y + (dly0 * lw), lu, 1.0f);
            addVert(rx0, ry0, ru, 1.0f);
#else
            float a0 = std::atan2(dly0, dlx0);
            float a1 = std::atan2(dly1, dlx1);
            if (a1 < a0) a1 += float(M_PI) * 2;
            for (int i = 0; i < 2; i++) {
                // No extra divs with bevel joins
                float a = a0 + float(i) * (a1 - a0);
                float lx = p1.x + std::cos(a) * lw;
                float ly = p1.y + std::sin(a) * lw;
                addVert(lx, ly, lu, 1.0f);
                addVert(p1.x, p1.y, pu, 1.0f);
            }
#endif
            addVert(p1.x + (dlx1 * lw), p1.y + (dly1 * lw), lu, 1.0f);
            addVert(rx1, ry1, ru, 1.0f);
        } else {
            float lx0 = p1.x + (p1.dmx * lw);
            float ly0 = p1.y + (p1.dmy * lw);
            addVert(p1.x + (dlx0 * lw), p1.y + (dly0 * lw), lu, 1.0f);
            addVert(p1.x, p1.y, pu, 1.0f);

            addVert(lx0, ly0, lu, 1.0f);
            addVert(lx0, ly0, lu, 1.0f);

            addVert(p1.x + (dlx1 * lw), p1.y + (dly1 * lw), lu, 1.0f);
            addVert(p1.x, p1.y, pu, 1.0f);
        }
        addVert(p1.x + (dlx1 * lw), p1.y + (dly1 * lw), lu, 1.0f);
        addVert(rx1, ry1, ru, 1.0f);
    }
}

void QCPainterEngine::addRoundJoin(const QCPoint &p0, const QCPoint &p1,
                                      float lw, float rw, float lu, float ru,
                                      int roundDivs)
{
    float dlx0 = p0.dy;
    float dly0 = -p0.dx;
    float dlx1 = p1.dy;
    float dly1 = -p1.dx;
    if (p1.flags & PointLeft) {
        float lx0, ly0, lx1, ly1;
        adjustBevel(p1.flags & PointInnerBevel, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);
        float a0 = std::atan2(-dly0, -dlx0);
        float a1 = std::atan2(-dly1, -dlx1);
        if (a1 > a0) a1 -= float(M_PI) * 2;

        addVert(lx0, ly0, lu, 1.0f);
        addVert(p1.x - (dlx0 * rw), p1.y - (dly0 * rw), ru, 1.0f);

        int divs = qCeil((a0 - a1) / M_PI) * roundDivs;
        divs = std::clamp(divs, 2, roundDivs);
        for (int i = 0; i < divs; i++) {
            float u = i / float(divs - 1);
            float a = a0 + u * (a1 - a0);
            float rx = p1.x + std::cos(a) * rw;
            float ry = p1.y + std::sin(a) * rw;
            addVert(p1.x, p1.y, 0.5f, 1.0f);
            addVert(rx, ry, ru, 1.0f);
        }

        addVert(lx1, ly1, lu, 1.0f);
        addVert(p1.x - dlx1 * rw, p1.y - dly1 * rw, ru, 1.0f);

    } else {
        float rx0,ry0,rx1,ry1;
        adjustBevel(p1.flags & PointInnerBevel, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);
        float a0 = std::atan2(dly0, dlx0);
        float a1 = std::atan2(dly1, dlx1);
        if (a1 < a0) a1 += float(M_PI) * 2;

        addVert(p1.x + dlx0 * rw, p1.y + dly0 * rw, lu, 1.0f);
        addVert(rx0, ry0, ru, 1.0f);

        int divs = qCeil((a1 - a0) / M_PI) * roundDivs;
        divs = std::clamp(divs, 2, roundDivs);
        for (int i = 0; i < divs; i++) {
            float u = i / float(divs - 1);
            float a = a0 + u * (a1 - a0);
            float lx = p1.x + std::cos(a) * lw;
            float ly = p1.y + std::sin(a) * lw;
            addVert(lx, ly, lu, 1.0f);
            addVert(p1.x, p1.y, 0.5f, 1.0f);
        }

        addVert(p1.x + dlx1 * rw, p1.y + dly1 * rw, lu, 1.0f);
        addVert(rx1, ry1, ru, 1.0f);
    }
}

void QCPainterEngine::addButtCapStart(const QCPoint &p,
                                         float dx, float dy, float w, float d,
                                         float aa, float u0, float u1)
{
    const float px = p.x - dx * d;
    const float py = p.y - dy * d;
    const float dlxw = dy * w;
    const float dlyw = -dx * w;
    addVert(px + dlxw - dx * aa, py + dlyw - dy * aa, u0, 0.0f);
    addVert(px - dlxw - dx * aa, py - dlyw - dy * aa, u1, 0.0f);
    addVert(px + dlxw, py + dlyw, u0, 1.0f);
    addVert(px - dlxw, py - dlyw, u1, 1.0f);
}

void QCPainterEngine::addRoundCapStart(const QCPoint &p,
                                          float dx, float dy, float w, int roundDivs,
                                          float u0, float u1)
{
    const float px = p.x;
    const float py = p.y;
    const float dlx = dy;
    const float dly = -dx;
    for (int i = 0; i < roundDivs; i++) {
        float a = i / float(roundDivs - 1) * M_PI;
        float ax = std::cos(a) * w;
        float ay = std::sin(a) * w;
        addVert(px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1.0);
        addVert(px, py, 0.5f, 1.0f);
    }
    addVert(px + dlx * w, py + dly * w, u0, 1.0f);
    addVert(px - dlx * w, py - dly * w, u1, 1.0f);
}

void QCPainterEngine::addButtCapEnd(const QCPoint &p,
                                       float dx, float dy, float w, float d,
                                       float aa, float u0, float u1)
{
    const float px = p.x + dx * d;
    const float py = p.y + dy * d;
    const float dlxw = dy * w;
    const float dlyw = -dx * w;
    addVert(px + dlxw, py + dlyw, u0, 1.0f);
    addVert(px - dlxw, py - dlyw, u1, 1.0f);
    addVert(px + dlxw + dx * aa, py + dlyw + dy * aa, u0, 0.0f);
    addVert(px - dlxw + dx * aa, py - dlyw + dy * aa, u1, 0.0f);
}

void QCPainterEngine::addRoundCapEnd(const QCPoint &p,
                                        float dx, float dy, float w, int roundDivs,
                                        float u0, float u1)
{
    const float px = p.x;
    const float py = p.y;
    const float dlx = dy;
    const float dly = -dx;
    addVert(px + (dlx * w), py + (dly * w), u0, 1.0f);
    addVert(px - (dlx * w), py - (dly * w), u1, 1.0f);
    for (int i = 0; i < roundDivs; i++) {
        float a = i / float(roundDivs - 1) * M_PI;
        float ax = std::cos(a) * w;
        float ay = std::sin(a) * w;
        addVert(px, py, 0.5f, 1.0f);
        addVert(px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1.0f);
    }
}

// Adds vertex at verticesCount, and increases verticesCount.
inline void QCPainterEngine::addVert(float x, float y, float u, float v) noexcept
{
    auto &vtx = ctx.vertices[ctx.verticesCount++];
    vtx.x = x;
    vtx.y = y;
    vtx.u = u;
    vtx.v = v;
}

// Ensures that there is at least \a count amount of vertices available
void QCPainterEngine::ensureVertices(int count)
{
    auto &verts = ctx.vertices;
    if (ctx.verticesCount + count > verts.size()) {
        // Increase capacity with the vertices amount + 10% of the current size.
        int newSize = (ctx.verticesCount + count) + verts.size() * 0.1;
        verts.resize(newSize);
    }
}

// Prepare current commands to match \a path before the fill/stroke.
void QCPainterEngine::preparePainterPath(const QCanvasPath &path, bool ignoreTransform)
{
    beginPath();
    const QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(&path);
    appendCommandsData(pathd->commandsData.constData(), pathd->commandsDataCount, ignoreTransform);
    appendCommands(pathd->commands.constData(), pathd->commandsCount);
}

// Append \a path into current commands.
void QCPainterEngine::appendPainterPath(const QCanvasPath &path,
                                         const QTransform &transform)
{
    const QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(&path);
    if (transform.isIdentity()) {
        appendCommandsData(pathd->commandsData.constData(), pathd->commandsDataCount);
    } else {
        // Prepare path with the transform
        QTransform prevTransform = state.transform;
        state.transform *= transform;
        appendCommandsData(pathd->commandsData.constData(), pathd->commandsDataCount);
        state.transform = prevTransform;
    }
    appendCommands(pathd->commands.constData(), pathd->commandsCount);
}

// Append \a path into current commands.
// Including \a count amount of commands, starting from \a start.
void QCPainterEngine::appendPainterPath(const QCanvasPath &path,
                                         qsizetype start,
                                         qsizetype count,
                                         const QTransform &transform)
{
    const QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(&path);

    const auto commandsSize = pathd->commandsCount;
    int commandsDataStart = 0;
    int commandsDataCount = 0;
    if (start == 0 && count == commandsSize) {
        // Adding full path
        commandsDataCount = pathd->commandsDataCount;
    } else {
        // Make sure start & count are inside valid range.
        start = qBound(0, start, commandsSize);
        count = qBound(0, count, commandsSize - start);
        if (count == 0)
            return;

        // Calculate commands data amounts, based on the commands start & count.
        const int endCommand = start + count;
        for (int i = 0; i < endCommand; i++) {
            auto dataSize = QCanvasPathPrivate::dataSizeOf(pathd->commands.at(i));
            if (i < start) {
                commandsDataStart += dataSize;
            } else {
                commandsDataCount += dataSize;
            }
        }
    }
    // Note: commandsDataStart and commandsDataCount don't need to be
    // validated as they are always in range when we don't allow raw
    // non-const access into QCanvasPath data.
    const auto commandsData = &pathd->commandsData.at(commandsDataStart);
    if (transform.isIdentity()) {
        appendCommandsData(commandsData, commandsDataCount);
    } else {
        // Prepare path with the transform
        QTransform prevTransform = state.transform;
        state.transform *= transform;
        appendCommandsData(commandsData, commandsDataCount);
        state.transform = prevTransform;
    }
    appendCommands(&pathd->commands.at(start), int(count));
}

// Returns true if path has changed or some state property related
// to fill vertices generation has changed compared to cached path.
// Note: excluding the transform, because cached paths do not bake in the
// transform in the geometry.
bool QCPainterEngine::fillCachedPathUpdateRequired(QCanvasPath *path, int pathGroup)
{
    Q_ASSERT(pathGroup != -1);
    QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(path);
    QCCachedPath &cp = ctx.cachedFillPaths[pathd->serialNumber];
    bool updateRequired = false;
    if (pathGroup != cp.pathGroup
        || pathd->pathIterations != cp.pathIterations
        || pathd->commandsCount != cp.commandsCount
        || !m_renderer->isPathCachedForFill(path, pathGroup, state))
    {
        updateRequired = true;
        cp.pathGroup = pathGroup;
        cp.pathIterations = pathd->pathIterations;
        cp.commandsCount = pathd->commandsCount;
    }
    return updateRequired;
}

// Returns true if path has changed or some state property related
// to stroke vertices generation has changed compared to cached path.
// Note: excluding the transform, because cached paths do not bake in the
// transform in the geometry.
bool QCPainterEngine::strokeCachedPathUpdateRequired(QCanvasPath *path, int pathGroup)
{
    Q_ASSERT(pathGroup != -1);
    QCanvasPathPrivate *pathd = QCanvasPathPrivate::get(path);
    QCCachedPath &cp = ctx.cachedStrokePaths[pathd->serialNumber];
    bool updateRequired = false;
    if (pathGroup != cp.pathGroup
        || pathd->pathIterations != cp.pathIterations
        || pathd->commandsCount != cp.commandsCount
        || !m_renderer->isPathCachedForStroke(path, pathGroup, state))
    {
        updateRequired = true;
        cp.pathGroup = pathGroup;
        cp.pathIterations = pathd->pathIterations;
        cp.commandsCount = pathd->commandsCount;
    }
    return updateRequired;
}

// Return current fillPaint taking into account the state.
// When \a ignoreTransform is true, don't apply state transform
// here as it gets applied in vertex shader.
QCPaint QCPainterEngine::getFillPaint(bool ignoreTransform)
{
    QCPaint fillPaint = state.fill;
    fillPaint.alpha = state.alpha;
    // Apply current transform
    if (!ignoreTransform &&
        fillPaint.brushType != BrushColor &&
        !(state.transform.isIdentity() && state.brushTransform.isIdentity())) {
        fillPaint.transform *= (state.brushTransform * state.transform);
    }
    return fillPaint;
}

//  Return current strokePaint taking into account the state and set \a strokeWidth.
// When \a ignoreTransform is true, don't apply state transform
// here as it gets applied in vertex shader.
QCPaint QCPainterEngine::getStrokePaint(float *strokeWidth, bool ignoreTransform)
{
    float scale = getAverageScale(state.transform);
    *strokeWidth = std::clamp(state.strokeWidth * scale, 0.0f, QCPAINTER_MAX_STROKE_WIDTH);
    float expa = 1.0f;
    if (*strokeWidth < state.antialias) {
        // If the stroke width is less than pixel size, use alpha to emulate coverage.
        float alpha = std::clamp((*strokeWidth * QCPAINTER_ANTIALIAS_MULTIPLIER) / state.antialias, 0.0f, 1.0f);
        // Since coverage is area, scale by alpha*alpha.
        expa = alpha * alpha;
        *strokeWidth = state.antialias;
    }

    QCPaint strokePaint = state.stroke;
    strokePaint.alpha = state.alpha * expa;
    // Apply current transform and optional brush transform
    if (!ignoreTransform &&
        strokePaint.brushType != BrushColor &&
        !(state.transform.isIdentity() && state.brushTransform.isIdentity())) {
        strokePaint.transform *= (state.brushTransform * state.transform);
    }
    return strokePaint;
}

// Return the effective text align, depending on text direction.
// Returned value is either Left, Right or Center, never Start or End.
QCanvasPainter::TextAlign QCPainterEngine::effectiveTextAlign(QStringView text) const
{
    QCanvasPainter::TextAlign align = state.textAlignment;
    auto effectiveDirection = state.textDirection;
#if QT_CONFIG(im)
    bool emptyAuto = effectiveDirection == QCanvasPainter::TextDirection::Auto && text.isEmpty();
    if (emptyAuto || effectiveDirection == QCanvasPainter::TextDirection::Inherit) {
        // Get inherited direction from QGuiApplication. Also used for Auto,
        // when the string is empty. See:
        // https://doc.qt.io/qt-6/qtquick-positioning-righttoleft.html#text-alignment
        if (qGuiApp->isRightToLeft())
            effectiveDirection = QCanvasPainter::TextDirection::RightToLeft;
        else
            effectiveDirection = QCanvasPainter::TextDirection::LeftToRight;
    }
#else
    if (effectiveDirection == QCanvasPainter::TextDirection::Inherit) {
        // No access to inputMethod(), so fail to auto detect
        effectiveDirection = QCanvasPainter::TextDirection::Auto;
    }
#endif
    if (effectiveDirection == QCanvasPainter::TextDirection::Auto) {
        // Get automatic direction from QString isRightToLeft().
        if (text.isRightToLeft())
            effectiveDirection = QCanvasPainter::TextDirection::RightToLeft;
        else
            effectiveDirection = QCanvasPainter::TextDirection::LeftToRight;
    }

    if (effectiveDirection == QCanvasPainter::TextDirection::RightToLeft) {
        if (align == QCanvasPainter::TextAlign::Start) {
            align = QCanvasPainter::TextAlign::Right;
        } else if (align == QCanvasPainter::TextAlign::End) {
            align = QCanvasPainter::TextAlign::Left;
        }
    } else if (align == QCanvasPainter::TextAlign::Start) {
        align = QCanvasPainter::TextAlign::Left;
    } else if (align == QCanvasPainter::TextAlign::End) {
        align = QCanvasPainter::TextAlign::Right;
    }

    return align;
}

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
// *** From QSGDistanceFieldGlyphNode - Start ***

static float thresholdFunc(float glyphScale)
{
    static const float base = qt_qc_envFloat("QT_DF_BASE", 0.5f);
    static const float baseDev = qt_qc_envFloat("QT_DF_BASEDEVIATION", 0.065f);
    static const float devScaleMin = qt_qc_envFloat("QT_DF_SCALEFORMAXDEV", 0.15f);
    static const float devScaleMax = qt_qc_envFloat("QT_DF_SCALEFORNODEV", 0.3f);
    return base - ((qBound(devScaleMin, glyphScale, devScaleMax) - devScaleMin) / (devScaleMax - devScaleMin) * -baseDev + baseDev);
}

static float spreadFunc(float glyphScale)
{
    static const float range = qt_qc_envFloat("QT_DF_RANGE", 0.06f);
    return range / glyphScale;
}

void QCPainterEngine::updateStateFontVars()
{
    auto &font = state.font;
    // Calculating optimal font scale.
    // This is how QSGDistanceFieldGlyphNode does it:
    //const float combinedScale = m_fontScale * m_matrixScale;
    const auto rFont = QRawFont::fromFont(font);
    // TODO: This is a heavy operation. If supported, it should be cached.
    //const bool narrowFont = qt_fontHasNarrowOutlines(rFont);
    const bool narrowFont = false;
    const float baseFontSize = QT_DISTANCEFIELD_BASEFONTSIZE(narrowFont);
    const float combinedScale = rFont.pixelSize() / baseFontSize * ctx.devicePxRatio;
    // Then use it to calculate the optimal smoothstep min & max.
    const float base = thresholdFunc(combinedScale);
    const float range = spreadFunc(combinedScale) * state.textAntialias;
    ctx.fontAlphaMin = qMax(0.0f, base - range);
    ctx.fontAlphaMax = qMin(base + range, 1.0f);
}
// *** From QSGDistanceFieldGlyphNode - End ***
#endif

namespace QCanvasPainterDebugCounterUtils {

void fillDebugCounters(QVariantMap *dst, const QCDebugCounters &src)
{
    // The keys and the related environment variables are documented. See e.g.
    // the docs for QCanvasPainterItem::debug. Therefore, keys should never be
    // removed. If some future logic change dictates that something does not
    // make sense anymore, set the value to zero, but keep the entry in the map.

    dst->insert(QStringLiteral(u"fillDrawCallCount"), src.fillDrawCallCount);
    dst->insert(QStringLiteral(u"strokeDrawCallCount"), src.strokeDrawCallCount);
    dst->insert(QStringLiteral(u"textDrawCallCount"), src.textDrawCallCount);
    dst->insert(QStringLiteral(u"fillTriangleCount"), src.fillTriangleCount);
    dst->insert(QStringLiteral(u"strokeTriangleCount"), src.strokeTriangleCount);
    dst->insert(QStringLiteral(u"textTriangleCount"), src.textTriangleCount);
    dst->insert(QStringLiteral(u"drawCallCount"), src.drawCallCount);
    dst->insert(QStringLiteral(u"triangleCount"), src.triangleCount);
    dst->insert(QStringLiteral(u"imageMemoryUsage"), src.imageMemoryUsage);
    dst->insert(QStringLiteral(u"imageCount"), src.imageCount);
    dst->insert(QStringLiteral(u"pathGroupCount"), src.pathGroupCount);
    dst->insert(QStringLiteral(u"cachedSubpathCount"), src.cachedSubpathCount);
    dst->insert(QStringLiteral(u"cachedPathVertexDataSize"), src.cachedPathVertexDataSize);
}

} // namespace QQCanvasPainterDebugCounterUtils

QT_END_NAMESPACE
