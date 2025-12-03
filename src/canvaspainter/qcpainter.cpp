// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcpainter.h"
#include "qcpainter_p.h"

#include "engine/qcpainterengine_p.h"
#include "engine/qcpainterrhirenderer_p.h"
#include <rhi/qrhi.h>
#include "qcbrush.h"
#include "qccustombrush.h"

#include "qctext.h"
#include "qcimage_p.h"
#include "qcboxshadow.h"

#include <QtGui/QScreen>
#include <QtGui/qguiapplication.h> // Used by mmToPx & ptToPx
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QC_INFO, "qt.qcpainter.general")

/*!
    \class QCPainter
    \brief The QCPainter class performs hardware-accelerated painting on QRhi.
    \inmodule QtCanvasPainter

    Qt CanvasPainter (QCPainter) provides painting API optimized for
    harware-accelerated (GPU) painting. The API follows closely HTML Canvas 2D
    Context specification, ported to Qt C++. It is also influenced by QPainter,
    but with a more compact API.

    Here's a simple example of using QCPainter.
    \code
    QRectF rect(50, 50, 120, 60);
    QRectF shadowRect = rect.translated(2, 4);
    // Paint shadow
    QCBoxShadow shadow(shadowRect, 30, 15, "#60373F26");
    painter.drawBoxShadow(shadow);
    // Paint rounded rect
    painter.beginPath();
    painter.roundRect(rect, 30);
    painter.setFillStyle("#DBEB00");
    painter.fill();
    // Paint text
    painter.setTextAlign(QCPainter::TextAlign::Center);
    painter.setTextBaseline(QCPainter::TextBaseline::Middle);
    QFont font("Titillium Web", 18);
    painter.setFont(font);
    painter.setFillStyle("#373F26");
    painter.fillText("HELLO!", rect);
    \endcode

    \image hello_qcpainter.png

    For the most parts and from the naming perspective, QCPainter follow closely the
    HTML Canvas 2D Context (https://html.spec.whatwg.org/multipage/canvas.html#2dcontext).
    This makes the API familiar to use for many developers, and with the ability
    to easily reuse existing canvas code. But the aim is NOT to be 100% compatible with
    the HTML canvas. QCPainter misses some of the features to make it simpler,
    more performant on QRhi hardware accelerated graphics API, and to better target
    modern UI needs. Due to these reasons, QCPainter also has additional features
    compared to HTML Canvas 2D Context.

    These are some of the functionality we are at least currently missing compared
    to HTML canvas:
    \list
    \li Clipping: All clipping is (transformed) rectangle and clipping to path shapes
    are not supported.
    \li Fill mode: Only the default Non-zero fillrule is supported, no support for
    Even-odd fillrule.
    \li Dashes: Strokes are always solid lines, dashed/dotted stroke patterns are
    not supported.
    \li Path testing: There are no isPointInPath() or isPointInStroke() methods.
    \li Text stroking: No support for outline stroking of text.
    \li Filter: Canvas SVG filter effects are not supported.
    \li CompositeModes: The amount of composite modes is limited to 3, which can
    be supported without rendering into extra buffers.
    \li Shadows: Built-in shadow methods are not supported.
    \endlist

    On the other hand, some of the additional features QCPainter offers compared
    to HTML canvas include:
    \list
    \li Path groups: QCPainter allows painting to static paths and caching these
    paths as groups for optimal GPU usage.
    \li Adjustable antialiasing: Due to path vertex antialiasing and SDF text
    rendering, the pixel amount of antialiasing can be freely adjusted for
    smoother painting.
    \li Box Gradient: In addition to linear, radial and conical gradients,
    QCPainter supports also rounded rectangle box gradient.
    \li Box Shadow: QCPainter supports also CSS box-shadow type of brush. The
    rendering uses SDF approach similar to Qt Quick RectangularShadow
    (https://doc.qt.io/qt-6/qml-qtquick-effects-rectangularshadow.html), making
    it very performant.
    \li Custom brushes: QCPainter also allows filling & stroking with custom
    vertex and fragment shaders. These can be used also for text.
    \li Text wrapping: QCPainter supports automatic wrapping of text into
    multiple lines, with different wrapping modes.
    \endlist

    QCPainter is architecture agnostic, and usable for both Qt Quick and
    Qt Widgets applications. Actually, it is usable even without either of
    those, with just QWindow and QRhi. To utilize QCPainter, use it from
    one of these classes, dending on the architecture of your application:
    \list
    \li Qt Quick: Use \l QQuickCPainterItem and \l QQuickCPainterRenderer.
    \li Qt Widgets: Use \l QCPainterWidget.
    \li QRhi-based QWindow, or offscreen QRhi buffers: Use \l QCPainterFactory and \c QCRhiPaintDriver.
    \endlist
*/

//     TODO: Move this into proper place as doc with example code.
/*
    PathGroups and caching

    Painting paths through QCPainterPath allows engine to cache the path
    geometry (vertices). This improves the performance of static paths,
    while potentially increasing GPU memory consumption.

    When painting paths using \l QCPainterPath and \l fill() \l or stroke(),
    it is possible to set a \c pathGroup as a second parameter. This defines
    the GPU buffer where the path is cached. By default, \c pathGroup is \c 0
    meaning that the first buffer is used. Setting \c pathGroup to \c -1 means
    that path does not allocate own buffer, but same dynamic buffer is used
    as with direct painting using beginPath() followed by commands and fill/stroke.

    Arranging paths into pathGroups allows effeciently optimizing the rendering
    performance and GPU memory usage. Paths which belong together and often change
    at the same time, should be in the same group for optimal buffer usage.

    When the path changes, geometry is automatically updated.
    Things that cause geometry update of the path group:
    - Clearing the path elements or adding new elements.
    - Changing the stroke line width (\a QCPainter::setLineWidth() ).
    - Adjusting antialiasing amount (\a QCPainter::setAntialias() ).
    - Changing state transform (\a QCPainter::transform(), \a QCPainter::rotate() etc. ).
    - Changing line cap or line join type (\a QCPainter::setLineCap(), \a QCPainter::setLineJoin() ).

    In cases where path does not need to be painted anymore, or application should
    release GPU memory, cache can be released by calling \a removePathGroup().
    This isn't usually needed, as path caches are released in the painter destructor.

    \sa removePathGroup()
*/

/*!
    \enum QCPainter::PathWinding

    PathWinding is used to specify the direction of path drawing.
    This direction is used to determine if a subpath is solid or hole in the path.

    \value CounterClockWise (default) Counter-clockwise winding for solid shapes.

    \value ClockWise Clockwise winding for holes.

    \sa setPathWinding()
*/

/*!
    \enum QCPainter::LineCap

    LineCap is used to define how the end of the line (cap) is drawn.

    \value Butt (default) Square line ending that does not cover the end point of the line.

    \value Round Round line ending.

    \value Square Square line ending that covers the end point and extends beyond it by half the line width.

    \sa setLineCap()
*/

/*!
    \enum QCPainter::LineJoin

    LineJoin is used to define how the joins between two connected lines are drawn.

    \value Round Circular arc between the two lines is filled.

    \value Bevel The triangular notch between the two lines is filled.

    \value Miter (default) The outer edges of the lines are extended to meet at an angle, and this area is filled.

    \sa setLineJoin(), setMiterLimit()
*/

