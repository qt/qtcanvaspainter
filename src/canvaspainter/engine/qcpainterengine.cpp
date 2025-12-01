// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2013 Mikko Mononen memon@inside.org
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcpainterengine_p.h"
#include "qcpainterrhirenderer_p.h"
#include "qccustombrush.h"
#include "qccustombrush_p.h"
#include "qcpainterpath_p.h"
#include "qctext.h"
#include "qctext_p.h"
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
#include "qctextlayout_p.h"
#include <QtGui/private/qdistancefield_p.h>
#endif
#include <qdebug.h>
#include <float.h>

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

static const float QCPAINTER_MAX_STROKE_WIDTH = 100.0f;
static const float QCPAINTER_MAX_ANTIALIAS_WIDTH = 10.0f;
static const int QCPAINTER_MAX_TESSELATE_LEVEL = 11;

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
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    auto &ct = ctx.cachedTexts;
    for (auto i = ct.cbegin(), end = ct.cend(); i != end; ++i)
        delete i.value().layout;
    ct.clear();
#endif
    ctx.commands.clear();
    ctx.commandsData.clear();
    ctx.paths.clear();
    ctx.points.clear();
    ctx.currentPath = nullptr;
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
}

void QCPainterEngine::reset()
{
    state.fill = QCPaint();
    state.stroke = QCPaint();
    state.strokeWidth = 1.0f;
    state.miterLimit = 10.0f;
    state.lineCap = QCPainter::LineCap::Butt;
    state.lineJoin = QCPainter::LineJoin::Miter;
    state.compositeOperation = QCPainter::CompositeOperation::SourceOver;
    state.alpha = 1.0f;
    state.brightness = 1.0f;
    state.contrast = 1.0f;
    state.saturate = 1.0f;
    state.transform.reset();
    state.clip.extent[0] = -1.0f;
    state.clip.extent[1] = -1.0f;
    state.customFill = nullptr;
    state.customStroke = nullptr;
    state.textWrapMode = QCPainter::WrapMode::NoWrap;
    state.textAlignment = QCPainter::TextAlign::Start;
    state.textBaseline = QCPainter::TextBaseline::Alphabetic;
    state.textDirection = QCPainter::TextDirection::Inherit;
    state.textLineHeight = 0.0f;
    state.textAntialias = 1.0f;
    state.font = QFont();
    ctx.fontId = 0;
    ctx.fontAlphaMin = -1.0f;
    ctx.fontAlphaMax = -1.0f;
    ctx.pathsCount = 0;
    ctx.pointsCount = 0;
    ctx.verticesCount = 0;
    ctx.commandsCount = 0;
    ctx.commandsDataCount = 0;
    ctx.currentPath = nullptr;
    ctx.currentPainterPath = nullptr;
    ctx.currentPathGroup = -1;
    ctx.preparedPainterPath = nullptr;
    ctx.renderHints = QCPainter::RenderHint::Antialiasing;
}


// ***** Painting styles *****

void QCPainterEngine::setStrokeColor(const QColor &color)
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
    state.customStroke = nullptr;
}

void QCPainterEngine::setStrokePaint(const QCPaint &paint)
{
    state.stroke = paint;
    state.customStroke = nullptr;
}

void QCPainterEngine::setFillColor(const QColor &color)
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
    state.customFill = nullptr;
}

void QCPainterEngine::setFillPaint(const QCPaint &paint)
{
    state.fill = paint;
    state.customFill = nullptr;
}

void QCPainterEngine::setLineWidth(float width)
{
    state.strokeWidth = width;
}

void QCPainterEngine::setLineCap(QCPainter::LineCap lineCap)
{
    state.lineCap = lineCap;
}

void QCPainterEngine::setLineJoin(QCPainter::LineJoin lineJoin)
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

void QCPainterEngine::setGlobalSaturate(float value)
{
    state.saturate = value;
}

// ***** Custom paints *****

void QCPainterEngine::setCustomStrokeBrush(QCCustomBrush *brush)
{
    state.customStroke = brush;
}

void QCPainterEngine::setCustomFillBrush(QCCustomBrush *brush)
{
    state.customFill = brush;
}

// ***** Transform *****

QTransform QCPainterEngine::currentTransform() const
{
    return state.transform;
}

void QCPainterEngine::transform(const QTransform &transform)
{
    state.transform *= transform;
}

void QCPainterEngine::setTransform(const QTransform &transform)
{
    state.transform = transform;
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

// ***** Images *****

int QCPainterEngine::createImage(int width, int height,
                                        QCPainter::ImageFlags flags,
                                        const uchar* data)
{
    return m_renderer->renderCreateTexture(TextureFormatRGBA, width, height, flags, data);
}

bool QCPainterEngine::deleteImage(int imageId)
{
    return m_renderer->renderDeleteTexture(imageId);
}


// ***** Brushes *****

QCPaint QCPainterEngine::createLinearGradient(float startX, float startY,
                                              float endX, float endY,
                                              const QColor &iColor, const QColor &oColor,
                                              int imageId)
{
    QCPaint p;
    p.brushType = BrushLinearGradient;

    float dx = endX - startX;
    float dy = endY - startY;
    float d = std::sqrt(dx*dx + dy*dy);
    const float small = 0.0001f;
    if (d > small) {
        dx /= d;
        dy /= d;
    } else {
        dx = 0;
        dy = 1;
    }
    p.transform.setMatrix(dy, -dx, 0,
                          dx, dy, 0,
                          startX, startY, 1);
    p.feather = qMax(1.0f, d);

    // Note: extent and radius not used.

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }

    return p;
}

