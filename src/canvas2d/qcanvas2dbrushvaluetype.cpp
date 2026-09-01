// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dbrushvaluetype_p.h"
#include "qcanvas2dutils_p.h"

QT_BEGIN_NAMESPACE

// ***** Linear Gradient *****

/*!
    \qmlvaluetype lineargradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides lineargradient2d type matching to QCanvasLinearGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasLinearGradient
*/


/*!
    \qmlmethod void lineargradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createLinearGradient(0, 0, 100, 100);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DLinearGradientValueType::addColorStop(float offset, QColor color)
{
    QCanvasLinearGradient::addColorStop(offset, color);
}

/*!
    \qmlmethod point lineargradient2d::startPosition()

    Returns the start point of linear gradient.
*/

QPointF QCanvas2DLinearGradientValueType::startPosition() const
{
    return QCanvasLinearGradient::startPosition();
}

/*!
    \qmlmethod void lineargradient2d::setStartPosition(point start)
    \overload

    Sets the start point of linear gradient to \a start.
*/

/*!
    \qmlmethod void lineargradient2d::setStartPosition(real x, real y)

    Sets the start point of linear gradient to ( \a x, \a y).
*/

void QCanvas2DLinearGradientValueType::setStartPosition(float x, float y)
{
    QCanvasLinearGradient::setStartPosition(x, y);
}

/*!
    \qmlmethod point lineargradient2d::endPosition()

    Returns the end point of linear gradient.
*/

QPointF QCanvas2DLinearGradientValueType::endPosition() const
{
    return QCanvasLinearGradient::endPosition();
}

/*!
    \qmlmethod void lineargradient2d::setEndPosition(point end)
    \overload

    Sets the end point of linear gradient to \a end.
*/

/*!
    \qmlmethod void lineargradient2d::setEndPosition(real x, real y)

    Sets the end point of linear gradient to ( \a x, \a y).
*/

void QCanvas2DLinearGradientValueType::setEndPosition(float x, float y)
{
    QCanvasLinearGradient::setEndPosition(x, y);
}

// ***** Radial Gradient *****

/*!
    \qmlvaluetype radialgradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides radialgradient2d type matching to QCanvasRadialGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasRadialGradient
*/


/*!
    \qmlmethod void radialgradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createRadialGradient(100, 1000, 10, 100, 100, 80);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DRadialGradientValueType::addColorStop(float offset, QColor color)
{
    QCanvasRadialGradient::addColorStop(offset, color);
}

/*!
    \qmlmethod point radialgradient2d::centerPosition()

    Returns the center point of radial gradient.
    This is the same as \l outerCenterPosition().

    \sa setCenterPosition()
*/

QPointF QCanvas2DRadialGradientValueType::centerPosition() const
{
    return QCanvasRadialGradient::centerPosition();
}

/*!
    \qmlmethod void radialgradient2d::setCenterPosition(point center)
    \overload

    Sets the both center points of radial gradient to \a center.
    So after calling this, the gradient is symmetric (inner and outer
    positions are the same).
*/

/*!
    \qmlmethod void radialgradient2d::setCenterPosition(real x, real y)

    Sets the both center points of radial gradient to ( \a x, \a y).
    So after calling this, the gradient is symmetric (inner and outer
    positions are the same).

    \sa centerPosition()
*/

void QCanvas2DRadialGradientValueType::setCenterPosition(float x, float y)
{
    return QCanvasRadialGradient::setCenterPosition(x, y);
}

/*!
    \qmlmethod point radialgradient2d::innerCenterPosition()

    Returns the inner center point of radial gradient.

    \sa setInnerCenterPosition()
*/

QPointF QCanvas2DRadialGradientValueType::innerCenterPosition() const
{
    return QCanvasRadialGradient::innerCenterPosition();
}

/*!
    \qmlmethod void radialgradient2d::setInnerCenterPosition(point center)
    \overload

    Sets the inner center point of radial gradient to \a center.
*/