/*!
    \enum QCPainter::TextAlign

    TextAlign is used to define how the text is aligned horizontally.

    \value Left Align the left side of the text horizontally to the specified position.

    \value Right Align the right side of the text horizontally to the specified position.

    \value Center Align the center of the text horizontally to the specified position.

    \value Start (default) The text is aligned at the normal start of the line (left-aligned for left-to-right locales, right-aligned for right-to-left locales).

    \value End The text is aligned at the normal end of the line (right-aligned for left-to-right locales, left-aligned for right-to-left locales).

    \sa setTextAlign(), setTextDirection(), fillText()
*/

/*!
    \enum QCPainter::TextBaseline

    TextBaseline is used to define how the text is aligned (baselined) vertically.

    \value Top Align the top of the text vertically to the specified position.

    \value Hanging Align the hanging baseline of the text vertically to the specified position.

    \value Middle Align the middle of the text vertically to the specified position.

    \value Alphabetic (default) Align the baseline of the text vertically to the specified position.

    \value Bottom Align the bottom of the text vertically to the specified position.

    \sa setTextBaseline(), fillText()
*/

/*!
    \enum QCPainter::TextDirection

    TextDirection is used to define how the text is aligned horizontally.

    \value LeftToRight The text direction is left-to-right.

    \value RightToLeft The text direction is right-to-left.

    \value Inherit (default) The text direction is inherited from QGuiApplication layoutDirection. See https://doc.qt.io/qt-6/qguiapplication.html#layoutDirection-prop.

    \value Auto The text direction is detected automatically based from the text string. See \l QString::isRightToLeft().
    \note As this requires analyzing the text, it is potentially slower that other options.

    \sa setTextDirection(), setTextAlign(), fillText()
*/

/*!
    \enum QCPainter::CompositeOperation

    Qt CanvasPainter supports 3 composite operations:

    \value SourceOver The default value. Draws new shapes on top of the existing content.
    \value SourceAtop The new shape is only drawn where it overlaps the existing content.
    \value DestinationOut The existing content is kept where it doesn't overlap with the new shape.

    \sa setGlobalCompositeOperation()
*/

/*!
    \enum QCPainter::WrapMode

    WrapMode is used to define how the text is wrapped to multiple lines.

    \value NoWrap (default) No wrapping will be performed. If the text contains insufficient newlines, then contentWidth will exceed a set width.

    \value Wrap If possible, wrapping occurs at a word boundary; otherwise it will occur at the appropriate point on the line, even in the middle of a word.

    \value WordWrap Wrapping is done on word boundaries only. If a word is too long, content width will exceed a set width.

    \value WrapAnywhere Wrapping is done at any point on a line, even if it occurs in the middle of a word.

    \sa setTextWrapMode(), fillText()
*/

/*!
    \enum QCPainter::ImageFlag

    This enum specifies flags related to images. Use with
    \l addImage() to set the flags.

    \value GenerateMipmaps Set this to generate mipmaps for the image.
    Mipmaps should be used when smoother output is preferred for images
    which are scaled to smaller than the original size.
    \value RepeatX Use with image pattern to repeate image in X-coordinate.
    \value RepeatY Use with image pattern to repeate image in Y-coordinate.
    \value Repeat Use with image pattern to repeate image in both coordinates.
    \value FlipY Flips (inverses) image in Y direction when rendered.
    \value Premultiplied Image data has premultiplied alpha.
    \value Nearest Image interpolation is Nearest instead Linear
    \value NativeTexture Signifies this is a texture outside of QCPainter.
*/

/*!
    \enum QCPainter::RenderHint

    This enum specifies flags to QCPainter related to rendering. Use
    \l setRenderHint() to set the flags.

    \value Antialiasing Setting this to false disables antialiasing.
    Enabling it results into higher rendering cost.
    The default value is true.

    \value HighQualityStroking Setting this to true gives a more correct
    rendering in some less common cases where stroking overlaps and
    doesn't have full opacity. Enabling it results into higher rendering cost.
    The default value is false.
*/

/*!
    Constructs a painter with \a parent.
*/

QCPainter::QCPainter(QObject *parent)
 : QObject(*new QCPainterPrivate, parent)
{
}

/*!
    Destroys the painter.
*/

QCPainter::~QCPainter()
{
}

// *** State Handling ***

/*!
    Pushes and saves the current render state into a state stack.
    A matching restore() must be used to restore the state.

    \sa restore()
*/

void QCPainter::save()
{
    Q_D(QCPainter);
    d->m_e->save();
}

/*!
    Pops and restores current render state.
    So previously saved state will be restored.
    If save() has not been called and the state stack
    is empty, calling this does nothing.
    \sa save()
*/

void QCPainter::restore()
{
    Q_D(QCPainter);
    d->m_e->restore();
}

/*!
    Resets the current painter state to default values.

    \sa save(), restore()
*/

void QCPainter::reset()
{
    Q_D(QCPainter);
    d->m_e->reset();
}

// *** Render styles ***

/*!
    Sets the stroke style to a solid \a color.
    The default stroke style is solid black color (0, 0, 0, 1).
*/

void QCPainter::setStrokeStyle(const QColor &color)
{
    Q_D(QCPainter);
    d->m_e->setStrokeColor(color);
}

/*!
    \overload

    Sets the stroke style to \a brush.
    The default stroke style is solid black color (0, 0, 0, 1).
*/

void QCPainter::setStrokeStyle(const QCBrush &brush)
{
    Q_D(QCPainter);
    if (brush.type() == QCBrush::BrushType::Custom) {
        auto b = static_cast<QCCustomBrush *>(const_cast<QCBrush *>(&brush));
        d->m_e->setCustomStrokeBrush(b);
    } else {
        d->m_e->setStrokePaint(brush.createPaint(this));
    }
}

/*!
    Sets the fill style to a solid \a color.
    The default fill style is solid black color (0, 0, 0, 1).
*/

void QCPainter::setFillStyle(const QColor &color)
{
    Q_D(QCPainter);
    d->m_e->setFillColor(color);
}

/*!
    \overload

    Sets the fill style to \a brush.
    The default fill style is solid black color (0, 0, 0, 1).
*/

void QCPainter::setFillStyle(const QCBrush &brush)
{
    Q_D(QCPainter);
    if (brush.type() == QCBrush::BrushType::Custom) {
        auto b = static_cast<QCCustomBrush *>(const_cast<QCBrush *>(&brush));
        d->m_e->setCustomFillBrush(b);
    } else {
        d->m_e->setFillPaint(brush.createPaint(this));
    }
}

/*!
    Sets the miter limit to \a limit. Miter limit controls when a sharp corner
    is beveled. When the corner length would become longer than this limit,
    QCPainter::LineJoin::Bevel will be applied between the lines instead.
    This has only effect with the QCPainter::LineJoin::Miter line join.
    The default limit is \c 10.0.

    \sa setLineJoin()
*/

void QCPainter::setMiterLimit(float limit)
{
    Q_D(QCPainter);
    d->m_e->setMiterLimit(limit);
}

/*!
    Sets the line width of stroke to \a width in pixels.
    The default line width is \c 1.0.

    \sa stroke()
*/

void QCPainter::setLineWidth(float width)
{
    Q_D(QCPainter);
    d->m_e->setLineWidth(width);
}

/*!
    Sets the end of the line of stoke to \a cap.
    The default line cap is \c QCPainter::LineCap::Butt.
*/

void QCPainter::setLineCap(LineCap cap)
{
    Q_D(QCPainter);
    d->m_e->setLineCap(cap);
}

/*!
    Sets the line join of stroke to \a join.
    The default line join is \c QCPainter::LineJoin::Miter.

    \sa setMiterLimit()
*/

