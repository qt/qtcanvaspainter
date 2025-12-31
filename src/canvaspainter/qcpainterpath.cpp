// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcpainterpath.h"
#include "qcpainterpath_p.h"
#include "engine/qcpainterengineutils_p.h"
#include <QTransform>

QT_BEGIN_NAMESPACE

/* QCPainterPath is native path format of QCPainter.
 *
 * It has some similarities to QPainterPath, but prioritizes performance and simplicity over features.
 * Some differences:
 * - Elements format is private so it can be more optimal.
 * - No setElementPositionAt() method. No ability to adjust existing path elements makes
 *   it simpler to detect when the path has changed (commandsCount or pathIterations has changed).
 * - No length(), percentAtLength() or pointAtPercent() methods. The length of the path is not calculated.
 * - No contains(), intersects() or subtracted() methods. The path area is not calculated.
 * - No translate() or translated() methods. Instead of translating all path elements one-by-one,
 *   they can be translated (and rotatated & scaled) when renderered.
 * - No addText() method. Text is not supported natively as a path element.
 * - The fillrule is always WindingFill (nonzero), OddEvenFill is not supported.
 *
 * So in terms of functionality, QCPainterPath is closer to HTML Canvas API Path2D object:
 * https://developer.mozilla.org/en-US/docs/Web/API/Path2D
 */

// TODO: Measure optimal values for these
// These are the minimum sizes used if not calling reserve().
// Arrays will then grow automatically as path commands are added.
#ifndef QCPAINTER_MINIMUM_PATH_COMMANDS_SIZE
#define QCPAINTER_MINIMUM_PATH_COMMANDS_SIZE 64
#endif
#ifndef QCPAINTER_MINIMUM_PATH_COMMANDS_DATA_SIZE
#define QCPAINTER_MINIMUM_PATH_COMMANDS_DATA_SIZE 128
#endif

/*!
    \class QCPainterPath
    \brief QCPainterPath is the native path format of QCPainter.
    \inmodule QtCanvasPainter

    A painter path is an object composed of a number of graphical building blocks,
    such as rectangles, ellipses, lines, and curves. QCPainterPath API matches to
    QCPainter path painting, making it easy to adjust code between paintind directly
    or painting into a path. The main reason use QCPainterPath is to avoid recreating
    the (static) paths and be able to cache the paths GPU buffers.

    Compared to QPainterPath, QCPainterPath is more optimized for rendering with fewer
    features for comparing or adjusting the paths. In particular:
    \list
    \li There are no methods for intersection or subtraction between two paths.
    \li There is no method for translating the path.
    \li There is no method for adding text.
    \li The fill rule is always \c WindingFill (nonzero), \c OddEvenFill is not supported.
    \endlist

    From a functionality point of view, QCPainterPath is more similar to HTML Canvas
    \l{https://developer.mozilla.org/en-US/docs/Web/API/Path2D} {Path2D},
    with some additions and the API matching to QCPainter.

    \section1 PathGroups and caching

    Painting paths through QCPainterPath allows the engine to cache the path
    geometry (vertices). This improves the performance of static paths,
    while potentially increasing GPU memory consumption.

    When painting paths using \l{QCPainter::}{fill()} or \l{QCPainter::}{stroke()}
    that take \l QCPainterPath as a parameter, it is possible to set a \c pathGroup
    as a second parameter. This defines the GPU buffer where the path is cached.
    By default, \c pathGroup is \c 0, meaning that the first buffer is used.
    Setting the \c pathGroup to \c -1 means that the path does not allocate its own
    buffer, and the same dynamic buffer is used as with direct painting using
    beginPath() followed by commands and fill/stroke.

    Arranging paths into path groups allows efficient optimization of the rendering
    performance and the GPU memory usage. Paths that belong together and often change
    at the same time should be in the same group for optimal buffer usage.

    When the path changes, its geometry (vertex buffer) is automatically updated.
    Things that cause a geometry update of the path group are:
    \list
    \li Clearing the path elements or adding new elements.
    \li Changing the stroke line width (\l{QCPainter::setLineWidth()}).
    \li Adjusting antialiasing amount (\l{QCPainter::setAntialias()}).
    \li Changing line cap or line join type (\l{QCPainter::setLineCap()}, \l{QCPainter::setLineJoin()}).
    \endlist

    Note that changing the state transform (\l{QCPainter::transform()}, \l{QCPainter::rotate()} etc.)
    does not invalidate the path, so moving/scaling/rotating a cached path is very efficient.

    In cases where the path does not need to be painted anymore, or the application
    should release GPU memory, the cache can be released by calling
    \l{QCPainter::removePathGroup()}. This isn't usually needed, as the cached paths
    are automatically released during the painter destructor.

    \sa QCPainter::addPath(), QCPainter::removePathGroup()

*/

