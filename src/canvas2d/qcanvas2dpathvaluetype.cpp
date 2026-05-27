// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dpathvaluetype_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlvaluetype path2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides path2d type, a painter path container matching to QCanvasPath.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasPath
*/

/*!
  \qmlmethod void path2d::closePath()
   Closes the current subpath by drawing a line to the beginning of the subpath, automatically starting a new path.
   The current point of the new path is the previous subpath's first point.
*/

void QCanvas2DPathValueType::closePath()
{
    QCanvasPath::closePath();

}

/*!
  \qmlmethod void path2d::moveTo(real x, real y)

   Creates a new subpath with a point at (\a x, \a y).
*/

void QCanvas2DPathValueType::moveTo(float x, float y)
{
    QCanvasPath::moveTo(x, y);
}

/*!
  \qmlmethod void path2d::lineTo(real x, real y)

   Draws a line from the current position to the point at (\a x, \a y).
*/

void QCanvas2DPathValueType::lineTo(float x, float y)
{
    QCanvasPath::lineTo(x, y);
}

/*!
  \qmlmethod void path2d::bezierCurveTo(real cp1x, real cp1y, real cp2x, real cp2y, real x, real y)

  Adds a cubic bezier curve between the current position and the given endPoint using the control points specified by (\a {cp1x}, \a {cp1y}),
  and (\a {cp2x}, \a {cp2y}).
  After the curve is added, the current position is updated to be at the end point (\a {x}, \a {y}) of the curve.
*/

void QCanvas2DPathValueType::bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y)
{
    QCanvasPath::bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
}

/*!
    \qmlmethod void path2d::quadraticCurveTo(real cpx, real cpy, real x, real y)

    Adds a quadratic bezier curve between the current point and the endpoint
    (\a x, \a y) with the control point specified by (\a cpx, \a cpy).
*/

void QCanvas2DPathValueType::quadraticCurveTo(float cpx, float cpy, float x, float y)
{
    QCanvasPath::quadraticCurveTo(cpx, cpy, x, y);
}

/*!
    \qmlmethod void path2d::arcTo(real x1, real y1, real x2,
        real y2, real radius)

    Adds an arc with starting point (\a x1, \a y1), ending point (\a x2, \a y2),
    and \a radius to the current subpath and connects it to the previous subpath
    by a straight line.

*/

void QCanvas2DPathValueType::arcTo(float x1, float y1, float x2, float y2, float radius)
{
    QCanvasPath::arcTo(x1, y1, x2, y2, radius);
}

/*!
    \qmlmethod void path2d::arc(real x, real y, real radius,
        real startAngle, real endAngle, bool counterClockWise)

    Adds an arc to the current subpath that lies on the circumference of the
    circle whose center is at the point (\a x, \a y) and whose radius is
    \a radius.

    Both \a startAngle and \a endAngle are measured from the x-axis in radians.

    The default curve direction is clockwise. To change direction to opposite,
    set \a counterClockWise to true.
*/

void QCanvas2DPathValueType::arc(
        float x,
        float y,
        float radius,
        float startAngle,
        float endAngle,
        bool counterClockWise)
{
    auto direction = counterClockWise ? QCanvasPainter::PathWinding::CounterClockWise :
            QCanvasPainter::PathWinding::ClockWise;
    QCanvasPath::arc(x, y, radius, startAngle, endAngle, direction);
}

/*!
    \qmlmethod void path2d::rect(real x, real y, real width, real height)

    Adds a rectangle at position (\a x, \a y), with the given width \a width and
    height \a height, as a closed subpath.
*/

void QCanvas2DPathValueType::rect(float x, float y, float width, float height)
{
    QCanvasPath::rect(x, y, width, height);
}

/*!
    \qmlmethod void path2d::roundRect(real x, real y, real width, real height, real radius)

    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a width, \a height),
    to the path. The \a radius argument specify the radius of the
    ellipses defining the corners of the rounded rectangle.
*/

void QCanvas2DPathValueType::roundRect(float x, float y, float width, float height, float radius)
{
    QCanvasPath::roundRect(x, y, width, height, radius);
}

/*!
    \qmlmethod void path2d::roundRect(real x, real y, real width, real height,
                   real radiusTopLeft, real radiusTopRight,
                   real radiusBottomRight, real radiusBottomLeft)
    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a width, \a height),
    to the path. The \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight and
    \a radiusBottomLeft arguments specify the radius of the ellipses defining the
    corners of the rounded rectangle.
*/

void QCanvas2DPathValueType::roundRect(
        float x,
        float y,
        float width,
        float height,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft)
{
    QCanvasPath::roundRect(x, y, width, height,
                           radiusTopLeft, radiusTopRight,
                           radiusBottomRight, radiusBottomLeft);
}