void QCPainter::setLineJoin(LineJoin join)
{
    Q_D(QCPainter);
    d->m_e->setLineJoin(join);
}

/*!
    Sets the global alpha (transparency) value to \a alpha. This alpha value is
    applied to all rendered shapes. Already transparent paths will get
    proportionally more transparent as well.
    Alpha should be between 0.0 (fully transparent) and 1.0 (fully opaque).
    By default alpha is \c 1.0.
*/

void QCPainter::setGlobalAlpha(float alpha)
{
    Q_D(QCPainter);
    d->m_e->setGlobalAlpha(alpha);
}

/*!
    Sets the global composite operation mode to \a operation. This mode is
    applied to all painting operations. The default mode is
    \c QCPainter::CompositeOperation::SourceOver.
*/

void QCPainter::setGlobalCompositeOperation(CompositeOperation operation)
{
    Q_D(QCPainter);
    d->m_e->setGlobalCompositeOperation(operation);
}

/*!
    Sets the global brightness to \a value. This brightess is
    applied to all rendered shapes. A value of 0 will cause painting
    to be completely black. Value can also be bigger than 1.0, to
    increase the brightness.
    By default, brightness is \c 1.0.
*/

void QCPainter::setGlobalBrightness(float value)
{
    Q_D(QCPainter);
    d->m_e->setGlobalBrightness(value);
}

/*!
    Sets the global contrast to \a value. This contrast is
    applied to all rendered shapes. A value of 0 will cause painting
    to be completely gray (0.5, 0.5, 0.5). Value can also be bigger
    than 1.0, to increase the contrast.
    By default, contrast is \c 1.0.
*/

void QCPainter::setGlobalContrast(float value)
{
    Q_D(QCPainter);
    d->m_e->setGlobalContrast(value);
}

/*!
    Sets the global saturations to \a value. This saturations is
    applied to all rendered shapes. A value of 0 will disable saturation
    and cause painting to be completely grayscale. Value can also be bigger
    than 1.0, to increase the saturation.
    By default, saturation is \c 1.0.
*/

void QCPainter::setGlobalSaturate(float value)
{
    Q_D(QCPainter);
    d->m_e->setGlobalSaturate(value);
}

// *** Transforms ***

/*!
    Resets current transform to a identity matrix.
*/

void QCPainter::resetTransform()
{
    Q_D(QCPainter);
    d->m_e->resetTransform();
}

/*!
    Resets the current transform and uses \a transform instead.
*/

void QCPainter::setTransform(const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->setTransform(transform);
}

/*!
    Multiplies the current coordinate system by specified \a transform.
*/

void QCPainter::transform(const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->transform(transform);
}

/*!
    Translates current coordinate system by \a x and \a y.
*/

void QCPainter::translate(float x, float y)
{
    Q_D(QCPainter);
    d->m_e->translate(x, y);
}

/*!
    \overload

    Translates current coordinate system by \a point.
*/

void QCPainter::translate(QPointF point)
{
    translate(float(point.x()),
              float(point.y()));
}

/*!
    Rotates current coordinate system by \a angle. Angle is specified in radians.
*/

void QCPainter::rotate(float angle)
{
    Q_D(QCPainter);
    d->m_e->rotate(angle);
}

/*!
    Skews (shears) the current coordinate system along X axis by \a angleX
    and along Y axis by \a angleY. Angles are specifid in radians.
*/

void QCPainter::skew(float angleX, float angleY)
{
    Q_D(QCPainter);
    d->m_e->skew(angleX, angleY);
}

/*!
    Scales the current coordinat system by \a scale. Both x and y coordinates
    are scaled evenly.
*/

void QCPainter::scale(float scale)
{
    Q_D(QCPainter);
    d->m_e->scale(scale, scale);
}

/*!
    Sets the current brush transform to \a transform. This transform is
    applied to both stroke and fill brushes.
*/
void QCPainter::setBrushTransform(const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->setBrushTransform(transform);
}

/*!
    \overload

    Scales the current coordinat system by \a scaleX and \a scaleY.
*/

void QCPainter::scale(float scaleX, float scaleY)
{
    Q_D(QCPainter);
    d->m_e->scale(scaleX, scaleY);
}

/*!
    Returns the current transform.
*/

const QTransform QCPainter::getTransform() const
{
    Q_D(const QCPainter);
    return d->m_e->currentTransform();
}

// *** Clipping ***

/*!
    Sets the current scissor rectangle to (\a x, \a y, \a width, \a height).
    The scissor rectangle is transformed by the current transform.
    \note Clipping has some performance cost and it should only be used
    when needed.
    \sa resetClipping()
*/

void QCPainter::setClipRect(float x, float y, float width, float height)
{
    Q_D(QCPainter);
    d->m_e->setClipRect(QRectF(x, y, width, height));
}

/*!
    \overload

    Sets the current scissor rectangle to \a rect.
    The scissor rectangle is transformed by the current transform.
    \note Clipping has some performance cost and it should only be used
    when needed.
    \sa resetClipping()
*/

void QCPainter::setClipRect(const QRectF &rect)
{
    setClipRect(float(rect.x()),
                float(rect.y()),
                float(rect.width()),
                float(rect.height()));
}

/*!
    Resets and disables clipping.
    \sa setClipRect()
*/

void QCPainter::resetClipping()
{
    Q_D(QCPainter);
    d->m_e->resetClipRect();
}

//  *** Paths ***

/*!
    Begins drawing a new path while clearing the current path.
*/

void QCPainter::beginPath()
{
    Q_D(QCPainter);
    d->m_e->beginPath();
}

/*!
    Closes the current sub-path with a line segment.
    This is equivalent to lineTo([starting point]) as the last path element.
*/

void QCPainter::closePath()
{
    Q_D(QCPainter);
    d->m_e->closePath();
}

/*!
    Starts new sub-path with ( \a x, \a y) as first point.
*/

void QCPainter::moveTo(float x, float y)
{
    Q_D(QCPainter);
    d->m_e->moveTo(x, y);
}

/*!
    \overload

    Starts new sub-path with \a point as first point.
*/

void QCPainter::moveTo(QPointF point)
{
    moveTo(float(point.x()),
           float(point.y()));
}

/*!
    Adds line segment from the last point in the path to the ( \a x, \a y) point.
*/

void QCPainter::lineTo(float x, float y)
{
    Q_D(QCPainter);
    d->m_e->lineTo(x, y);
}

/*!
    \overload

    Adds line segment from the last point in the path to the \a point.
*/

void QCPainter::lineTo(QPointF point)
{
    lineTo(float(point.x()),
           float(point.y()));
}

/*!
    Adds cubic bezier segment from last point in the path via two
    control points (\a cp1X, \a cp1Y and \a cp2X, \a cp2Y) to the specified point (\a x, \a y).
*/

void QCPainter::bezierCurveTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y)
{
    Q_D(QCPainter);
    d->m_e->bezierTo(cp1X, cp1Y, cp2X, cp2Y, x, y);
}

/*!
    \overload

    Adds cubic bezier segment from last point in the path via two
    control points (\a controlPoint1 and \a controlPoint2) to the specified point \a endPoint.
*/

void QCPainter::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(float(controlPoint1.x()),
                  float(controlPoint1.y()),
                  float(controlPoint2.x()),
                  float(controlPoint2.y()),
                  float(endPoint.x()),
                  float(endPoint.y()));
}

/*!
    Adds quadratic bezier segment from last point in the path via
    a control point (\a cpX, \a cpY) to the specified point (\a x, \a y).
*/