/*!
    Constructs an empty path.
*/
QCPainterPath::QCPainterPath()
    : d_ptr(new QCPainterPathPrivate)
{
}

/*!
    Constructs an empty path, allocating space for \a commandsSize amount
    of commands and optionally \a commandsDataSize amount of data.
    If \a commandsDataSize parameter is not given, space is automatically
    reserved for \c{2 * commandsSize} amount of data, which is optimal
    amount when the path commands are straight lines (\l moveTo(),
    \l lineTo(), \l rect()).

    Reserving correct space is an optimization for path creation and
    memory usage. It isn't mandatory as sufficient space will automatically
    be ensured while adding commands to the path.

    \sa reserve()
*/

QCPainterPath::QCPainterPath(qsizetype commandsSize, qsizetype commandsDataSize)
    : d_ptr(new QCPainterPathPrivate)
{
    d_ptr->commands.resize(commandsSize);
    if (commandsDataSize < 0)
        d_ptr->commandsData.resize(2 * commandsSize);
    else
        d_ptr->commandsData.resize(commandsDataSize);

}

/*!
    Constructs a path that is a copy of the given \a path.
*/

QCPainterPath::QCPainterPath(const QCPainterPath &path) noexcept
    : d_ptr(new QCPainterPathPrivate(*path.d_ptr))
{
}

/*!
    Destroys the path.
*/

QCPainterPath::~QCPainterPath(){
    delete d_ptr;
}

/*!
    Assigns the given \a path to this path and returns a reference to
    this path.
*/

QCPainterPath &QCPainterPath::operator=(const QCPainterPath &path) noexcept
{
    QCPainterPath(path).swap(*this);
    return *this;
}

/*!
    \fn QCPainterPath::QCPainterPath(QCPainterPath &&other) noexcept

    Move-constructs a new QCPainterPath from \a other.
*/

/*!
    \fn QCPainterPath &QCPainterPath::operator=(QCPainterPath &&other)

    Move-assigns \a other to this QCPainterPath instance.
*/

/*!
    \fn void QCPainterPath::swap(QCPainterPath &other)
    \memberswap{path}
*/

/*!
   Returns the path as a QVariant.
*/

QCPainterPath::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCPainterPath::operator!=(const QCPainterPath &path) const

    Returns \c true if the path is different from the given \a path;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCPainterPath::operator==(const QCPainterPath &path) const

    Returns \c true if the path is equal to the given \a path; otherwise
    false.

    \sa operator!=()
*/

bool QCPainterPath::operator==(const QCPainterPath &p) const
{
    if (p.d_ptr == d_ptr)
        return true;

    if (p.d_ptr->commandsCount != d_ptr->commandsCount ||
        p.d_ptr->commandsDataCount != d_ptr->commandsDataCount)
        return false;

    // Note: Check commands and data up to their count, not
    // to full size of the lists.
    for (qsizetype i = 0; i < d_ptr->commandsCount; ++i) {
        if (d_ptr->commands.at(i) != p.d_ptr->commands.at(i))
            return false;
    }
    for (qsizetype i = 0; i < d_ptr->commandsDataCount; ++i) {
        if (!qFuzzyCompare(d_ptr->commandsData.at(i), p.d_ptr->commandsData.at(i)))
            return false;
    }

    return true;
}