/*!
    \qmlmethod void path2d::ellipse(real x, real y, real radiusX, real radiusY)

    Creates new ellipse shaped sub-path into center ( \a x, \a y) with
    \a radiusX and \a radiusY.

    The ellipse is composed of a clockwise curve, starting and finishing at
    zero degrees (the 3 o'clock position).
*/

void QCanvas2DPathValueType::ellipse(float x, float y, float radiusX, float radiusY)
{
    QCanvasPath::ellipse(x, y, radiusX, radiusY);
}

/*!
    \qmlmethod void path2d::ellipseRect(real x, real y, real width, real height)

    Creates an ellipse within the bounding rectangle defined by its top-left
    corner at (\a x, \a y), width \a width and height \a height, and adds it to the
    path as a closed subpath.

    The ellipse is composed of a clockwise curve, starting and finishing at
    zero degrees (the 3 o'clock position).

    \note This method matches to \l QtQuick::Context2D::ellipse()
*/

void QCanvas2DPathValueType::ellipseRect(float x, float y, float width, float height)
{
    const QRectF rect(x, y, width, height);
    QCanvasPath::ellipse(rect);
}

/*!
    \qmlmethod void path2d::circle(real x, real y, real radius)

    Creates a circle defined by its center (\a x, \a y), and
    radius \a radius, and adds it to the path as a closed subpath.

    \note Compared to arc(), this method does not add a straight line from
    the last point in the subpath to the start point of the circle.
*/

void QCanvas2DPathValueType::circle(float x, float y, float radius)
{
    QCanvasPath::circle(x, y, radius);
}

/*!
    \qmlmethod void path2d::setPathWinding(const QString &winding)

    Sets the current sub-path \a winding to either \c "counterclockwise" (default)
    or \c "clockwise". "counterclockwise" draws solid subpaths while "clockwise" draws holes.

    \note This is a command, similar to lineTo, moveTo, etc., and therefore
    setting the winding should be done before the rest of the commands to which
    the changed winding is meant to be applied to.

    \sa beginSolidSubPath(), beginHoleSubPath()
*/

void QCanvas2DPathValueType::setPathWinding(const QString &winding)
{
    QCanvasPainter::PathWinding w = QCanvasPainter::PathWinding::CounterClockWise;
    if (winding == QStringLiteral("clockwise") || winding == QStringLiteral("ClockWise"))
        w = QCanvasPainter::PathWinding::ClockWise;

    QCanvasPath::setPathWinding(w);
}

/*!
  \qmlmethod void path2d::beginSolidSubPath()

  Start a solid subpath.
*/

void QCanvas2DPathValueType::beginSolidSubPath()
{
    QCanvasPath::beginSolidSubPath();
}

/*!
  \qmlmethod void path2d::beginHoleSubPath()

  Start a hole subpath.
*/

void QCanvas2DPathValueType::beginHoleSubPath()
{
    QCanvasPath::beginHoleSubPath();
}

/*!
    \qmlmethod void path2d::addPath(path2d path, transform2d transform)

    Adds a \a path into this path, using \a transform as a transformation matrix.
    Providing \a transform parameter is optional.
*/

void QCanvas2DPathValueType::addPath(const QCanvasPath &path, const QTransform &transform)
{
    QCanvasPath::addPath(path, transform);
}

/*!
    \qmlmethod void path2d::addPath(path2d path, int start, int count, transform2d transform)

    Adds \a path into the current path, starting from the command at \a start
    and including \a count amount of commands. Optionally using \a transform to
    alter the path points.
    The range of \a start and \a count is checked, so that commands are not
    accessed more than the path has commands.
    In case the path shouldn't continue from the current path position, call
    first \l moveTo().
*/
void QCanvas2DPathValueType::addPath(const QCanvasPath &path, qsizetype start, qsizetype count, const QTransform &transform)
{
    QCanvasPath::addPath(path, start, count, transform);
}

/*!
    \qmlmethod void path2d::addPath(string svgPath, transform2d transform)

    Adds a \a svgPath into this path, using \a transform as a transformation matrix.
    Providing \a transform parameter is optional.
*/

void QCanvas2DPathValueType::addPath(const QString &svgPath, const QTransform &transform)
{
    QCanvasPath::addPath(svgPath, transform);
}

/*!
  \qmlmethod bool path2d::isEmpty()

  Returns true when the path does not contain any painting commands.
  \sa clear()
*/

bool QCanvas2DPathValueType::isEmpty() const
{
    return QCanvasPath::isEmpty();
}

/*!
  \qmlmethod void path2d::clear()

  Clears the path from all the painting commands.
  \sa isEmpty()
*/

void QCanvas2DPathValueType::clear()
{
    QCanvasPath::clear();
}

QT_END_NAMESPACE