void QCPainter::quadraticCurveTo(float cpX, float cpY, float x, float y)
{
    Q_D(QCPainter);
    d->m_e->quadTo(cpX, cpY, x, y);
}

/*!
    \overload

    Adds quadratic bezier segment from last point in the path via
    a \a controlPoint to the specified \a endPoint.
*/

void QCPainter::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(float(controlPoint.x()),
                     float(controlPoint.y()),
                     float(endPoint.x()),
                     float(endPoint.y()));
}

/*!
    Adds an arc segment at the corner defined by the last path point,
    and two specified points (\a x1, \a y1 and \a x2, \a y2) with \a radius.
*/

void QCPainter::arcTo(float x1, float y1, float x2, float y2, float radius)
{
    Q_D(QCPainter);
    d->m_e->arcTo(x1, y1, x2, y2, radius);
}

/*!
    \overload

    Adds an arc segment at the corner defined by the last path point,
    and two specified points (\a controlPoint1 and \a controlPoint2) with \a radius.
*/

void QCPainter::arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius)
{
    arcTo(float(controlPoint1.x()),
          float(controlPoint1.y()),
          float(controlPoint2.x()),
          float(controlPoint2.y()),
          radius);
}

/*!
    Creates new circle arc shaped sub-path. The arc center is at \a centerX, \a centerY,
    with \a radius, and the arc is drawn from angle \a a0 to \a a1,
    and swept in \a direction (ClockWise or CounterClockWise).
    When \a isConnected is set to false, arc does not add line from the previous
    path position to the start of the arc.
    Angles are specified in radians.
*/

void QCPainter::arc(float centerX, float centerY, float radius, float a0, float a1, PathWinding direction, bool isConnected)
{
    Q_D(QCPainter);
    d->m_e->addArc(centerX, centerY, radius, a0, a1, direction, isConnected);
}

/*!
    \overload

    Creates new circle arc shaped sub-path. The arc center is at \a centerPoint,
    with \a radius, and the arc is drawn from angle \a a0 to \a a1,
    and swept in \a direction (ClockWise or CounterClockWise).
    When \a isConnected is set to false, arc does not add line from the previous
    path position to the start of the arc.
    Angles are specified in radians.
*/

void QCPainter::arc(QPointF centerPoint, float radius, float a0, float a1, PathWinding direction, bool isConnected)
{
    arc(float(centerPoint.x()),
        float(centerPoint.y()),
        radius, a0, a1, direction,
        isConnected);
}

/*!
    Creates new rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height.
*/

void QCPainter::rect(float x, float y, float width, float height)
{
    Q_D(QCPainter);
    d->m_e->addRect(x, y, width, height);
}

/*!
    \overload

    Creates new rectangle shaped sub-path at \a rect.
    This is an overloaded method using QRectF.
*/

void QCPainter::rect(const QRectF &rect)
{
    this->rect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

/*!
    Creates new rounded rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height. Corners rounding will be \a radius.
*/

void QCPainter::roundRect(float x, float y, float width, float height, float radius)
{
    Q_D(QCPainter);
    d->m_e->addRoundRect(x, y, width, height, radius);
}

/*!
    \overload

    Creates new rounded rectangle shaped sub-path at \a rect with \a radius corners.
    This is an overloaded method using QRectF.
*/

void QCPainter::roundRect(const QRectF &rect, float radius)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radius);
}

/*!
    \overload

    Creates new rounded rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height. Corners rounding can be varying per-corner, with
    \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight, \a radiusBottomLeft.
*/

void QCPainter::roundRect(float x, float y, float width, float height, float radiusTopLeft, float radiusTopRight, float radiusBottomRight, float radiusBottomLeft)
{
    Q_D(QCPainter);
    d->m_e->addRoundRect(x, y, width, height, radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

/*!
    \overload

    Creates new rounded rectangle shaped sub-path at \a rect. Corners rounding can be
    varying per-corner, with \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight,
    \a radiusBottomLeft.
*/

void QCPainter::roundRect(const QRectF &rect, float radiusTopLeft, float radiusTopRight, float radiusBottomRight, float radiusBottomLeft)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

/*!
    Creates new ellipse shaped sub-path into ( \a centerX, \a centerY) with \a radiusX and \a radiusY.
*/

void QCPainter::ellipse(float centerX, float centerY, float radiusX, float radiusY)
{
    Q_D(QCPainter);
    d->m_e->addEllipse(centerX, centerY, radiusX, radiusY);
}

/*!
    \overload

    Creates new ellipse shaped sub-path into \a centerPoint with \a radiusX and \a radiusY.
*/

void QCPainter::ellipse(QPointF centerPoint, float radiusX, float radiusY)
{
    ellipse(float(centerPoint.x()),
            float(centerPoint.y()),
            radiusX,
            radiusY);
}

/*!
    \overload

    Creates new ellipse shaped sub-path into \a rect.
    This ellipse will cover the \a rect area.
*/

void QCPainter::ellipse(const QRectF &rect)
{
    ellipse(float(rect.x() + rect.width() * 0.5),
            float(rect.y() + rect.height() * 0.5),
            float(rect.width() * 0.5),
            float(rect.height() * 0.5));
}

/*!
    Creates new circle shaped sub-path into ( \a centerX, \a centerY) with \a radius.
*/

void QCPainter::circle(float centerX, float centerY, float radius)
{
    Q_D(QCPainter);
    d->m_e->addCircle(centerX, centerY, radius);
}

/*!
    \overload

    Creates new circle shaped sub-path into \a centerPoint with \a radius.
*/

void QCPainter::circle(QPointF centerPoint, float radius)
{
    circle(float(centerPoint.x()),
           float(centerPoint.y()), radius);
}

/*!
    Adds \a path into the current path.

    \note QCPainter uses WindingFill (nonzero) fillrule,
    which means that all QPainterPaths don't render correctly. This is notable
    for example when path contains text characters with holes in them.

    \note This method is available mostly for the compatibility with QPainter
    and QPainterPath. It does not increase performance compared to painting
    the path directly with QCPainter methods.
*/

void QCPainter::addPath(const QPainterPath &path)
{
    Q_D(QCPainter);
    d->m_e->addPath(path);
}

/*!
    Adds \a path into the current path, optionally using \a transform to
    alter the path points. When \a transform is not provided (or it is
    identity matrix), this operation is very fast as it reuses the path data.
*/

void QCPainter::addPath(const QCPainterPath &path, const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->addPath(path, transform);
}

/*!
    Adds \a path into the current path, starting from the command at \a start
    and including \a count amount of commands. Optionally using \a transform to
    alter the path points.
    The range of \a start and \a count is checked, so that commands are not
    accessed more than \l QCPainterPath::commandsSize().
    In case the path shouldn't continue from the current path position, call
    first \l moveTo() e.g. with \c{path.positionAt(start - 1)}.
*/

void QCPainter::addPath(const QCPainterPath &path,
                        qsizetype start, qsizetype count,
                        const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->addPath(path, start, count, transform);
}


/*!
    Sets the current sub-path \a winding to either CounterClockWise (default) or ClockWise.
    CounterClockWise draws solid subpaths while ClockWise draws holes.
*/

void QCPainter::setPathWinding(PathWinding winding)
{
    Q_D(QCPainter);
    d->m_e->setPathWinding(winding);
}

/*!
    Start a solid subpath. This is equivalent to
    \c setPathWinding(QCPainter::PathWinding::CounterClockWise))
*/

void QCPainter::beginSolidSubPath()
{
    Q_D(QCPainter);
    d->m_e->setPathWinding(PathWinding::CounterClockWise);
}

/*!
    Start a hole subpath. This is equivalent to
    \c setPathWinding(QCPainter::PathWinding::ClockWise))
*/

void QCPainter::beginHoleSubPath()
{
    Q_D(QCPainter);
    d->m_e->setPathWinding(PathWinding::ClockWise);
}

/*!
    Fills the current path with current fill style.
    \sa setFillStyle()
*/

void QCPainter::fill()
{
    Q_D(QCPainter);
    d->m_e->fill();
}

/*!
    Strokes the current path with current stroke style.
    \sa setStrokeStyle()
*/

void QCPainter::stroke()
{
    Q_D(QCPainter);
    d->m_e->stroke();
}

/*!
    \overload

    Fills the \a path with current fill style and belonging
    into \a pathGroup. By default, \a pathGroup is \c 0, so using the
    first group. When \a pathGroup is \c -1, the path will not be cached
    on GPU side. More information about using path cache group in {TODO: LINK}.
    Transform \a transform is used when filling the path.
    This does not require calling beginPath().
    \sa setFillStyle()
*/

void QCPainter::fill(const QCPainterPath &path, int pathGroup, const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->fill(path, pathGroup, transform);
}

/*!
    \overload

    Strokes the \a path with current stroke style and belonging
    into \a pathGroup. By default, \a pathGroup is \c 0, so using the
    first group. When \a pathGroup is \c -1, the path will not be cached
    on GPU side. More information about using path cache group in {TODO: LINK}.
    Transform \a transform is used when filling the path.
    This does not require calling beginPath().
    \sa setStrokeStyle()
*/

void QCPainter::stroke(const QCPainterPath &path, int pathGroup, const QTransform &transform)
{
    Q_D(QCPainter);
    d->m_e->stroke(path, pathGroup, transform);
}

// *** Direct drawing ***

/*!
    Draws a filled rectangle into specified position ( \a x, \a y) at size \a width, \a height.
    \note This is provided for convenience. When filling more than just a single rect,
    prefer using rect().
*/

void QCPainter::fillRect(float x, float y, float width, float height)
{
    Q_D(QCPainter);
    d->m_e->beginPath();
    d->m_e->addRect(x, y, width, height);
    d->m_e->fill();
}

/*!
    \overload

    Draws a filled rectangle into \a rect.
    This is an overloaded method using QRectF.
    \note This is provided for convenience. When filling more than just a single rect,
    prefer using rect().
*/

void QCPainter::fillRect(const QRectF &rect)
{
    fillRect(float(rect.x()),
             float(rect.y()),
             float(rect.width()),
             float(rect.height()));
}

/*!
    Fills the rectangle specified by \a x, \a y, \a width, \a height with
    transparent black. As clearing does not need blending, it can be faster
    than fillRect().
*/
void QCPainter::clearRect(float x, float y, float width, float height)
{
    Q_D(QCPainter);
    d->m_e->beginPath();
    d->m_e->addRect(x, y, width, height);
    d->m_e->fillForClear();
}

/*!
    \overload
    Fills \a rect with transparent black.
    This is an overloaded method using QRectF.
*/
void QCPainter::clearRect(const QRectF &rect)
{
    clearRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()));
}