/*!
    Closes the current subpath by drawing a line to the beginning of
    the subpath, automatically starting a new path.
*/

void QCPainterPath::closePath()
{
    Q_D(QCPainterPath);
    d->appendCommand(QCCommand::Close);
}

/*!
    \overload

    Moves the current position to (\a{x}, \a{y}) and starts a new
    subpath, implicitly closing the previous path.
*/
void QCPainterPath::moveTo(float x, float y)
{
    Q_D(QCPainterPath);
    float data[] = { x, y };
    d->appendCommandsData(data, 2);
    d->appendCommand(QCCommand::MoveTo);

}

/*!
    Moves the current point to the given \a point, implicitly starting
    a new subpath and closing the previous one.
*/
void QCPainterPath::moveTo(QPointF point)
{
    moveTo(float(point.x()), float(point.y()));
}

/*!
   \overload
    Draws a line from the current position to the point (\a{x},
    \a{y}).
*/
void QCPainterPath::lineTo(float x, float y)
{
    Q_D(QCPainterPath);
    float data[] = { x, y };
    d->appendCommandsData(data, 2);
    d->appendCommand(QCCommand::LineTo);
}

/*!
    Adds a straight line from the current position to the given \a
    point.  After the line is drawn, the current position is updated
    to be at the end point of the line.
*/
void QCPainterPath::lineTo(QPointF point)
{
    lineTo(float(point.x()), float(point.y()));
}

/*!
*/
void QCPainterPath::bezierCurveTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y)
{
    Q_D(QCPainterPath);
    float data[] = { cp1X, cp1Y, cp2X, cp2Y, x, y };
    d->appendCommandsData(data, 6);
    d->appendCommand(QCCommand::BezierTo);
}

/*!
    Adds a cubic Bezier curve between the current position and the
    given \a endPoint using the control points specified by \a controlPoint1, and
    \a controlPoint2.

    After the curve is added, the current position is updated to be at
    the end point of the curve.
*/
void QCPainterPath::bezierCurveTo(
    QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(float(controlPoint1.x()), float(controlPoint1.y()),
                  float(controlPoint2.x()), float(controlPoint2.y()),
                  float(endPoint.x()), float(endPoint.y()));
}

/*!
    Adds a quadratic Bezier curve between the current point and the endpoint
    (\a{x}, \a{y}) with the control point specified by
    (\a{cpX}, \a{cpY}).
*/
void QCPainterPath::quadraticCurveTo(float cpX, float cpY, float x, float y)
{
    Q_D(QCPainterPath);
    // Continue from previous point
    const QPointF prev = currentPosition();
    float prevX = prev.x();
    float prevY = prev.y();
    static constexpr float m = 2.0f / 3.0f;
    float cp1X = prevX + m * (cpX - prevX);
    float cp1Y = prevY + m * (cpY - prevY);
    float cp2X = x + m * (cpX - x);
    float cp2Y = y + m * (cpY - y);
    float data[] = { cp1X, cp1Y, cp2X, cp2Y, x, y };
    d->appendCommandsData(data, 6);
    d->appendCommand(QCCommand::BezierTo);
}

/*!
    Adds a quadratic Bezier curve between the current position and the
    given \a endPoint with the control point specified by \a controlPoint.
*/
void QCPainterPath::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(float(controlPoint.x()), float(controlPoint.y()),
                     float(endPoint.x()), float(endPoint.y()));
}