/*!
    \qmlmethod void radialgradient2d::setInnerCenterPosition(real x, real y)

    Sets the inner center point of radial gradient to ( \a x, \a y).

    \sa innerCenterPosition()
*/

void QCanvas2DRadialGradientValueType::setInnerCenterPosition(float x, float y)
{
    return QCanvasRadialGradient::setInnerCenterPosition(x, y);
}

/*!
    \qmlmethod point radialgradient2d::outerCenterPosition()

    Returns the outer center point of radial gradient.

    \sa setOuterCenterPosition()
*/

QPointF QCanvas2DRadialGradientValueType::outerCenterPosition() const
{
    return QCanvasRadialGradient::outerCenterPosition();
}

/*!
    \qmlmethod void radialgradient2d::setOuterCenterPosition(point center)
    \overload

    Sets the outer center point of radial gradient to \a center.
*/

/*!
    \qmlmethod void radialgradient2d::setOuterCenterPosition(real x, real y)

    Sets the outer center point of radial gradient to ( \a x, \a y).

    \sa outerCenterPosition()
*/

void QCanvas2DRadialGradientValueType::setOuterCenterPosition(float x, float y)
{
    return QCanvasRadialGradient::setOuterCenterPosition(x, y);
}

/*!
    \qmlmethod real radialgradient2d::outerRadius()

    Returns the outer radius of radial gradient.

    \sa setOuterRadius()
*/

float QCanvas2DRadialGradientValueType::outerRadius() const
{
    return QCanvasRadialGradient::outerRadius();
}

/*!
    \qmlmethod void radialgradient2d::setOuterRadius(real radius)

    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from outer center position.
*/

void QCanvas2DRadialGradientValueType::setOuterRadius(float radius)
{
    return QCanvasRadialGradient::setOuterRadius(radius);
}

/*!
    \qmlmethod real radialgradient2d::innerRadius()

    Returns the inner radius of radial gradient.
    \sa setInnerRadius()
*/

float QCanvas2DRadialGradientValueType::innerRadius() const
{
    return QCanvasRadialGradient::innerRadius();
}

/*!
    \qmlmethod void radialgradient2d::setInnerRadius(real radius)

    Sets the inner radius of radial gradient to \a radius.
    Start color will be drawn at this radius from inner center position.
    The default inner radius is \c 0.0 meaning that gradient starts
    directly from inner center position.
*/

void QCanvas2DRadialGradientValueType::setInnerRadius(float radius)
{
    return QCanvasRadialGradient::setInnerRadius(radius);
}

// ***** Conical Gradient *****

/*!
    \qmlvaluetype conicalgradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides conicalgradient2d type matching to QCanvasConicalGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasConicalGradient
*/


/*!
    \qmlmethod void conicalgradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createConicalGradient(100, 100, Math.PI / 2);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DConicalGradientValueType::addColorStop(float offset, QColor color)
{
    QCanvasConicalGradient::addColorStop(offset, color);
}

/*!
    \qmlmethod point conicalgradient2d::centerPosition()

    Returns the center point of conical gradient.
    \sa setCenterPosition()
*/

QPointF QCanvas2DConicalGradientValueType::centerPosition() const
{
    return QCanvasConicalGradient::centerPosition();
}

/*!
    \qmlmethod void conicalgradient2d::setCenterPosition(point center)
    \overload

    Sets the center point of conical gradient to \a center.
*/

/*!
    \qmlmethod void conicalgradient2d::setCenterPosition(real x, real y)

    Sets the center point of conical gradient to ( \a x, \a y).
*/

void QCanvas2DConicalGradientValueType::setCenterPosition(float x, float y)
{
    QCanvasConicalGradient::setCenterPosition(x, y);
}

/*!
    \qmlmethod real conicalgradient2d::startAngle()

    Returns the start angle of conical gradient in radians.
*/

float QCanvas2DConicalGradientValueType::startAngle() const
{
    return QCanvasConicalGradient::startAngle();
}