QCPaint QCPainterEngine::createRadialGradient(float centerX, float centerY,
                                              float iRadius, float oRadius,
                                              const QColor &iColor, const QColor &oColor,
                                              int imageId)
{
    QCPaint p;
    p.brushType = BrushRadialGradient;
    const float r = (iRadius + oRadius) * 0.5f;
    const float f = (oRadius - iRadius);
    p.transform = p.transform.translate(centerX, centerY);

    // Note: extent not used.

    p.radius = r;
    p.feather = qMax(1.0f, f);

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }

    return p;
}

QCPaint QCPainterEngine::createConicalGradient(float centerX, float centerY, float angle,
                                                  const QColor &iColor, const QColor &oColor,
                                                  int imageId)
{
    QCPaint p;
    p.brushType = BrushConicalGradient;
    p.transform = p.transform.translate(centerX, centerY);

    // Note: p.extent not used

    // Angle is in radius variable, as radians
    // Rotating clockwise, starting from east
    p.radius = angle + M_PI_2;

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
    return p;
}

QCPaint QCPainterEngine::createBoxGradient(float x, float y,
                                           float width, float height,
                                           float radius, float feather,
                                           const QColor &iColor, const QColor &oColor,
                                           int imageId)
{
    QCPaint p;
    p.brushType = BrushBoxGradient;
    p.transform = p.transform.translate(x + (width * 0.5f), y + (height * 0.5f));

    p.extent[0] = width * 0.5f;
    p.extent[1] = height * 0.5f;

    p.radius = qMin(radius, qMin(width, height) * 0.5f);
    p.feather = qMax(0.0f, feather);

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }

    return p;
}

QCPaint QCPainterEngine::createImagePattern(float x, float y, float width, float height,
                                               int imageId, float angle, const QColor &tintColor)
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

QCPaint QCPainterEngine::createBoxShadow(float x, float y, float width, float height,
                                         const QVector4D &radius,
                                         float blur, const QColor &color)
{
    QCPaint p;
    p.brushType = BrushBoxShadow;

    p.transform = p.transform.translate(x + (width * 0.5f), y + (height * 0.5f));

    p.extent[0] = width * 0.5f;
    p.extent[1] = height * 0.5f;

    // Unused, individual corner radius in outerColor
    //p.radius = radius;

    p.feather = blur;

    p.innerColor = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
    p.outerColor = { radius.x(), radius.y(), radius.z(), radius.w() };
    p.imageId = 0;

    return p;
}

QCPaint QCPainterEngine::createGridPattern(float x, float y, float width, float height,
                                           float lineWidth, float angle, float feather,
                                           const QColor &gridColor, const QColor &backgroundColor)
{
    QCPaint p;
    p.brushType = BrushGrid;
    p.transform = p.transform.translate(x, y);
    if (!qFuzzyIsNull(angle))
        p.transform = p.transform.rotateRadians(angle);

    p.extent[0] = width;
    p.extent[1] = height;

    p.feather = feather;
    p.radius = lineWidth;

    p.innerColor = { gridColor.redF(), gridColor.greenF(), gridColor.blueF(), gridColor.alphaF() };
    p.outerColor = { backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF() };

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
    ctx.preparedPainterPath = nullptr;
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
    QCPainter::PathWinding direction;
    if (crossProduct(dx0, dy0, dx1, dy1) > 0.0f) {
        direction = QCPainter::PathWinding::ClockWise;
        cx = x1 + dx0 * d + dy0 * radius;
        cy = y1 + dy0 * d - dx0 * radius;
        a0 = std::atan2(dx0, -dy0);
        a1 = std::atan2(-dx1, dy1);
    } else {
        direction = QCPainter::PathWinding::CounterClockWise;
        cx = x1 + dx0 * d - dy0 * radius;
        cy = y1 + dy0 * d + dx0 * radius;
        a0 = std::atan2(-dx0, dy0);
        a1 = std::atan2(dx1, -dy1);
    }

    addArc(cx, cy, radius, a0, a1, direction, true);
}