/*!
    Creates an arc using the points QPointF(\a x1, \a y1) and QPointF(\a
    x2, \a y2) with the given \a radius.
*/
void QCPainterPath::arcTo(float x1, float y1, float x2, float y2, float radius)
{
    // Continue from previous point
    const QPointF prev = currentPosition();
    float prevX = prev.x();
    float prevY = prev.y();

#ifdef QCPAINTER_EQUAL_POINTS_CHECKING_ENABLED
    Q_D(QCPainterPath);
    // See if straight line is enough
    if (pointsEquals(prevX, prevY, x1, y1, d->distTol) ||
        pointsEquals(x1, y1, x2, y2, d->distTol) ||
        pointInSegment(x1, y1, prevX, prevY, x2, y2, d->distTol) ||
        radius < d->distTol) {
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
    float dd = radius / std::tan(a * 0.5f);

    float cx, cy, a0, a1;
    QCPainter::PathWinding direction;
    if (crossProduct(dx0, dy0, dx1, dy1) > 0.0f) {
        direction = QCPainter::PathWinding::ClockWise;
        cx = x1 + dx0 * dd + dy0 * radius;
        cy = y1 + dy0 * dd - dx0 * radius;
        a0 = std::atan2(dx0, -dy0);
        a1 = std::atan2(-dx1, dy1);
    } else {
        direction = QCPainter::PathWinding::CounterClockWise;
        cx = x1 + dx0 * dd - dy0 * radius;
        cy = y1 + dy0 * dd + dx0 * radius;
        a0 = std::atan2(-dx0, dy0);
        a1 = std::atan2(dx1, -dy1);
    }

    arc(cx, cy, radius, a0, a1, direction, true);
}

/*!
    Creates an arc using the points \a point1 and \a point2 with the given \a radius.
*/
void QCPainterPath::arcTo(QPointF point1, QPointF point2, float radius)
{
    arcTo(float(point1.x()), float(point1.y()),
          float(point2.x()), float(point2.y()),
          radius);
}

/*!
    Creates an arc centered on QPointF(\a centerX, \a centerY) with the given \a radius,
    starting at an angle of \a a0 radians and ending at \a a1 radians. The arc spans the
    given \a direction. If \a isConnected is \c false, the previous path is closed and
    a new sub-path is started.
*/
void QCPainterPath::arc(
    float centerX,
    float centerY,
    float radius,
    float a0,
    float a1,
    QCPainter::PathWinding direction,
    bool isConnected)
{
    Q_D(QCPainterPath);
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

    QCCommand firstCmd = d->commandsCount == 0 || !isConnected ? QCCommand::MoveTo : QCCommand::LineTo;
    float prevtanx = 0, prevtany = 0;
    float prevvx = 0, prevvy = 0;
    // divsCount is max 5 and min 1
    const int dSize = 2 + divsCount * 6;
    QVarLengthArray<QCCommand, 6> commands(1 + divsCount);
    QVarLengthArray<float, 32> data(dSize);
    int cCount = 0;
    int dCount = 0;
    for (int i = 0; i <= divsCount; i++) {
        float a = a0 + da * (i/(float)divsCount);
        float dx = std::cos(a);
        float dy = std::sin(a);
        float vx = centerX + dx * radius;
        float vy = centerY + dy * radius;
        float tanx = -dy * radius * kappa;
        float tany = dx * radius * kappa;
        if (i == 0) {
            commands[cCount++] = firstCmd;
            data[dCount++] = vx;
            data[dCount++] = vy;
        } else {
            commands[cCount++] = QCCommand::BezierTo;
            data[dCount++] = prevvx + prevtanx;
            data[dCount++] = prevvy + prevtany;
            data[dCount++] = vx - tanx;
            data[dCount++] = vy - tany;
            data[dCount++] = vx;
            data[dCount++] = vy;
        }
        prevvx = vx;
        prevvy = vy;
        prevtanx = tanx;
        prevtany = tany;
    }
    d->appendCommandsData(data.constData(), dCount);
    d->appendCommands(commands.constData(), cCount);
}

/*!
   \overload

    Creates an arc centered on \a centerPoint with the given \a radius,
    starting at an angle of \a a0 radians and ending at \a a1 radians. The arc spans the
    given \a direction. If \a isConnected is \c false, the previous path is closed and
    a new sub-path is started.
*/
void QCPainterPath::arc(
    QPointF centerPoint,
    float radius,
    float a0,
    float a1,
    QCPainter::PathWinding direction,
    bool isConnected)
{
    arc(float(centerPoint.x()), float(centerPoint.y()),
        radius, a0, a1, direction, isConnected);
}

/*!
   Creates a rectangle positioned at QPointF(\a x, \a y) with the given \a width and \a height.
*/
void QCPainterPath::rect(float x, float y, float width, float height)
{
    Q_D(QCPainterPath);
    float data[] = {
        x, y,
        x, y + height,
        x + width, y + height,
        x + width, y
    };
    d->appendCommandsData(data, 8);
    static constexpr QCCommand commands[] = {
        QCCommand::MoveTo,
        QCCommand::LineTo,
        QCCommand::LineTo,
        QCCommand::LineTo,
        QCCommand::Close
    };
    d->appendCommands(commands, 5);
}

/*!
   \overload

    Creates a rectangle specified by \a rect
*/
void QCPainterPath::rect(const QRectF &rect)
{
    this->rect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

/*!
    Adds the given rectangle \a x, \a y, \a width, \a height with rounded corners to the path. The
    corners are quarter circles with the given \a radius.
*/
void QCPainterPath::roundRect(float x, float y, float width, float height, float radius)
{
    static const float MINR = 0.1f;
    const bool noRadius = (radius < MINR);
    if (noRadius) {
        rect(x, y, width, height);
    } else {
        Q_D(QCPainterPath);
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

        d->appendCommandsData(data, 34);
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
        d->appendCommands(commands, 10);
    }
}

/*!
    \overload
    Adds the given rectangle \a rect with rounded corners to the path. The
    corners are quarter circles with the given \a radius.
*/
void QCPainterPath::roundRect(const QRectF &rect, float radius)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radius);
}