/*!
    \qmlmethod void conicalgradient2d::setStartAngle(real angle)

    Sets the start angle of conical gradient to \a angle in radians.
    The angle starts from a line going horizontally right from the
    center, and proceeds clockwise.
*/

void QCanvas2DConicalGradientValueType::setStartAngle(float angle)
{
    QCanvasConicalGradient::setStartAngle(angle);
}

// ***** Box Gradient *****

/*!
    \qmlvaluetype boxgradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides boxgradient2d type matching to QCanvasBoxGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasBoxGradient
*/


/*!
    \qmlmethod void boxgradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createBoxGradient(50, 50, 100, 100, 20, 10);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DBoxGradientValueType::addColorStop(float offset, QColor color)
{
    QCanvasBoxGradient::addColorStop(offset, color);
}

/*!
    \qmlmethod rect boxgradient2d::rect()

    Returns the rectangle area of the box gradient.
    \sa setRect()
*/

QRectF QCanvas2DBoxGradientValueType::rect() const
{
    return QCanvasBoxGradient::rect();
}

/*!
    \qmlmethod void boxgradient2d::setRect(rect)
    \overload

    Sets the rectangle of box gradient to \a rect.
*/

/*!
    \qmlmethod void boxgradient2d::setRect(x, y, width, height)

    Sets the rectangle of box gradient to position ( \a x, \a y) and size ( \a width, \a height).
*/

void QCanvas2DBoxGradientValueType::setRect(float x, float y, float width, float height)
{
    QCanvasBoxGradient::setRect(x, y, width, height);
}

/*!
    \qmlmethod real boxgradient2d::feather()

    Returns the feather of the box gradient.
    \sa setFeather()
*/

float QCanvas2DBoxGradientValueType::feather() const
{
    return QCanvasBoxGradient::feather();
}

/*!
    \qmlmethod void boxgradient2d::setFeather(real feather)

    Sets the feather of box gradient to \a feather.
*/

void QCanvas2DBoxGradientValueType::setFeather(float feather)
{
    QCanvasBoxGradient::setFeather(feather);
}

/*!
    \qmlmethod real boxgradient2d::radius()

    Returns the radius of the box gradient.
    \sa setRadius()
*/

float QCanvas2DBoxGradientValueType::radius() const
{
    return QCanvasBoxGradient::radius();
}

/*!
    \qmlmethod void boxgradient2d::setRadius(real radius)

    Sets the radius of box gradient to \a radius.
    The maximum radius is half of width or height of rect(),
    depending on which one is smaller.
*/

void QCanvas2DBoxGradientValueType::setRadius(float radius)
{
    QCanvasBoxGradient::setRadius(radius);
}

// ***** Grid Pattern *****

/*!
    \qmlvaluetype gridpattern2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides gridpattern2d type matching to QCanvasGridPattern.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasGridPattern
*/

/*!
    \qmlmethod point gridpattern2d::startPosition()

    Returns the start point of grid pattern.
    \sa setStartPosition()
*/

QPointF QCanvas2DGridPatternValueType::startPosition() const
{
    return QCanvasGridPattern::startPosition();
}

/*!
    \qmlmethod void gridpattern2d::setStartPosition(point start)
    \overload

    Sets the start point of grid pattern to \a start.
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/

/*!
    \qmlmethod void gridpattern2d::setStartPosition(real x, real y)

    Sets the start point of grid pattern to (\a x, \a y).
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/

void QCanvas2DGridPatternValueType::setStartPosition(float x, float y)
{
    QCanvasGridPattern::setStartPosition(x, y);
}

/*!
    \qmlmethod size gridpattern2d::cellSize()

    Returns the size of a single cell in grid pattern.
    \sa setCellSize()
*/

QSizeF QCanvas2DGridPatternValueType::cellSize() const
{
    return QCanvasGridPattern::cellSize();
}