void QCPainterEngine::addArc(float x, float y, float radius,
                                float a0, float a1, QCPainter::PathWinding direction,
                                bool isConnected)
{
    // Clamp angles
    float da = a1 - a0;
    static constexpr float TWOPI = float(M_PI) * 2;
    if (direction == QCPainter::PathWinding::ClockWise) {
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

    if (direction == QCPainter::PathWinding::CounterClockWise)
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
        x, y + height,
        x + width, y + height,
        x + width, y
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
        const float halfSize = std::min(std::abs(width), std::abs(height)) * 0.5f;
        const float maxRad = std::min(radius, halfSize);
        const float rX = maxRad * sign(width);
        const float rY = maxRad * sign(height);
        const float xW = x + width;
        const float yH = y + height;
        const float rYCK = rY * COMP_KAPPA90;
        const float rXCK = rX * COMP_KAPPA90;
        float data[] = {
            x, y + rY,
            x, yH - rY,
            x, yH - rYCK, x + rXCK, yH, x + rX, yH,
            xW - rX, yH,
            xW - rXCK, yH, xW, yH - rYCK, xW, yH - rY,
            xW, y + rY,
            xW, y + rYCK, xW - rXCK, y, xW - rX, y,
            x + rX, y,
            x + rXCK, y, x, y + rYCK, x, y + rY
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
            x, y + rYTL,
            x, yH - rYBL,
            x, yH - rYBL * COMP_KAPPA90, x + rXBL * COMP_KAPPA90, yH, x + rXBL, yH,
            xW - rXBR, yH,
            xW - rXBR * COMP_KAPPA90, yH, xW, yH - rYBR * COMP_KAPPA90, xW, yH - rYBR,
            xW, y + rYTR,
            xW, y + rYTR * COMP_KAPPA90, xW - rXTR * COMP_KAPPA90, y, xW - rXTR, y,
            x + rXTL, y,
            x + rXTL * COMP_KAPPA90, y, x, y + rYTL * COMP_KAPPA90, x, y + rYTL
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
        xmRadX, y,
        xmRadX, y + radYK, x - radXK, ypRadY, x, ypRadY,
        x + radXK, ypRadY, xpRadX, y + radYK, xpRadX, y,
        xpRadX, y - radYK, x + radXK, ymRadY, x, ymRadY,
        x - radXK, ymRadY, xmRadX, y - radYK, xmRadX, y,
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
        x - radius, y,
        x - radius, y + radK, x - radK, ypRad, x, ypRad,
        x + radK, ypRad, x + radius, y + radK, x + radius, y,
        x + radius, y - radK, x + radK, ymRad, x, ymRad,
        x - radK, ymRad, x - radius, y - radK, x - radius, y,
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

void QCPainterEngine::addPath(const QCPainterPath &path, const QTransform &transform)
{
    appendPainterPath(path, transform);
}

void QCPainterEngine::addPath(const QCPainterPath &path, qsizetype start, qsizetype count, const QTransform &transform)
{
    appendPainterPath(path, start, count, transform);
}

void QCPainterEngine::setPathWinding(QCPainter::PathWinding winding)
{
    QCCommand c = winding == QCPainter::PathWinding::ClockWise ?
                      QCCommand::WindingCW : QCCommand::WindingCCW;
    appendCommand(c);
}

void QCPainterEngine::fill()
{
#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::FILL);
#endif
    commandsToPaths();
    expandFill();

    const QCPaint fillPaint = getFillPaint();
    m_renderer->renderFill(fillPaint, state, ctx.edgeAAWidth,
                           ctx.bounds, ctx.paths, ctx.pathsCount,
                           ctx.currentPainterPath, ctx.currentPathGroup,
                           ctx.currentPathTransform);
#ifdef QCPAINTER_PERF_DEBUG
    perf.logEnd(QCPerfLogging::FILL);
#endif
}

void QCPainterEngine::stroke()
{
#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::STROKE);
#endif

    float strokeWidth;
    const QCPaint strokePaint = getStrokePaint(&strokeWidth);

    commandsToPaths();
    expandStroke(strokeWidth * 0.5f, state.lineCap, state.lineJoin, state.miterLimit);
    m_renderer->renderStroke(strokePaint, state, ctx.edgeAAWidth,
                             strokeWidth, ctx.paths, ctx.pathsCount,
                             ctx.currentPainterPath, ctx.currentPathGroup,
                             ctx.currentPathTransform);
#ifdef QCPAINTER_PERF_DEBUG
    perf.logEnd(QCPerfLogging::STROKE);
#endif
}

void QCPainterEngine::fill(const QCPainterPath &path, int pathGroup, const QTransform &transform)
{
    if (path.isEmpty())
        return;

    QCPainterPath *p = const_cast<QCPainterPath *>(&path);
    const bool cacheGeometry = (pathGroup != -1);
    const bool pathUpdateRequired = fillPathUpdateRequired(p, pathGroup);
    if (!cacheGeometry) {
        // Not caching, so prepare and fill normally.
        if (pathUpdateRequired || ctx.preparedPainterPath != &path || ctx.preparedTransform != transform)
            preparePainterPath(path, transform);
        fill();
    } else if (pathUpdateRequired) {
        // Caching - Update required.
        if (ctx.preparedPainterPath != &path || ctx.preparedTransform != transform)
            preparePainterPath(path);
        // Fill with currently rendering painterpath
        ctx.currentPainterPath = p;
        ctx.currentPathGroup = pathGroup;
        ctx.currentPathTransform = transform;
        fill();
        ctx.currentPainterPath = nullptr;
        ctx.currentPathGroup = -1;
        ctx.currentPathTransform.reset();
    } else {
        // Caching - No need to update.
        const QCPaint fillPaint = getFillPaint();

        // Uses pathsCount 0, meaning that previous path data can be reused.
        m_renderer->renderFill(fillPaint, state, ctx.edgeAAWidth,
                               ctx.bounds, ctx.paths, 0,
                               p, pathGroup, transform);
    }
}

void QCPainterEngine::stroke(const QCPainterPath &path, int pathGroup, const QTransform &transform)
{
    if (path.isEmpty())
        return;

    QCPainterPath *p = const_cast<QCPainterPath *>(&path);
    const bool cacheGeometry = (pathGroup != -1);
    const bool pathUpdateRequired = strokePathUpdateRequired(p, pathGroup);
    if (!cacheGeometry) {
        // Not caching, so prepare and stroke normally.
        // TODO: Updated testing shows that this mode wouldn't have pros compared to cached...
        // Slower both animated and non-animated, and doesn't reduce mem usage as dynamic
        // buffers need similar amount to static buffers(?)
        // So consider if having this -1 group makes sense other than for testing?
        if (pathUpdateRequired || ctx.preparedPainterPath != &path || ctx.preparedTransform != transform)
            preparePainterPath(path, transform);
        stroke();
    } else if (pathUpdateRequired) {
        // Caching - Update required.
        if (ctx.preparedPainterPath != &path || ctx.preparedTransform != transform)
            preparePainterPath(path);
        // Stroke with currently rendering painterpath
        ctx.currentPainterPath = p;
        ctx.currentPathGroup = pathGroup;
        ctx.currentPathTransform = transform;
        stroke();
        ctx.currentPainterPath = nullptr;
        ctx.currentPathGroup = -1;
        ctx.currentPathTransform.reset();
    } else {
        // Caching - No need to update.
        float strokeWidth;
        const QCPaint strokePaint = getStrokePaint(&strokeWidth);
        // Uses pathsCount 0, meaning that previous path data can be reused.
        m_renderer->renderStroke(strokePaint, state, ctx.edgeAAWidth,
                                 strokeWidth, ctx.paths, 0,
                                 p, pathGroup, transform);
    }
}

// ***** Blending *****

void QCPainterEngine::setGlobalCompositeOperation(QCPainter::CompositeOperation op)
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
    m_renderer->setFlag(QCPainterRhiRenderer::SimpleClipping, false);
    m_renderer->setFlag(QCPainterRhiRenderer::TransformedClipping, false);
}

void QCPainterEngine::setClipRect(const QRectF &rect)
{
    if (rect.isEmpty()) {
        resetClipRect();
    } else {
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
            m_renderer->setFlag(QCPainterRhiRenderer::SimpleClipping, true);
            m_renderer->setFlag(QCPainterRhiRenderer::TransformedClipping, false);
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
            m_renderer->setFlag(QCPainterRhiRenderer::SimpleClipping, false);
            m_renderer->setFlag(QCPainterRhiRenderer::TransformedClipping, true);
        }
    }
}


// ***** Text *****

void QCPainterEngine::setTextWrapMode(QCPainter::WrapMode wrapMode)
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

void QCPainterEngine::setTextAlignment(QCPainter::TextAlign align)
{
    state.textAlignment = align;
}

void QCPainterEngine::setTextBaseline(QCPainter::TextBaseline baseline)
{
    state.textBaseline = baseline;
}

void QCPainterEngine::setTextDirection(QCPainter::TextDirection direction)
{
    state.textDirection = direction;
}

void QCPainterEngine::prepareText(QCText &text)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    auto &font = state.font;
    QCTextCache &ct = ctx.cachedTexts[text.getId()];
    // Font size/scaling can also be moved (and eventually will) into the non-rendering part
    // but involves a bit more work to scale with the baselines, offsets, etc
    QCTextPrivate *textp = QCTextPrivate::get(&text);
    if (!textp->isPrepared && (textp->isLayoutDirty || text.fontSize() != font.pixelSize())) {
        text.setFontSize(font.pixelSize());
        int width, height;
        //TODO: This should be a pointer if some other text updates the atlas
        ct.atlasId = m_renderer->populateFont(font, text, &width, &height);

        auto currentSize = ct.transformedVerts.size();
        ct.transformedVerts.resize(ct.verts.size());
        ct.sizeChange = int(ct.transformedVerts.size() - currentSize);

        textp->isPrepared = true;
    }

#endif
}