/*!
    Adds the rectangle \a x, \a y, \a width, \a height with rounded corners to the path. The
    corners are quarter circles with radius \a radiusTopLeft, \a radiusTopRight
    \a radiusBottomRight and \a radiusBottomLeft, respectively.
*/
void QCPainterPath::roundRect(
    float x,
    float y,
    float width,
    float height,
    float radiusTopLeft,
    float radiusTopRight,
    float radiusBottomRight,
    float radiusBottomLeft)
{
    static const float MINR = 0.1f;
    const bool noRadius = (radiusTopLeft < MINR) && (radiusTopRight < MINR) &&
                          (radiusBottomRight < MINR) && (radiusBottomLeft < MINR);
    if (noRadius) {
        rect(x, y, width, height);
    } else {
        Q_D(QCPainterPath);
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

        d->appendCommandsData(data, 34);
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
        d->appendCommands(commands, 10);
    }
}

/*!
    \overload
    Adds the rectangle \a rect with rounded corners to the path. The
    corners are quarter circles with radius \a radiusTopLeft, \a radiusTopRight
    \a radiusBottomRight and \a radiusBottomLeft, respectively.
*/
void QCPainterPath::roundRect(
    const QRectF &rect,
    float radiusTopLeft,
    float radiusTopRight,
    float radiusBottomRight,
    float radiusBottomLeft)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radiusTopLeft, radiusTopRight,
              radiusBottomRight, radiusBottomLeft);
}

/*!
    Creates an ellipse centered at (\a x, \a y), with radii defined by \a radiusX, \a radiusY
    and adds it to the path as a closed subpath.
*/
void QCPainterPath::ellipse(float x, float y, float radiusX, float radiusY)
{
    Q_D(QCPainterPath);
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
    d->appendCommandsData(data, 26);
    static constexpr QCCommand commands[] = {
        QCCommand::MoveTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
    };
    d->appendCommands(commands, 5);
}