/*!
    Draws a stoked rectangle into specified position ( \a x, \a y) at size \a width, \a height.
    \note This is provided for convenience. When stroking more than just a single rect,
    prefer using rect().
*/

void QCPainter::strokeRect(float x, float y, float width, float height)
{
    Q_D(QCPainter);
    d->m_e->beginPath();
    d->m_e->addRect(x, y, width, height);
    d->m_e->stroke();
}

/*!
    \overload

    Draws a stoked rectangle into \a rect.
    This is an overloaded method using QRectF.
    \note This is provided for convenience. When stroking more than just a single rect,
    prefer using rect().
*/

void QCPainter::strokeRect(const QRectF &rect)
{
    strokeRect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

// *** Shadows ***

/*!
    Draws a box \a shadow. The shadow will be painted with the
    position, size, color, blur etc. set in the \a shadow.
    \sa QCBoxShadow
*/

void QCPainter::drawBoxShadow(const QCBoxShadow &shadow)
{
    Q_D(QCPainter);
    d->m_e->save();
    d->m_e->beginPath();
    const auto r = shadow.boundingRect();
    d->m_e->addRect(r.x(), r.y(), r.width(), r.height());
    d->m_e->setFillPaint(shadow.createPaint(this));
    d->m_e->fill();
    // TODO: Remove this or add debugging API.
    const bool shadowRectDebug = false;
    if (shadowRectDebug) {
        d->m_e->setStrokeColor("#60ff0000");
        strokeRect(r);
    }
    d->m_e->restore();
}

// *** Images ***

/*!
    Draw \a image into \a x, \a y, at its default size.
*/

void QCPainter::drawImage(const QCImage &image, float x, float y)
{
    Q_D(QCPainter);
    d->m_e->drawImageId(image.id(), x, y, image.width(), image.height(), image.tintColor());
    d->markTextureIdUsed(image.id());
}

/*!
    \overload

    Draw \a image into \a x, \a y, at given \a width and \a height.
*/

void QCPainter::drawImage(const QCImage &image, float x, float y, float width, float height)
{
    Q_D(QCPainter);
    d->m_e->drawImageId(image.id(), x, y, width, height, image.tintColor());
    d->markTextureIdUsed(image.id());
}

/*!
    \overload

    Draw \a image into position and size of \a destinationRect.
*/

void QCPainter::drawImage(const QCImage &image, const QRectF &destinationRect)
{
    drawImage(image,
              float(destinationRect.x()),
              float(destinationRect.y()),
              float(destinationRect.width()),
              float(destinationRect.height()));
}

/*!
    \overload

    Draw \a image into position and size of \a destinationRect, from \a sourceRect area of image.
*/

void QCPainter::drawImage(const QCImage &image, const QRectF &sourceRect, const QRectF &destinationRect)
{
    Q_D(QCPainter);
    float sx = float(sourceRect.x());
    float sy = float(sourceRect.y());
    float sw = float(sourceRect.width());
    float sh = float(sourceRect.height());
    float dx = float(destinationRect.x());
    float dy = float(destinationRect.y());
    float dw = float(destinationRect.width());
    float dh = float(destinationRect.height());
    float startX = dx - sx * (dw/sw);
    float startY = dy - sy * (dh/sh);
    float endX = dw * image.width() / sw;
    float endY = dh * image.height() / sh;
    QCPaint ip = d->m_e->createImagePattern(startX, startY, endX, endY, image.id(), 0.0f, image.tintColor());
    d->m_e->save();
    d->m_e->beginPath();
    d->m_e->addRect(dx, dy, dw, dh);
    d->m_e->setFillPaint(ip);
    d->m_e->fill();
    d->m_e->restore();
    d->markTextureIdUsed(image.id());
}

// *** Text ***

/*!
    Sets the \a font as currently active font.
*/

void QCPainter::setFont(const QFont &font)
{
    Q_D(QCPainter);
    d->setFont(font);
}

/*!
    Sets the horizontal alignment of text to \a align.
    The default alignment is \c QCPainter::TextAlign::Start.
*/

void QCPainter::setTextAlign(QCPainter::TextAlign align)
{
    Q_D(QCPainter);
    d->m_e->setTextAlignment(align);
}

/*!
    Sets the vertical alignment (baseline) of text to \a baseline.
    The default alignment is \c QCPainter::TextBaseline::Alphabetic.
*/

void QCPainter::setTextBaseline(QCPainter::TextBaseline baseline)
{
    Q_D(QCPainter);
    d->engine()->setTextBaseline(baseline);
}

/*!
    Sets the direction (baseline) of text to \a direction.
    The default direction is \c QCPainter::TextDirection::Inherit.
*/

void QCPainter::setTextDirection(QCPainter::TextDirection direction)
{
    Q_D(QCPainter);
    d->engine()->setTextDirection(direction);
}

/*!
    Sets the text wrap mode to \a wrapMode.
    The default wrap mode is \c QCPainter::WrapMode::NoWrap.
*/

void QCPainter::setTextWrapMode(QCPainter::WrapMode wrapMode)
{
    Q_D(QCPainter);
    d->engine()->setTextWrapMode(wrapMode);
}

/*!
    Sets the line height adjustment in pixels to \a height
    for wrapped text. The default line height is \c 0.
*/

void QCPainter::setTextLineHeight(float height)
{
    Q_D(QCPainter);
    d->engine()->setTextLineHeight(height);
}

/*!
    Set the current text antialiasing amount. The value \a antialias
    is multiplier to normal antialiasing, meaning that \c 0.0 disables
    antialiasing and 2.0 doubles it. The default value is 1.0.

    \note Due to the used text antialiasing technique (SDF),
    the maximum antialiasing amount is quite limited and this
    affects less when the font size is small.
*/

void QCPainter::setTextAntialias(float antialias)
{
    Q_D(QCPainter);
    d->engine()->setTextAntialias(antialias);
}

// TODO: Document or remove, depending if this is used in public API.
void QCPainter::prepareText(const QCText &text)
{
    Q_D(QCPainter);
    d->prepareText(text);
}

/*!
    Draws \a text string at specified location ( \a x, \a y), with current textAlign and textBaseline.
    To make the text wrap into multiple lines, set optional \a maxWidth parameter to preferred
    row width in pixels. White space is stripped at the beginning of the rows,
    the text is split at word boundaries or when new-line characters are encountered.
    Words longer than the max width are split at nearest character (i.e. no hyphenation).

    \a cacheIndex is optional.
*/
void QCPainter::fillText(const QString &text, float x, float y, float maxWidth, int cacheIndex)
{
    // TODO: Document cacheIndex properly.
    Q_D(QCPainter);
    d->fillText(text, x, y, maxWidth, cacheIndex);
}

/*!
    \overload

    Draws \a text string at specified \a point, with current textAlign and textBaseline.
    To make the text wrap into multiple lines, set optional \a maxWidth parameter to preferred
    row width in pixels. White space is stripped at the beginning of the rows,
    the text is split at word boundaries or when new-line characters are encountered.
    Words longer than the max width are split at nearest character (i.e. no hyphenation).

    This is an overloaded method using QPointF.
*/

void QCPainter::fillText(const QString &text, QPointF point, float maxWidth, int cacheIndex)
{
    Q_D(QCPainter);
    d->fillText(text, point.x(), point.y(), maxWidth, cacheIndex);
}

/*!
    \overload

    Draws \a text string inside \a rect, with current textAlign and textBaseline.
    Width of the rect parameter is used as maxWidth.

    This is an overloaded method using QRectF. It is often useful to set the
    text baseline to \l TextBaseline::Top or \l TextBaseline::Middle when painting
    text with this method.

    \a cacheIndex is optional.
*/
void QCPainter::fillText(const QString &text, const QRectF &rect, int cacheIndex)
{
    // TODO: Document cacheIndex properly.
    Q_D(QCPainter);
    d->fillText(text, rect, cacheIndex);
}

/*!
    \overload

    Draws the text container \a text according to its specification.
*/
void QCPainter::fillText(const QCText &text)
{
    Q_D(QCPainter);
    d->fillText(text);
}

/*!
    Measures bounding box of a \a text string at (\a x, \a y).
    To measure multi-line text, set optional \a maxWidth parameter to preferred row width in pixels.
    Returns QRectF with values [xmin, ymin, width, height].
    Measured values are returned in local coordinate space.
*/

QRectF QCPainter::textBoundingBox(const QString &text, float x, float y, float maxWidth)
{
    Q_D(QCPainter);
    return d->textBoundingBox(text, x, y, maxWidth);
}

/*!
    \overload

    Measures bounding box of a \a text string at \a point.
    To measure multi-line text, set optional \a maxWidth parameter to preferred row width in pixels.
    Returns QRectF with values [xmin, ymin, width, height].
    Measured values are returned in local coordinate space.
*/

QRectF QCPainter::textBoundingBox(const QString &text, QPointF point, float maxWidth)
{
    Q_D(QCPainter);
    return d->textBoundingBox(text, point.x(), point.y(), maxWidth);
}

/*!
    \overload

    Measures bounding box of a \a text string at \a rect.
    Returns QRectF with values [xmin, ymin, width, height].
    Measured values are returned in local coordinate space.
*/

QRectF QCPainter::textBoundingBox(const QString &text, const QRectF &rect)
{
    Q_D(QCPainter);
    return d->textBoundingBox(text, rect);
}

// TODO: API Docs
QRectF QCPainter::textBoundingBox(const QCText &text)
{
    Q_D(QCPainter);
    return d->textBoundingBox(text);
}

/*!
    Set the current antialiasing amount to \a antialias in pixels.
    More antialias means smoother painting. This only affects fill and stroke painting,
    not images or texts.
    The default value is \c 1.0.

    Antialiasing can be modified per-path so it can be set before each stroke/fill.
    To disable antialiasing from the whole QQuickCPainterItem, use
    QCPainter::RenderHint::Antialiasing render hint.
    \sa setRenderHints()
*/

void QCPainter::setAntialias(float antialias)
{
    Q_D(QCPainter);
    d->m_e->setAntialias(antialias);
}

/*!
    Returns the ratio between physical pixels and device-independent pixels.
    The default value is \c 1.0.
*/

float QCPainter::devicePixelRatio() const
{
    Q_D(const QCPainter);
    return d->m_devicePixelRatio;
}

/*!
    Sets the given render \a hint on the painter if \a on is true;
    otherwise clears the render hint.

    \sa setRenderHints(), renderHints()
*/

void QCPainter::setRenderHint(RenderHint hint, bool on)
{
    setRenderHints(hint, on);
}

/*!
    Sets the given render \a hints on the painter if \a on is true;
    otherwise clears the render hints.

    \sa setRenderHint(), renderHints()
*/

void QCPainter::setRenderHints(RenderHints hints, bool on)
{
    Q_D(QCPainter);
    d->m_e->setRenderHints(hints, on);
}

/*!
    Returns a flag that specifies the rendering hints that are set for
    this painter.

    \sa setRenderHint()
*/

QCPainter::RenderHints QCPainter::renderHints() const
{
    Q_D(const QCPainter);
    return d->m_e->renderHints();
}

// ***** Static methods *****

/*!
    Static helper method to convert millimeters \a mm into pixels.
    This allows doing resolution independent drawing. For example to set
    the line width to 2mm use:

    painter->setLineWidth(QCPainter::mmToPx(2));
*/

float QCPainter::mmToPx(float mm)
{
    float ldp = 72.0f;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        ldp = float(screen->physicalDotsPerInch());
    } else {
        qWarning() << "QScreen required for mmToPx";
    }
    return ldp * mm / 25.4f;
}