void QCPainterEngine::fillText(const QString &text, const QRectF &rect, int cacheIndex)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    if (cacheIndex > -1) {
        auto &ct = ctx.cachedCTexts[cacheIndex];
        ct.setText(text);
        ct.setRect(rect);
        fillText(ct);
    } else {
        std::vector<QCRhiDistanceFieldGlyphCache::TexturedPoint2D> verts{};
        std::vector<uint32_t> indices{};
        int width, height;
        auto tex = m_renderer->populateFont(state.font, rect, text, verts, indices, &width, &height);

        const QCPaint p = getFillPaint();
        ctx.fontId = tex;
        updateStateFontVars();

        if (!state.customFill) {
            m_renderer->renderTextFill(p, state, verts, indices);
        } else {
            m_renderer->renderTextFillCustom(
                p, state, state.customFill, verts, indices);
        }
    }
#endif
}

void QCPainterEngine::fillText(QCText &text)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    QCTextCache &ct = ctx.cachedTexts[text.getId()];
    QCTextPrivate *textp = QCTextPrivate::get(&text);
    if (state.transform != ct.previousTransform)
        textp->isDirty = true;

    prepareText(text);

    const QCPaint p = getFillPaint();
    ctx.fontId = ct.atlasId;
    updateStateFontVars();

    // Add non-rendering transformation here
    for (size_t i = 0; i < ct.verts.size(); ++i) {
        auto v = ct.verts[i];
        auto tp = state.transform.map(QPointF(v.x + text.x(), v.y + text.y()));
        v.x = tp.x();
        v.y = tp.y();
        ct.transformedVerts[i] = v;
    }

    if (!state.customFill) {
        m_renderer->renderTextFill(
            p,
            state,
            ct.transformedVerts,
            ct.indices,
            text,
            textp->isDirty | textp->isLayoutDirty);
    } else {
        m_renderer->renderTextFillCustom(
            p,
            state,
            state.customFill,
            ct.transformedVerts,
            ct.indices,
            text,
            textp->isDirty | textp->isLayoutDirty);
    }

    textp->isLayoutDirty = false;
    textp->isDirty = false;

    ct.previousTransform = state.transform;
#endif
}