/*!
    \overload
    Creates an ellipse within the rectangle \a rect
    and adds it to the path as a closed subpath.
*/
void QCPainterPath::ellipse(const QRectF &rect)
{
    ellipse(float(rect.x() + rect.width() * 0.5),
            float(rect.y() + rect.height() * 0.5),
            float(rect.width() * 0.5),
            float(rect.height() * 0.5));
}

/*!
   Adds a circle with center at QPointF(\a x, \a y) and the given \a radius to the path.
*/
void QCPainterPath::circle(float x, float y, float radius)
{
    Q_D(QCPainterPath);
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
    d->appendCommandsData(data, 26);
    static constexpr QCCommand commands[] = {
        QCCommand::MoveTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
        QCCommand::BezierTo,
    };
    d->appendCommands(commands, 5);
}

/*!
   \overload
   Adds a circle with center at \a centerPoint and the given \a radius to the path.
*/
void QCPainterPath::circle(QPointF centerPoint, float radius)
{
    circle(float(centerPoint.x()), float(centerPoint.y()), radius);
}

/*!
    Sets the current sub-path \a winding to either \c QCPainter::CounterClockWise (default)
    or \c QCPainter::ClockWise. CounterClockWise draws solid subpaths while ClockWise draws holes.
*/
void QCPainterPath::setPathWinding(QCPainter::PathWinding winding)
{
    Q_D(QCPainterPath);
    QCCommand c = winding == QCPainter::PathWinding::ClockWise ?
                      QCCommand::WindingCW : QCCommand::WindingCCW;
    d->appendCommand(c);
}

/*!
    Adds \a path into this path, optionally using \a transform to
    alter the path points. When \a transform is not provided (or it is
    identity matrix), this operation is very fast as it reuses the path data.
*/

void QCPainterPath::addPath(const QCPainterPath &path, const QTransform &transform)
{
    addPath(path, qsizetype(0), path.commandsSize(), transform);
}

/*!
    Adds \a path into the current path, starting from the command at \a start
    and including \a count amount of commands. Optionally using \a transform to
    alter the path points.
    The range of \a start and \a count is checked, so that commands are not
    accessed more than \l QCPainterPath::commandsSize().
    In case the path shouldn't continue from the current path position, call
    first \l moveTo() with \c{path.positionAt(start - 1)}.
*/