/*!
    \qmlmethod void gridpattern2d::setCellSize(size cellSize)
    \overload

    Sets the size of a single cell in grid pattern to \a cellSize.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

/*!
    \qmlmethod void gridpattern2d::setCellSize(real width, real height)

    Sets the size of a single cell in grid pattern to \a width, \a height.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

void QCanvas2DGridPatternValueType::setCellSize(float width, float height)
{
    QCanvasGridPattern::setCellSize(width, height);
}

/*!
    \qmlmethod real gridpattern2d::lineWidth()

    Returns the width of a stroke line in grid pattern.
    \sa setLineWidth()
*/

float QCanvas2DGridPatternValueType::lineWidth() const
{
    return QCanvasGridPattern::lineWidth();
}

/*!
    \qmlmethod void gridpattern2d::setLineWidth(real width)

    Sets the width of a stroke line in grid pattern to \a width.
    The default value is \c 1.0.
*/

void QCanvas2DGridPatternValueType::setLineWidth(float width)
{
    QCanvasGridPattern::setLineWidth(width);
}

/*!
    \qmlmethod real gridpattern2d::feather()

    Returns the pattern feather in pixels.
*/

float QCanvas2DGridPatternValueType::feather() const
{
    return QCanvasGridPattern::feather();
}

/*!
    \qmlmethod void gridpattern2d::setFeather(real feather)

    Sets the pattern feather to \a feather in pixels.
    The default value is \c 1.0, meaning a single pixel antialiasing.
*/

void QCanvas2DGridPatternValueType::setFeather(float feather)
{
    QCanvasGridPattern::setFeather(feather);
}

/*!
    \qmlmethod real gridpattern2d::rotation()

    Returns the pattern rotation in radians.
*/

float QCanvas2DGridPatternValueType::rotation() const
{
    return QCanvasGridPattern::rotation();
}

/*!
    \qmlmethod void gridpattern2d::setRotation(real rotation)

    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the grid startPosition().
    The default value is \c 0.0, meaning the grid is not rotated.
*/

void QCanvas2DGridPatternValueType::setRotation(float rotation)
{
    QCanvasGridPattern::setRotation(rotation);
}

/*!
    \qmlmethod color gridpattern2d::lineColor()

    Returns the pattern grid line color.
*/

QColor QCanvas2DGridPatternValueType::lineColor() const
{
    return QCanvasGridPattern::lineColor();
}

/*!
    \qmlmethod void gridpattern2d::setLineColor(color lineColor)

    Sets the pattern grid line color to \a lineColor.
    The default value is white.
*/

void QCanvas2DGridPatternValueType::setLineColor(QColor color)
{
    QCanvasGridPattern::setLineColor(color);
}

/*!
    \qmlmethod color gridpattern2d::backgroundColor()

    Returns the pattern grid background color.
*/

QColor QCanvas2DGridPatternValueType::backgroundColor() const
{
    return QCanvasGridPattern::backgroundColor();
}

/*!
    \qmlmethod void gridpattern2d::setBackgroundColor(color backgroundColor)

    Sets the pattern grid background color to \a backgroundColor.
    The default value is black.
*/

void QCanvas2DGridPatternValueType::setBackgroundColor(QColor color)
{
    QCanvasGridPattern::setBackgroundColor(color);
}

// ***** Box Shadow *****

/*!
    \qmlvaluetype boxshadow2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides boxshadow2d type matching to QCanvasBoxShadow.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasBoxShadow
*/

/*!
    \qmlmethod rect boxshadow2d::rect()

    Returns the rect area of shadow box.
    \sa setRect()
*/

QRectF QCanvas2DBoxShadowValueType::rect() const
{
    return QCanvasBoxShadow::rect();
}

/*!
    \qmlmethod void boxshadow2d::setRect(rect shadowRect)
    \overload

    Sets the rect area of shadow box to \a shadowRect.
*/

/*!
    \qmlmethod void boxshadow2d::setRect(real x, real y, real width, real height)

    Sets the rect area of shadow box to (\a x, \a y, \a width, \a height).
    \sa rect()
*/

void QCanvas2DBoxShadowValueType::setRect(float x, float y, float width, float height)
{
    QCanvasBoxShadow::setRect(x, y, width, height);
}