QRectF QCPainterEngine::textBoundingBox(const QString &text, const QRectF &rect)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    auto &font = state.font;
    // TODO: Do we need here to set correct fontWeight and fontItalic to qfont?
    // Do it also in shared method.
    auto metrics = QFontMetrics{font};

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
    layout.setText(text);
    layout.setFont(font);
    int leading = metrics.leading();
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

    QRectF textRect = layout.boundingRect();
    // Adjust rect height
    float textOffsetY = QCTextLayout::calculateVerticalAlignment(state.textBaseline, rect, metrics, layout.boundingRect());
    textRect.adjust(0, textOffsetY, 0, textOffsetY);
    // Adjust rect width
    float textOffsetX = (textRect.width() - layoutWidth);
    auto textAlign = effectiveTextAlign(text);
    if (textAlign == QCPainter::TextAlign::Center)
        textRect.adjust(0.5f * textOffsetX, 0, -0.5f * textOffsetX, 0);
    else if (textAlign == QCPainter::TextAlign::Left)
        textRect.adjust(0, 0, -textOffsetX, 0);
    else
        textRect.adjust(textOffsetX, 0, 0, 0);
    return textRect;
#else
    return QRectF();
#endif
}

QRectF QCPainterEngine::textBoundingBox(QCText &text)
{
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    prepareText(text);
    return ctx.cachedTexts[text.getId()].layout->bounds();
#else
    return QRectF();
#endif
}

// ***** Other *****

QCDrawDebug QCPainterEngine::drawDebug() const
{
    return ctx.drawDebug;
}

void QCPainterEngine::setAntialias(float antialias)
{
    antialias = std::clamp(antialias, 0.0f, QCPAINTER_MAX_ANTIALIAS_WIDTH);
    ctx.edgeAAWidth = antialias / ctx.devicePxRatio;
}

void QCPainterEngine::setMiterLimit(float limit)
{
    state.miterLimit = limit;
}

void QCPainterEngine::removePathGroup(int pathGroup)
{
    // Remove from engine side
    erase_if(ctx.cachedStrokePaths, [pathGroup](const QHash<const QCPainterPath*, QCCachedPath>::iterator it) {
        return it->pathGroup == pathGroup;
    });
    erase_if(ctx.cachedFillPaths, [pathGroup](const QHash<const QCPainterPath*, QCCachedPath>::iterator it) {
        return it->pathGroup == pathGroup;
    });
    // Remove from renderer side
    m_renderer->removePathGroup(pathGroup);
}

void QCPainterEngine::setRenderHints(QCPainter::RenderHints hints, bool on)
{
    if (on)
        ctx.renderHints |= hints;
    else
        ctx.renderHints &= ~hints;

    // Set the changed hints into renderer.
    if (hints & QCPainter::RenderHint::Antialiasing)
        m_renderer->setFlag(QCPainterRhiRenderer::Antialiasing, on);
    if (hints & QCPainter::RenderHint::HighQualityStroking)
        m_renderer->setFlag(QCPainterRhiRenderer::StencilStrokes, on);
}

QCPainter::RenderHints QCPainterEngine::renderHints() const
{
    return ctx.renderHints;
}

// ********** private **********