/*!
    Static helper method to convert points \a pt into pixels.
*/

float QCPainter::ptToPx(float pt)
{
    float ldp = 72.0f;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        ldp = float(screen->physicalDotsPerInch());
    } else {
        qWarning() << "QScreen required for ptToPx";
    }
    return pt * (ldp / 72.0f);
}

/*!
    Adds \a image with \a flags available for the painter as a texture.
    Returns QCImage with the texture id and other information about the image.
    Returned QCImage can then be used with \l drawImage and \l QCImagePattern.
    After calling this method, \a image QImage does not need be kept in memory.

    Calling with the same \a image is a cheap operation, since a cache hit is
    expected.

    Care must be taken when optimizing to call addImage() only once. That is not
    always sufficient, depending on the application design. For example, if the
    underlying graphics resources are lost, e.g. because the painter is
    associated with a new QRhi under the hood due to moving a widget to a new
    top-level, then calling this function is essential in order to re-create the
    native graphics textures from \a image.

    \sa removeImage
*/

QCImage QCPainter::addImage(const QImage &image, QCPainter::ImageFlags flags)
{
    Q_D(QCPainter);
    return d->getQCImage(image, flags);
}

/*!
    \overload

    Adds \a texture with \a flags available for the painter as a texture. The
    flag NativeTexture is set implicitly. The returned QCImage can be used with
    \l drawImage and \l QCImagePattern.

    \note The ownership of \a texture is \b not taken.

    \sa removeImage
*/