/*!
    \qmlmethod rect boxshadow2d::boundingRect()

    Returns the area covered by the shadow. This takes into
    account the shadow \l rect(), \l blur() and \l spread().
    If you don't want to use \l QCanvasPainter::drawBoxShadow() helper
    method, this can be used to for e.g. adding correctly sized
    \l QCanvasPainter::rect() into the path.

    \sa setRect(), setBlur(), setSpread()
*/

QRectF QCanvas2DBoxShadowValueType::boundingRect() const
{
    return QCanvasBoxShadow::boundingRect();
}

/*!
    \qmlmethod real boxshadow2d::radius()

    Returns the radius of shadow box.
    \sa setRadius()
*/

float QCanvas2DBoxShadowValueType::radius() const
{
    return QCanvasBoxShadow::radius();
}

/*!
    \qmlmethod void boxshadow2d::setRadius(real radius)

    Sets the shadow corner radius to \a radius in pixels.
    The default value is \c 0.0 meaning no radius.
*/

void QCanvas2DBoxShadowValueType::setRadius(float radius)
{
    QCanvasBoxShadow::setRadius(radius);
}

/*!
    \qmlmethod real boxshadow2d::blur()

    Returns the blur of shadow box.
    \sa setBlur()
*/

float QCanvas2DBoxShadowValueType::blur() const
{
    return QCanvasBoxShadow::blur();
}

/*!
    \qmlmethod void boxshadow2d::setBlur(real blur)

    Sets the shadow blur to \a blur in pixels.
    The default value is \c 0.0 meaning no blur.
*/

void QCanvas2DBoxShadowValueType::setBlur(float blur)
{
    QCanvasBoxShadow::setBlur(blur);
}

/*!
    \qmlmethod real boxshadow2d::spread()

    Returns the spread of shadow box.
    \sa setSpread()
*/

float QCanvas2DBoxShadowValueType::spread() const
{
    return QCanvasBoxShadow::spread();
}

/*!
    \qmlmethod void boxshadow2d::setSpread(real spread)

    Sets the shadow spread to \a spread in pixels.
    The default value is \c 0.0 meaning no spread.
*/

void QCanvas2DBoxShadowValueType::setSpread(float spread)
{
    QCanvasBoxShadow::setSpread(spread);
}

/*!
    \qmlmethod color boxshadow2d::color()

    Returns the color of shadow box.
    \sa setColor()
*/

QColor QCanvas2DBoxShadowValueType::color() const
{
    return QCanvasBoxShadow::color();
}

/*!
    \qmlmethod void boxshadow2d::setColor(color shadowColor)

    Sets the shadow color to \a shadowColor.
    The default value is black with full opacity.
*/

void QCanvas2DBoxShadowValueType::setColor(QColor color)
{
    QCanvasBoxShadow::setColor(color);
}

/*!
    \qmlmethod real boxshadow2d::topLeftRadius()

    Returns the top-left radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvas2DBoxShadowValueType::topLeftRadius() const
{
    return QCanvasBoxShadow::topLeftRadius();
}

/*!
    \qmlmethod void boxshadow2d::setTopLeftRadius(real radius)

    Sets the top-left corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvas2DBoxShadowValueType::setTopLeftRadius(float radius)
{
    QCanvasBoxShadow::setTopLeftRadius(radius);
}

/*!
    \qmlmethod real boxshadow2d::topRightRadius()

    Returns the top-right radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvas2DBoxShadowValueType::topRightRadius() const
{
    return QCanvasBoxShadow::topRightRadius();
}

/*!
    \qmlmethod void boxshadow2d::setTopRightRadius(real radius)

    Sets the top-right corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvas2DBoxShadowValueType::setTopRightRadius(float radius)
{
    QCanvasBoxShadow::setTopRightRadius(radius);
}

/*!
    \qmlmethod real boxshadow2d::bottomLeftRadius()

    Returns the bottom-left radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvas2DBoxShadowValueType::bottomLeftRadius() const
{
    return QCanvasBoxShadow::bottomLeftRadius();
}

/*!
    \qmlmethod void boxshadow2d::setBottomLeftRadius(real radius)

    Sets the bottom-left corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvas2DBoxShadowValueType::setBottomLeftRadius(float radius)
{
    QCanvasBoxShadow::setBottomLeftRadius(radius);
}

/*!
    \qmlmethod real boxshadow2d::bottomRightRadius()

    Returns the bottom-right radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvas2DBoxShadowValueType::bottomRightRadius() const
{
    return QCanvasBoxShadow::bottomRightRadius();
}

/*!
    \qmlmethod void boxshadow2d::setBottomRightRadius(real radius)

    Sets the bottom-right corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvas2DBoxShadowValueType::setBottomRightRadius(float radius)
{
    QCanvasBoxShadow::setBottomRightRadius(radius);
}

// ***** Image Pattern *****

/*!
    \qmlvaluetype imagepattern2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides imagepattern2d type matching to QCanvasImagePattern.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasImagePattern
*/