void QCPainterEngine::setDevicePixelRatio(float ratio)
{
    if (ratio > 0.0f) {
        ctx.tessTol = 0.25f / ratio;
        ctx.distTol = 0.01f / ratio;
        // Note: This is not called during the paint operations,
        // so it can set edgeAAWidth to default value.
        ctx.edgeAAWidth = 1.0f / ratio;
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
void QCPainterEngine::appendCommandsData(const float commandsData[], int dCount)
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

    if (state.transform.isIdentity()) {
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

void QCPainterEngine::handleSetPathWinding(QCPainter::PathWinding winding)
{
    if (auto *path = ctx.currentPath)
        path->winding = winding;
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

void QCPainterEngine::enforceWinding(int pointsOffset, int pointsCount, QCPainter::PathWinding winding)
{
    if (pointsCount <= 2)
        return;
    // Calculate if points area is positive or negative
    float area = 0;
    for (int i = 2; i < pointsCount; i++) {
        const QCPoint a = ctx.points.at(pointsOffset);
        const QCPoint b = ctx.points.at(pointsOffset + i - 1);
        const QCPoint c = ctx.points.at(pointsOffset + i);
        area += triarea2(a.x, a.y, b.x, b.y, c.x, c.y);
    }
    // If needed, reverse the points order
    if ((winding == QCPainter::PathWinding::CounterClockWise && area < 0.0f) ||
        (winding == QCPainter::PathWinding::ClockWise && area > 0.0f)) {
        int i = 0;
        int j = pointsCount - 1;
        auto &pts = ctx.points;
        while (i < j) {
            std::swap(pts[pointsOffset + i], pts[pointsOffset + j]);
            i++;
            j--;
        }
    }
}

// Create path elements from commands
void QCPainterEngine::commandsToPaths()
{
    // Paths already created for these commands
    if (ctx.pathsCount > 0)
        return;

#ifdef QCPAINTER_PERF_DEBUG
    perf.logStart(QCPerfLogging::C2P);
#endif

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
                tesselateBezier(last.x, last.y, cp1x, cp1y, cp2x, cp2y, px, py, 0, PointCorner);
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
            auto w = cmd == QCCommand::WindingCW ? QCPainter::PathWinding::ClockWise :
                         QCPainter::PathWinding::CounterClockWise;
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
    // Calculate the direction and length of line segments.
    for (int j = 0; j < pCount; j++) {
        QCPath &path = ctx.paths[j];
        // If the first and last points are the same, remove the last, mark as closed path.
        // In expandStroke() the closed paths will then add vertices to beginning.
        QCPoint ptFirst = ctx.points.at(path.pointsOffset);
        QCPoint ptLast = ctx.points.at(path.pointsOffset + path.pointsCount - 1);
        if (pointsEquals(ptLast.x, ptLast.y, ptFirst.x, ptFirst.y, ctx.distTol)) {
            path.pointsCount--;
            path.isClosed = true;
        }

        enforceWinding(path.pointsOffset, path.pointsCount, path.winding);

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

// Customized version of NanoVG bezier tesselation (https://github.com/memononen/nanovg)
void QCPainterEngine::tesselateBezier(float x1, float y1, float x2, float y2,
                                         float x3, float y3, float x4, float y4,
                                         int level, QCPointFlags flags)
{
    if (Q_UNLIKELY(level > QCPAINTER_MAX_TESSELATE_LEVEL)) {
        // This shouldn't usually happen, as we will
        // reach the tessTol accuracy. In case of e.g. huge circles, increase
        // the QCPAINTER_MAX_TESSELATE_LEVEL.
        return;
    }

    const float dx = x4 - x1;
    const float dy = y4 - y1;
    const float d2 = std::abs((x2 - x4) * dy - (y2 - y4) * dx);
    const float d3 = std::abs((x3 - x4) * dy - (y3 - y4) * dx);
    const float d23 = d2 + d3;
    if (d23 * d23 < ctx.tessTol * (dx * dx + dy * dy)) {
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

void QCPainterEngine::expandFill()
{
    const float aa = ctx.antialiasingEnabled ? ctx.edgeAAWidth : 0.0f;
    // Hardcoded miterLimit for fill.
    const float miterLimit = 2.4f;
    calculateJoins(aa, QCPainter::LineJoin::Miter, miterLimit);
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
                if (p1.flags & PointBevel) {
                    if (p1.flags & PointLeft) {
                        float lx = p1.x + p1.dmx * halfAA;
                        float ly = p1.y + p1.dmy * halfAA;
                        addVert(lx, ly, 0.5f, 1.0f);
                    } else {
                        float lx0 = p1.x + p0.dy * halfAA;
                        float ly0 = p1.y - p0.dx * halfAA;
                        addVert(lx0, ly0, 0.5f, 1.0f);
                        float lx1 = p1.x + p1.dy * halfAA;
                        float ly1 = p1.y - p1.dx * halfAA;
                        addVert(lx1, ly1, 0.5f, 1.0f);
                    }
                } else {
                    float lx = p1.x + p1.dmx * halfAA;
                    float ly = p1.y + p1.dmy * halfAA;
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
            // Calculate antialiasing
            const float rw = halfAA;
            const float ru = 1.0f;
            float lw = aa + halfAA;
            float lu = 0;

            // Create only half of aa for convex shapes so that
            // the shape can be rendered without stenciling.
            if (isConvex) {
                lw = halfAA; // This should generate the same vertex as fill inset above.
                lu = 0.5f; // Set outline fade at middle.
            }

            const int loopIndex = ctx.verticesCount;
            path.strokeOffset = ctx.verticesCount;
            // Looping
            int p0Index = path.pointsOffset + path.pointsCount - 1;
            int p1Index = path.pointsOffset;

            for (int j = 0; j < path.pointsCount; ++j) {
                const QCPoint &p0 = ctx.points[p0Index];
                const QCPoint &p1 = ctx.points[p1Index];
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
                addVert(verts.at(loopIndex).x, verts.at(loopIndex).y, lu, 1.0f);
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
void QCPainterEngine::expandStroke(float w, QCPainter::LineCap cap, QCPainter::LineJoin join, float miterLimit)
{
    // w is half of stroke width + aa
    const float aa = ctx.antialiasingEnabled ? ctx.edgeAAWidth : 0.0f;
    w += aa * 0.5f;
    calculateJoins(w, join, miterLimit);
    const int pCount = ctx.pathsCount;
    const int roundDivs = (join == QCPainter::LineJoin::Round) || (cap == QCPainter::LineCap::Round)
                              ? curveDivs(w, ctx.tessTol)
                              : 0;

    // Calculate max vertex usage
    int vertsCount = 0;
    for (int i = 0; i < pCount; i++) {
        const QCPath &path = ctx.paths.at(i);
        if (join == QCPainter::LineJoin::Round)
            vertsCount += (path.pointsCount + path.bevelCount * (roundDivs + 2) + 1) * 2; // plus one for loop
        else
            vertsCount += (path.pointsCount + path.bevelCount * 5 + 1) * 2; // plus one for loop
        if (!path.isClosed) {
            // Not looping, so need vertices for caps
            if (cap == QCPainter::LineCap::Round) {
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
        QCPath &path = ctx.paths[i];
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
            const QCPoint &p0 = ctx.points[p0Index];
            const QCPoint &p1 = ctx.points[p1Index];

            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            normalizePoint(&dx, &dy);
            if (cap == QCPainter::LineCap::Butt)
                addButtCapStart(p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (cap == QCPainter::LineCap::Square)
                addButtCapStart(p0, dx, dy, w, w - aa, aa, u0, u1);
            else if (cap == QCPainter::LineCap::Round)
                addRoundCapStart(p0, dx, dy, w, roundDivs, u0, u1);
        }

        for (int j = startPoint; j < endPoint; ++j) {
            // Add joins
            const QCPoint &p0 = ctx.points[p0Index];
            const QCPoint &p1 = ctx.points[p1Index];
            if (p1.flags & (PointBevel | PointInnerBevel)) {
                if (join == QCPainter::LineJoin::Round) {
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
            const QCPoint &p0 = ctx.points[p0Index];
            const QCPoint &p1 = ctx.points[p1Index];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            normalizePoint(&dx, &dy);
            if (cap == QCPainter::LineCap::Butt)
                addButtCapEnd(p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (cap == QCPainter::LineCap::Square)
                addButtCapEnd(p1, dx, dy, w, w - aa, aa, u0, u1);
            else if (cap == QCPainter::LineCap::Round)
                addRoundCapEnd(p1, dx, dy, w, roundDivs, u0, u1);
        }
        path.strokeCount = ctx.verticesCount - path.strokeOffset;
    }
}

void QCPainterEngine::calculateJoins(float w, QCPainter::LineJoin join, float miterLimit)
{
    // Inverse of width
    const float iw = (w > 0.0f) ? 1.0f / w : 0.0f;

    const int pCount = ctx.pathsCount;
    // Calculate which joins needs extra vertices to append, and gather vertex count.
    for (int i = 0; i < pCount; i++) {
        QCPath &path = ctx.paths[i];
        const int pointsCount = path.pointsCount;
        int p0Index = path.pointsOffset + pointsCount - 1;
        int p1Index = path.pointsOffset;
        int leftCount = 0;
        int bevelCount = 0;

        for (int j = 0; j < pointsCount; j++) {
            const QCPoint &p0 = ctx.points[p0Index];
            QCPoint &p1 = ctx.points[p1Index];
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

            // Calculate if we should use bevel or miter for inner join.
            float limit = std::max(1.01f, std::min(p0.len, p1.len) * iw);
            if ((dmr2 * limit * limit) < 1.0f)
                p1.flags |= PointInnerBevel;

            // Check if the corner needs to be beveled.
            if (p1.flags & PointCorner) {
                if (join == QCPainter::LineJoin::Bevel || join == QCPainter::LineJoin::Round || (dmr2 * miterLimit * miterLimit) < 1.0f ) {
                    p1.flags |= PointBevel;
                }
            }

            if ((p1.flags & (PointBevel | PointInnerBevel)))
                bevelCount++;

            p0Index = path.pointsOffset + j;
            p1Index = p0Index + 1;
        }

        path.isConvex = (leftCount == pointsCount);
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
                addVert(p1.x, p1.y, 0.5f, 1.0f);
                addVert(rx, ry, ru, 1.0f);
            }
#endif
            addVert(lx1, ly1, lu, 1.0f);
            addVert(p1.x - (dlx1 * rw), p1.y - (dly1 * rw), ru, 1.0f);
        } else {
            float rx0 = p1.x - (p1.dmx * rw);
            float ry0 = p1.y - (p1.dmy * rw);
            addVert(p1.x, p1.y, 0.5f, 1.0f);
            addVert(p1.x - (dlx0 * rw), p1.y - (dly0 * rw), ru, 1.0f);
            addVert(rx0, ry0, ru, 1.0f);
            addVert(rx0, ry0, ru, 1.0f);
            addVert(p1.x, p1.y, 0.5f, 1.0f);
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
                addVert(p1.x, p1.y, 0.5f, 1.0f);
            }
#endif
            addVert(p1.x + (dlx1 * lw), p1.y + (dly1 * lw), lu, 1.0f);
            addVert(rx1, ry1, ru, 1.0f);
        } else {
            float lx0 = p1.x + (p1.dmx * lw);
            float ly0 = p1.y + (p1.dmy * lw);
            addVert(p1.x + (dlx0 * lw), p1.y + (dly0 * lw), lu, 1.0f);
            addVert(p1.x, p1.y, 0.5f, 1.0f);

            addVert(lx0, ly0, lu, 1.0f);
            addVert(lx0, ly0, lu, 1.0f);

            addVert(p1.x + (dlx1 * lw), p1.y + (dly1 * lw), lu, 1.0f);
            addVert(p1.x, p1.y, 0.5f, 1.0f);
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
void QCPainterEngine::preparePainterPath(const QCPainterPath &path,
                                         const QTransform &transform)
{
    beginPath();
    appendPainterPath(path, transform);
    // Store currently prepared path so subsequential fill & stroke calls
    // with same (unchanged) path & transform require preparing commands only once.
    ctx.preparedPainterPath = &path;
    ctx.preparedTransform = transform;
}

// Append \a path into current commands.
void QCPainterEngine::appendPainterPath(const QCPainterPath &path,
                                         const QTransform &transform)
{
    const QCPainterPathPrivate *pathd = QCPainterPathPrivate::get(&path);
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
void QCPainterEngine::appendPainterPath(const QCPainterPath &path,
                                         qsizetype start,
                                         qsizetype count,
                                         const QTransform &transform)
{
    const QCPainterPathPrivate *pathd = QCPainterPathPrivate::get(&path);

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
            auto dataSize = QCPainterPathPrivate::dataSizeOf(pathd->commands.at(i));
            if (i < start) {
                commandsDataStart += dataSize;
            } else {
                commandsDataCount += dataSize;
            }
        }
    }
    // Note: commandsDataStart and commandsDataCount don't need to be
    // validated as they are always in range when we don't allow raw
    // non-const access into QCPainterPath data.
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
bool QCPainterEngine::fillPathUpdateRequired(QCPainterPath *path, int pathGroup)
{
    QCPainterPathPrivate *pathd = QCPainterPathPrivate::get(path);
    QCCachedPath &cp = ctx.cachedFillPaths[path];
    bool updateRequired = false;
    if (pathGroup != cp.pathGroup ||
        pathd->pathIterations != cp.pathIterations ||
        pathd->commandsCount != cp.commandsCount ||
        !qFuzzyCompare(ctx.edgeAAWidth, cp.edgeAAWidth) ||
        state.transform != cp.stateTransform ||
        !m_renderer->isPathCached(path, pathGroup)) {
        updateRequired = true;
        // Reset cache states
        cp.pathGroup = pathGroup;
        cp.pathIterations = pathd->pathIterations;
        cp.commandsCount = pathd->commandsCount;
        cp.edgeAAWidth = ctx.edgeAAWidth;
        cp.stateTransform = state.transform;
    }
    return updateRequired;
}

// Returns true if path has changed or some state property related
// to stroke vertices generation has changed compared to cached path.
bool QCPainterEngine::strokePathUpdateRequired(QCPainterPath *path, int pathGroup)
{
    QCPainterPathPrivate *pathd = QCPainterPathPrivate::get(path);
    QCCachedPath &cp = ctx.cachedStrokePaths[path];
    bool updateRequired = false;
    if (pathGroup != cp.pathGroup ||
        pathd->pathIterations != cp.pathIterations ||
        pathd->commandsCount != cp.commandsCount ||
        !qFuzzyCompare(ctx.edgeAAWidth, cp.edgeAAWidth) ||
        !qFuzzyCompare(state.strokeWidth, cp.strokeWidth) ||
        state.lineCap != cp.lineCap ||
        state.lineJoin != cp.lineJoin ||
        state.transform != cp.stateTransform ||
        !m_renderer->isPathCached(path, pathGroup)) {
        updateRequired = true;
        // Reset cache states
        cp.pathGroup = pathGroup;
        cp.pathIterations = pathd->pathIterations;
        cp.commandsCount = pathd->commandsCount;
        cp.edgeAAWidth = ctx.edgeAAWidth;
        cp.strokeWidth = state.strokeWidth;
        cp.lineCap = state.lineCap;
        cp.lineJoin = state.lineJoin;
        cp.stateTransform = state.transform;
    }
    return updateRequired;
}

// Return current fillPaint taking into account the state.
QCPaint QCPainterEngine::getFillPaint()
{
    QCPaint fillPaint = state.fill;
    fillPaint.alpha = state.alpha;
    // Apply current transform
    if (fillPaint.brushType != BrushColor && !state.transform.isIdentity())
        fillPaint.transform *= state.transform;
    return fillPaint;
}

//  Return current strokePaint taking into account the state and set \a strokeWidth.
QCPaint QCPainterEngine::getStrokePaint(float *strokeWidth)
{
    float scale = getAverageScale(state.transform);
    *strokeWidth = std::clamp(state.strokeWidth * scale, 0.0f, QCPAINTER_MAX_STROKE_WIDTH);
    float expa = 1.0f;
    if (*strokeWidth < ctx.edgeAAWidth) {
        // If the stroke width is less than pixel size, use alpha to emulate coverage.
        float alpha = std::clamp(*strokeWidth / ctx.edgeAAWidth, 0.0f, 1.0f);
        // Since coverage is area, scale by alpha*alpha.
        expa = alpha * alpha;
        *strokeWidth = ctx.edgeAAWidth;
    }

    QCPaint strokePaint = state.stroke;
    strokePaint.alpha = state.alpha * expa;
    // Apply current transform
    if (strokePaint.brushType != BrushColor && !state.transform.isIdentity())
        strokePaint.transform *= state.transform;
    return strokePaint;
}

// Return the effective text align, depending on text direction.
// Returned value is either Left, Right or Center, never Start or End.
QCPainter::TextAlign QCPainterEngine::effectiveTextAlign(QStringView text) const
{
    QCPainter::TextAlign align = state.textAlignment;
    auto effectiveDirection = state.textDirection;
#if QT_CONFIG(im)
    bool emptyAuto = effectiveDirection == QCPainter::TextDirection::Auto && text.isEmpty();
    if (emptyAuto || effectiveDirection == QCPainter::TextDirection::Inherit) {
        // Get inherited direction from QGuiApplication. Also used for Auto,
        // when the string is empty. See:
        // https://doc.qt.io/qt-6/qtquick-positioning-righttoleft.html#text-alignment
        if (qGuiApp->isRightToLeft())
            effectiveDirection = QCPainter::TextDirection::RightToLeft;
        else
            effectiveDirection = QCPainter::TextDirection::LeftToRight;
    }
#else
    if (effectiveDirection == QCPainter::TextDirection::Inherit) {
        // No access to inputMethod(), so fail to auto detect
        effectiveDirection = QCPainter::TextDirection::Auto;
    }
#endif
    if (effectiveDirection == QCPainter::TextDirection::Auto) {
        // Get automatic direction from QString isRightToLeft().
        if (text.isRightToLeft())
            effectiveDirection = QCPainter::TextDirection::RightToLeft;
        else
            effectiveDirection = QCPainter::TextDirection::LeftToRight;
    }

    if (effectiveDirection == QCPainter::TextDirection::RightToLeft) {
        if (align == QCPainter::TextAlign::Start) {
            align = QCPainter::TextAlign::Right;
        } else if (align == QCPainter::TextAlign::End) {
            align = QCPainter::TextAlign::Left;
        }
    } else if (align == QCPainter::TextAlign::Start) {
        align = QCPainter::TextAlign::Left;
    } else if (align == QCPainter::TextAlign::End) {
        align = QCPainter::TextAlign::Right;
    }

    return align;
}

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
// *** From QSGDistanceFieldGlyphNode - Start ***
static float qt_qc_envFloat(const char *name, float defaultValue)
{
    if (Q_LIKELY(!qEnvironmentVariableIsSet(name)))
        return defaultValue;
    bool ok = false;
    const float value = qgetenv(name).toFloat(&ok);
    return ok ? value : defaultValue;
}

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

QT_END_NAMESPACE