QCImage QCPainter::addImage(QRhiTexture *texture, QCPainter::ImageFlags flags)
{
    Q_D(QCPainter);
    return d->getQCImage(texture, flags);
}

/*!
    \overload

    Registers \a canvas with \a flags to the painter so that it is available as an image.
    The returned QCImage can be used with \l drawImage and \l QCImagePattern.

    \note \a canvas continues to manage the underlying native graphics
    resources, meaning removeImage() does not render \a canvas invalid.

    \sa removeImage
*/

QCImage QCPainter::addImage(const QCOffscreenCanvas &canvas, QCPainter::ImageFlags flags)
{
    Q_D(QCPainter);
    return d->getQCImage(canvas, flags);
}

/*!
    Removes image with \a imageId from the painter.
    \note This does not need to be normally called as images are removed
    in the painter destructor. Only use this to reduce memory
    usage when \a imageId is not needed anymore.
    \note Removed images can not be used in paint operations anymore.
    \sa addImage
*/

void QCPainter::removeImage(int imageId)
{
    Q_D(QCPainter);
    d->m_dataCache.removeTextureId(imageId);
}

/*!
    Removes all temporary textures from the cache.
*/

void QCPainter::cleanupResources()
{
    Q_D(QCPainter);
    d->m_dataCache.removeTemporaryResources();
}

/*!
    Returns the memory usage of texture images in kilobytes.
*/

qsizetype QCPainter::cacheMemoryUsage() const
{
    Q_D(const QCPainter);
    return d->m_dataCache.dataAmount() * 0.001;
}

/*!
    Returns the amount of images in the cache.
*/

qsizetype QCPainter::cacheTextureAmount() const
{
    Q_D(const QCPainter);
    return d->m_dataCache.size();
}

/*!
    Removes \a pathGroup from the painter cache. Calling fill() or stroke()
    for \a pathGroup after this, will regenerate the path into the group cache.

    \note This does not need to be normally called as paths are removed
    in the painter destructor. Only use this to reduce memory
    usage when \a pathGroup is not needed anymore or e.g. when the path
    has a lot less commands that it has had in the past and buffer size
    should be reduced.
    \sa fill(), stroke()
*/

void QCPainter::removePathGroup(int pathGroup)
{
    Q_D(QCPainter);
    d->engine()->removePathGroup(pathGroup);
}

// ***** Private *****

// Marks all unused temporary textures to be removed.
void QCDataCache::removeTemporaryResources()
{
    m_doingResourcesRemoval = true;
}

// Marks a texture with imageId to be removed
void QCDataCache::removeTextureId(int imageId)
{
    for (auto i = m_data.cbegin(), end = m_data.cend(); i != end; ++i) {
        if (i.value().id() == imageId) {
            m_cleanupTextures << i.value();
            break;
        }
    }
}

// Handles texture removal from GPU and from the cache
void QCDataCache::handleRemoveTextures()
{
    Q_ASSERT(m_painterPrivate);
    if (m_cleanupTextures.isEmpty() && !m_doingResourcesRemoval) {
        // Nothing to be done, reset the used textures list.
        m_usedTextureIDs.clear();
        return;
    }
    auto it = m_data.begin();
    while (it != m_data.end()) {
        QCImagePrivate *ip = (*it).d.get();
        bool remove = m_cleanupTextures.contains(*it);
        bool removeUnusedGradients = m_doingResourcesRemoval &&
                                     ip->type == QCImagePrivate::DataType::GradientTexture &&
                                     !m_usedTextureIDs.contains(ip->id);
        if (remove || removeUnusedGradients) {
            m_painterPrivate->m_e->deleteImage(ip->id);
            m_dataAmount -= ip->size;
            it = m_data.erase(it);
        } else {
            it++;
        }
    }
    // Cleanups done, so reset
    m_cleanupTextures.clear();
    m_usedTextureIDs.clear();
    m_doingResourcesRemoval = false;
}

void QCDataCache::clear()
{
    m_data.clear();
    m_cleanupTextures.clear();
    m_usedTextureIDs.clear();
    m_dataAmount = 0;
    m_doingResourcesRemoval = false;
}

// Marks imageId as being currently used
void QCDataCache::markTextureIdUsed(int imageId) {
    if (imageId > 0 && !m_usedTextureIDs.contains(imageId))
        m_usedTextureIDs << imageId;
}


QCPainterPrivate::QCPainterPrivate()
{
    m_dataCache.m_painterPrivate = this;
    m_e = new QCPainterEngine();
}

QCPainterPrivate::~QCPainterPrivate()
{
    // Cleanup possible pending textures
    handleCleanupTextures();

    delete m_e;
}

QCPainterEngine *QCPainterPrivate::engine() const
{
    return m_e;
}


static QRectF textAlignedRectFromPoint(QCPainter::TextAlign textAlignment, float x, float y, float maxWidth)
{
    QRectF rect;
    if (textAlignment == QCPainter::TextAlign::Left) {
        rect.setRect(x, y, maxWidth, 0);
    } else if (textAlignment == QCPainter::TextAlign::Center) {
        const float hWidth = maxWidth * 0.5f;
        rect.setRect(x - hWidth, y, maxWidth, 0);
    } else {
        rect.setRect(x - maxWidth, y, maxWidth, 0);
    }
    return rect;
}

void QCPainterPrivate::updateBackendName(QRhi *rhi) {
    if (!rhi)
        return;
    m_rhiBackendName = QString::fromUtf8(rhi->backendName());
}

// Delete the textures marked to be cleaned.
void QCPainterPrivate::handleCleanupTextures()
{
    if (m_renderer && m_renderer->ctx)
        m_dataCache.handleRemoveTextures();
}

// Removes all data from m_dataCache, does not destroy the actual textures. To
// be called when the renderer is going away (and so all rhi textures are going
// to be deleted by it).
void QCPainterPrivate::clearTextureCache()
{
    m_dataCache.clear();

    m_nativeTextureCache.clear(); // these we do not own anyway
}