/*!
    \qmlmethod point imagepattern2d::startPosition()

    Returns the start point of image pattern.
    \sa setStartPosition()
*/

QPointF QCanvas2DImagePatternValueType::startPosition() const
{
    return QCanvasImagePattern::startPosition();
}

/*!
    \qmlmethod void imagepattern2d::setStartPosition(point start)
    \overload

    Sets the start point of image pattern to \a start.
    Start position means top-left corner of an image in pattern.
    Pattern will then be extended to all positions from here
    (if image \c Repeat flag has been set).
*/

/*!
    \qmlmethod void imagepattern2d::setStartPosition(real x, real y)

    Sets the start point of image pattern to (\a x, \a y).
    Start position means top-left corner of an image in pattern.
    Pattern will then be extended to all positions from here
    (if image \c Repeat flag has been set).
*/

void QCanvas2DImagePatternValueType::setStartPosition(float x, float y)
{
    QCanvasImagePattern::setStartPosition(x, y);
}

/*!
    \qmlmethod size imagepattern2d::imageSize()

    Returns the size of a single image in pattern.
    \sa setImageSize()
*/

QSizeF QCanvas2DImagePatternValueType::imageSize() const
{
    return QCanvasImagePattern::imageSize();
}

/*!
    \qmlmethod void imagepattern2d::setImageSize(size imageSize)
    \overload

    Sets the size of a single image in pattern to \a imageSize.
*/

/*!
    \qmlmethod void imagepattern2d::setImageSize(real width, real height)

    Sets the size of a single image in pattern to ( \a width, \a height).
*/

void QCanvas2DImagePatternValueType::setImageSize(float width, float height)
{
    QCanvasImagePattern::setImageSize(width, height);
}

/*!
    \qmlmethod real imagepattern2d::rotation()

    Returns the pattern rotation in radians.
*/

float QCanvas2DImagePatternValueType::rotation() const
{
    return QCanvasImagePattern::rotation();
}

/*!
    \qmlmethod void imagepattern2d::setRotation(real rotation)

    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the image startPosition().
    The default value is \c 0.0, meaning the image is not rotated.
*/

void QCanvas2DImagePatternValueType::setRotation(float rotation)
{
    QCanvasImagePattern::setRotation(rotation);
}

/*!
    \qmlmethod color imagepattern2d::tintColor()

    Returns the pattern tint color.
*/

QColor QCanvas2DImagePatternValueType::tintColor() const
{
    return QCanvasImagePattern::tintColor();
}

/*!
    \qmlmethod void imagepattern2d::setTintColor(color tintColor)

    Sets the pattern tint color to \a tintColor.
    The color of the pattern image will be multiplied with
    this tint color in the shader.
    The default value is white, meaning no tinting.
    \note To set alpha globally, use QCanvasPainter::setGlobalAlpha()
*/

void QCanvas2DImagePatternValueType::setTintColor(QColor color)
{
    QCanvasImagePattern::setTintColor(color);
}

QT_END_NAMESPACE