void QCPainterPath::addPath(const QCPainterPath &path, qsizetype start, qsizetype count, const QTransform &transform)
{
    Q_D(QCPainterPath);
    const auto *pathd = path.d_ptr;

    const auto commandsSize = pathd->commandsCount;
    int commandsDataStart = 0;
    int commandsDataCount = 0;
    if (start == 0 && count == commandsSize) {
        // Adding full path
        commandsDataCount = int(pathd->commandsDataCount);
    } else {
        // Make sure start & count are inside the valid range.
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

    // There are always even amount of data as they are (x, y) points.
    Q_ASSERT(commandsDataCount % 2 == 0);
    d->ensureCommandsData(commandsDataCount);
    if (transform.isIdentity()) {
        // Add path data directly
        for (int i = 0; i < commandsDataCount; i++)
            d->commandsData[d->commandsDataCount++] = pathd->commandsData[commandsDataStart + i];
    } else {
        // Apply transform to path data and add
        int i = 0;
        while (i < commandsDataCount) {
            qreal p1 = pathd->commandsData.at(commandsDataStart + i);
            qreal p2 = pathd->commandsData.at(commandsDataStart + i + 1);
            transform.map(p1, p2, &p1, &p2);
            d->commandsData[d->commandsDataCount++] = float(p1);
            d->commandsData[d->commandsDataCount++] = float(p2);
            i += 2;
        }
    }
    // Add path commands
    d->ensureCommands(count);
    for (int i = 0; i < count; i++)
        d->commands[d->commandsCount++] = pathd->commands[start + i];
}

/*!
    Returns true when the path is empty.

    \sa clear
*/
bool QCPainterPath::isEmpty() const
{
    Q_D(const QCPainterPath);
    return d->commandsCount == 0;
}

/*!
    Clears the path commands and data.

    Call this when the path commands change to recreate the path.
    This does not affect the memory usage, use reserve() and squeeze() for that.

    \sa reserve(), squeeze()
*/
void QCPainterPath::clear()
{
    Q_D(QCPainterPath);
    d->commandsCount = 0;
    d->commandsDataCount = 0;
    d->pathIterations++;
}

/*!
    Releases any memory not required to store the path commands and data.
    This can be used to reduce the memory usage after calling the \l reserve().

    Normally this is not needed to be used, but it can be useful when the path size has
    been big due to reserving or adding many elements (\l lineTo, \l bezierCurveTo etc.) and
    then size is expected to be much smaller in future so calling first \c reserve()
    and then \c squeeze(), will release some memory.

    \sa reserve()
*/
void QCPainterPath::squeeze()
{
    Q_D(QCPainterPath);
    d->commands.squeeze();
    d->commandsData.squeeze();
}

/*!
    Returns the amount of commands in the path.

    \note Some path elements require several commands. For example \l moveTo and \l lineTo require
    \c 1 command, \l bezierCurveTo requires \c 6 commands and \l roundRect \c 10 commands.
*/
qsizetype QCPainterPath::commandsSize() const
{
    Q_D(const QCPainterPath);
    return d->commandsCount;
}

/*!
    Returns the amount of commands data in the path.

    Commands data basically means the points required by the commands.

    \note Some path elements require several data points. For example \l closePath requires
    \c 0, \l moveTo and \l lineTo require \c 2, \l bezierCurveTo requires 6 and \l roundRect
    requires \c 34 data points.
*/
qsizetype QCPainterPath::commandsDataSize() const
{
    Q_D(const QCPainterPath);
    return d->commandsDataCount;
}

/*!
    Returns the capacity of commands in the path.

    \sa commandsDataCapacity(), reserve()
*/
qsizetype QCPainterPath::commandsCapacity() const
{
    Q_D(const QCPainterPath);
    return d->commands.size();
}

/*!
    Returns the capacity of commands data in the path.

    \sa commandsCapacity(), reserve()
*/
qsizetype QCPainterPath::commandsDataCapacity() const
{
    Q_D(const QCPainterPath);
    return d->commandsData.size();
}

/*!
    Reserves a given amounts of space in QCPainterPath's internal memory.

    Attempts to allocate memory for at least \a commandsSize commands
    and \a commandsDataSize data points. Some path elements require
    multiple commands, see \l commandsSize() and \l commandsDataSize().
    If \a commandsDataSize parameter is not given, space is automatically
    reserved for \c{2 * commandsSize} amount of data, which is optimal
    amount when the path commands are straight lines (\l moveTo(),
    \l lineTo(), \l rect()).

    Reserving correct space is an optimization for path creation and
    memory usage. It isn't mandatory as sufficient space will automatically
    be ensured while adding commands into the path.

    \sa squeeze(), commandsCapacity(), commandsDataCapacity()
*/

void QCPainterPath::reserve(qsizetype commandsSize, qsizetype commandsDataSize)
{
    Q_D(QCPainterPath);
    d->commands.resize(commandsSize);
    if (commandsDataSize < 0)
        d->commandsData.resize(2 * commandsSize);
    else
        d->commandsData.resize(commandsDataSize);

}

/*!
    Returns the current position of the path.
    This means position where previous path command (\l moveTo, \l lineTo, \l bezierCurveTo etc.) has ended.
    When the path is empty, returns (0.0, 0.0).
*/
QPointF QCPainterPath::currentPosition() const
{
    Q_D(const QCPainterPath);
    if (d->commandsDataCount < 2)
        return QPointF();
    const float prevX = d->commandsData.at(d->commandsDataCount - 2);
    const float prevY = d->commandsData.at(d->commandsDataCount - 1);
    return QPointF(prevX, prevY);
}

/*!
    Returns the position of the path at \a index.
    This means position where path command (\l moveTo, \l lineTo, \l bezierCurveTo etc.)
    is at \a index.
    The index need to be between \c 0 and \l commandsSize() - 1.
    When the path is empty, returns (0.0, 0.0).
*/

QPointF QCPainterPath::positionAt(qsizetype index) const
{
    Q_D(const QCPainterPath);
    if (d->commandsDataCount < 2)
        return QPointF();
    index = qBound(0, index, d->commandsCount - 1);
    // Locate commandsData index matching to given commands index.
    qsizetype dataIndex = 0;
    for (int i = 0; i < (index + 1); i++)
        dataIndex += QCPainterPathPrivate::dataSizeOf(d->commands.at(i));
    const float posX = d->commandsData.at(dataIndex - 2);
    const float posY = d->commandsData.at(dataIndex - 1);
    return QPointF(posX, posY);
}

/*!
    Return this path, starting from the command at \a start
    and including \a count amount of commands. Optionally using \a transform to
    alter the path points.
    The range of \a start and \a count is checked, so that commands are not
    accessed more than \l commandsSize().
    In case the command at \a start is not \c MoveTo, the first command
    will be replaced with \c MoveTo so that this slice is an individual path.
*/

QCPainterPath QCPainterPath::sliced(qsizetype start, qsizetype count, const QTransform &transform) const &
{
    Q_D(const QCPainterPath);
    QCPainterPath path;
    if (d->commandsCount > start) {
        // We don't know exact amount of commandData, so default
        // 2 * (commands)count is a good estimation.
        path.reserve(count);
        if (d->commands.at(start) != QCCommand::MoveTo) {
            path.moveTo(positionAt(start));
            path.addPath(*this, start + 1, count - 1, transform);
        } else {
            path.addPath(*this, start, count, transform);
        }
    }
    return path;
}

// *** Private ***

// Append a single \a command.
void QCPainterPathPrivate::appendCommand(QCCommand command)
{
    ensureCommands(1);
    auto &c = this->commands;
    c[commandsCount++] = command;
}

// Append \a cCount amount of \a commands.
void QCPainterPathPrivate::appendCommands(const QCCommand commands[], int cCount)
{
    Q_ASSERT(cCount > 0);

    ensureCommands(cCount);
    auto &c = this->commands;
    for (int i = 0; i < cCount; i++)
        c[commandsCount++] = commands[i];
}

// Append \a dCount amount of \a commands data.
// Note: Compared to engine appendCommandsData, in QCPainterPath
// the commands are not transformed at this point.
void QCPainterPathPrivate::appendCommandsData(const float commandsData[], int dCount)
{
    // There are always even amount of data as they are (x, y) points.
    Q_ASSERT(dCount % 2 == 0);

    ensureCommandsData(dCount);
    auto &c = this->commandsData;
    for (int i = 0; i < dCount; i++)
        c[commandsDataCount++] = commandsData[i];
}

// Makes sure there is space for at least \a addition
// amount of new commands.
void QCPainterPathPrivate::ensureCommands(int addition)
{
    auto &c = commands;
    if (commandsCount + addition > c.size()) {
        // Increase capacity with the commands amount + 20% of the current size.
        int newSize = (commandsCount + addition) + c.size() * 0.2;
        newSize = std::max(QCPAINTER_MINIMUM_PATH_COMMANDS_SIZE, newSize);
        c.resize(newSize);
    }
}

// Makes sure there is space for at least \a addition
// amount of new commands data.
void QCPainterPathPrivate::ensureCommandsData(int addition)
{
    auto &c = commandsData;
    if (commandsDataCount + addition > c.size()) {
        // Increase capacity with the data amount + 20% of the current size.
        int newSize = (commandsDataCount + addition) + c.size() * 0.2;
        newSize = std::max(QCPAINTER_MINIMUM_PATH_COMMANDS_DATA_SIZE, newSize);
        c.resize(newSize);
    }
}

QT_END_NAMESPACE