// Marks that this imageId was used during the paint operation.
void QCPainterPrivate::markTextureIdUsed(int imageId) {
#ifdef QCPAINTER_TRACK_TEXTURE_USAGE
    m_dataCache.markTextureIdUsed(imageId);
#else
    Q_UNUSED(imageId);
#endif
}

qint64 QCPainterPrivate::generateImageKey(const QImage &image, QCPainter::ImageFlags flags) const
{
    return image.cacheKey() ^ int(flags);
}

// Get the texture id for the image. Id will be unique for the image+flags, or imageKey
// in case that is provided.
QCImage QCPainterPrivate::getQCImage(const QImage &image, QCPainter::ImageFlags flags, qint64 imageKey)
{
    Q_ASSERT(m_renderer);
    QCImage qcimage;
    qint64 key = imageKey;
    QCImagePrivate::DataType type = QCImagePrivate::DataType::GradientTexture;
    if (imageKey == 0) {
        key = generateImageKey(image, flags);
        // Currently when this method is called without imageKey it means that
        // the image is user defined (and not internal gradient texture).
        type = QCImagePrivate::DataType::UserTexture;
    }
    if (m_dataCache.contains(key)) {
        // Image is in cache
        qcimage = m_dataCache.image(key);
    } else {
        if (image.isNull()) {
            qWarning() << "Empty image";
        } else {
            QImage convertedImage;
            if (image.format() != QImage::Format_RGBA8888 && image.format() != QImage::Format_RGBA8888_Premultiplied) {
                qCDebug(QC_INFO) << "Converting image" << key << "from" << image.format() << "to" << QImage::Format_RGBA8888_Premultiplied;
                convertedImage = image.convertedTo(QImage::Format_RGBA8888_Premultiplied);
            } else {
                convertedImage = image;
            }
            if (convertedImage.format() == QImage::Format_RGBA8888_Premultiplied)
                flags |= QCPainter::ImageFlag::Premultiplied;
            else
                flags &= ~int(QCPainter::ImageFlag::Premultiplied);
            QCImagePrivate *ip = qcimage.d.get();
            ip->id = m_e->createImage(image.width(), image.height(), flags, convertedImage.constBits());
            ip->width = convertedImage.width();
            ip->height = convertedImage.height();
            ip->size = convertedImage.sizeInBytes();
            ip->type = type;
            m_dataCache.insert(key, qcimage);
        }
    }
    return qcimage;
}

QCImage QCPainterPrivate::getQCImage(QRhiTexture *texture, QCPainter::ImageFlags flags)
{
    Q_ASSERT(m_renderer);
    QCImage qcimage;
    const quint64 key = texture->globalResourceId();
    if (m_nativeTextureCache.contains(key)) {
        qcimage = m_nativeTextureCache.value(key);
        QCImagePrivate *ip = qcimage.d.get();
        if (ip->width != texture->pixelSize().width() || ip->height != texture->pixelSize().height())
            qcimage = {};
    }
    if (qcimage.isNull()) {
        QCImagePrivate *ip = qcimage.d.get();
        ip->id = m_renderer->renderCreateNativeTexture(texture, flags)->id;
        ip->width = texture->pixelSize().width();
        ip->height = texture->pixelSize().height();
        quint32 byteSize = 0;
        QCPainterRhiRenderer::textureFormatInfo(texture->format(), texture->pixelSize(), nullptr, &byteSize, nullptr);
        ip->size = byteSize;
        ip->type = QCImagePrivate::DataType::UserTexture;
        m_nativeTextureCache.insert(key, qcimage);
    }
    return qcimage;
}

QCImage QCPainterPrivate::getQCImage(const QCOffscreenCanvas &canvas, QCPainter::ImageFlags flags)
{
    if (canvas.isNull())
        return {};

    if (m_renderer->isOffscreenCanvasYUp())
        flags.setFlag(QCPainter::ImageFlag::FlipY, !flags.testFlag(QCPainter::ImageFlag::FlipY));

    return getQCImage(canvas.texture(), flags);
}

void QCPainterPrivate::setFont(const QFont &font)
{
    m_e->state.font = font;
}

void QCPainterPrivate::prepareText(const QCText &text)
{
    m_e->prepareText(const_cast<QCText &>(text));
}

void QCPainterPrivate::fillText(const QString &text, float x, float y, float maxWidth, int cacheIndex)
{
    // Unify point & rect APIs behavior.
    const QRectF rect = textAlignedRectFromPoint(m_e->state.textAlignment, x, y, maxWidth);
    fillText(text, rect, cacheIndex);
}

void QCPainterPrivate::fillText(const QString &text, const QRectF &rect, int cacheIndex)
{
    m_e->fillText(text, rect, cacheIndex);
}

void QCPainterPrivate::fillText(const QCText &text)
{
    m_e->fillText(const_cast<QCText &>(text));
}

QRectF QCPainterPrivate::textBoundingBox(const QCText &text)
{
    return m_e->textBoundingBox(const_cast<QCText &>(text));
}

QRectF QCPainterPrivate::textBoundingBox(const QString &text, float x, float y, float maxWidth)
{
    // Unify point & rect APIs behavior.
    const QRectF rect = textAlignedRectFromPoint(m_e->state.textAlignment, x, y, maxWidth);
    return m_e->textBoundingBox(text, rect);
}

QRectF QCPainterPrivate::textBoundingBox(const QString &text, const QRectF &rect)
{
    return m_e->textBoundingBox(text, rect);
}

/*!
    \return a new offscreen canvas with the given \a pixelSize, \a sampleCount,
    and \a flags.

    The size of the canvas is specified in pixels. The \a pixelSize, \a
    sampleCount, and \a flags properties are immutable afterwards. To get a
    canvas with a different size, sample count, or flags, create a new one.

    To target an offscreen canvas with with draw commands, call the appropriate
    \l QCRhiPaintDriver::beginPaint() overload when working with the lower level
    API, or \l QCPainterWidget::beginCanvasPainting() or \l
    QQuickCPainterRenderer::beginCanvasPainting() when using the convenience
    widget or Qt Quick item classes.

    Normally the contents of the canvas is cleared when painting to it. To
    disable this, pass \l{Flag::}{PreserveContents} in \a flags.

    To request multisample rendering onto the canvas (multisample antialiasing,
    MSAA), set a sample count larger than 1, such as 4 or 8. Preserving the
    canvas contents between render passes is not supported however when
    multisampling is enabled, and the \l{Flag::}{PreserveContents} flag will not
    work in this case.
 */
QCOffscreenCanvas QCPainter::createCanvas(QSize pixelSize, int sampleCount, QCOffscreenCanvas::Flags flags)
{
    Q_D(QCPainter);
    if (!d->m_renderer || !d->m_renderer->ctx) {
        qWarning("createCanvas() cannot be called without renderer");
        return {};
    }
    return d->m_renderer->createCanvas(pixelSize, sampleCount, flags);
}

/*!
    Destroys the resources backing \a canvas. \a canvas becomes a
    \l{QCOffscreenCanvas::isNull()}{null canvas} then.

    The painter automatically does this upon its destruction. Therefore, calling
    this function is only necessary when releasing the associated resources is
    desired right away.
*/
void QCPainter::destroyCanvas(QCOffscreenCanvas &canvas)
{
    Q_D(QCPainter);
    if (!d->m_renderer || !d->m_renderer->ctx) {
        qWarning("destroyCanvas() cannot be called without renderer");
        return;
    }
    d->m_renderer->destroyCanvas(canvas);
}

QT_END_NAMESPACE
