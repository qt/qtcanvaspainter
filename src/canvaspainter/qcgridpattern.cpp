// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcgridpattern.h"
#include "qcgridpattern_p.h"
#include "qcpainter_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCGridPattern
    \brief QCGridPattern is a brush for painting grid patterns.
    \inmodule QtCanvasPainter

    QCGridPattern is a brush for painting grid and bar patterns.

    Here is a simple example:
    \code
    // Rotated grid into background
    QRectF rect(20, 20, 180, 180);
    QCGridPattern gp1(rect, "#DBEB00", "#373F26");
    gp1.setCellSize(16, 16);
    gp1.setStartPosition(rect.topLeft());
    gp1.setLineWidth(2.0f);
    gp1.setRotation(M_PI / 4);
    gp1.setFeather(5.0f);
    painter.setFillStyle(gp1);
    painter.fillRect(rect);
    // Rounded rectangle, stroked with
    // grid pattern for dashes.
    float strokeW = 10;
    QRectF rect2(40, 40, 140, 140);
    QCGridPattern gp2;
    gp2.setLineColor(Qt::transparent);
    gp2.setBackgroundColor(Qt::white);
    gp2.setStartPosition(rect2.x() - strokeW,
                         rect2.y() - strokeW);
    gp2.setCellSize(40, 40);
    gp2.setLineWidth(strokeW);
    painter.setLineWidth(8);
    painter.setStrokeStyle(gp2);
    painter.beginPath();
    painter.roundRect(rect2, 10);
    painter.stroke();
    \endcode

    \image gridpattern_example_1.png

*/

/*!
    Constructs a default grid pattern.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0, grid line color white and background color black.
*/

QCGridPattern::QCGridPattern()
    : d(new QCGridPatternPrivate)
{
}

/*!
    Constructs an grid pattern.
    Pattern start position position and size is defined with \a rect.
    Grid line color is \a lineColor and background color is \a backgroundColor.
    Grid line width is \a lineWidth, feather (antialiasing) is \a feather and
    angle is \a angle.
*/

QCGridPattern::QCGridPattern(const QRectF &rect,
                             const QColor &lineColor,
                             const QColor &backgroundColor,
                             float lineWidth, float feather, float angle)
    : d(new QCGridPatternPrivate)
{
    d->x = float(rect.x());
    d->y = float(rect.y());
    d->width = float(rect.width());
    d->height = float(rect.height());
    d->lineColor = lineColor;
    d->backgroundColor = backgroundColor;
    d->lineWidth = lineWidth;
    d->feather = feather;
    d->angle = angle;
}

/*!
    Constructs an grid pattern.
    Pattern start position is ( \a x, \a y) and pattern size ( \a width, \a height).
    Grid line color is \a lineColor and background color is \a backgroundColor.
    Pattern angle is \a angle.
*/

QCGridPattern::QCGridPattern(float x, float y, float width, float height,
                             const QColor &lineColor,
                             const QColor &backgroundColor,
                             float lineWidth, float feather, float angle)
    : d(new QCGridPatternPrivate)
{
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->lineColor = lineColor;
    d->backgroundColor = backgroundColor;
    d->lineWidth = lineWidth;
    d->feather = feather;
    d->angle = angle;
}

/*!
    Constructs an grid pattern that is a copy of the given \a pattern.
*/

QCGridPattern::QCGridPattern(const QCGridPattern &pattern) noexcept
    : d(pattern.d)
{
}

/*!
    Destroys the grid pattern.
*/

QCGridPattern::~QCGridPattern() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCGridPatternPrivate)

/*!
    Assigns the given grid \a pattern to this pattern and returns a reference to
    this grid pattern.
*/

QCGridPattern &QCGridPattern::operator=(const QCGridPattern &pattern) noexcept
{
    QCGridPattern(pattern).swap(*this);
    return *this;
}

/*!
    \fn QCGridPattern::QCGridPattern(QCGridPattern &&other) noexcept

    Move-constructs a new QCGridPattern from \a other.
*/

/*!
    \fn QCGridPattern &QCGridPattern::operator=(QCGridPattern &&other)

    Move-assigns \a other to this QCGridPattern instance.
*/

/*!
    \fn void QCGridPattern::swap(QCGridPattern &other)
    \memberswap{pattern}
*/

/*!
   Returns the grid pattern as a QVariant.
*/

QCGridPattern::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCGridPattern::operator!=(const QCGridPattern &pattern) const

    Returns \c true if the grid pattern is different from the given \a pattern;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCGridPattern::operator==(const QCGridPattern &pattern) const

    Returns \c true if the grid pattern is equal to the given \a pattern; otherwise
    false.

    \sa operator!=()
*/

bool QCGridPattern::operator==(const QCGridPattern &p) const
{
    if (p.d == d)
        return true;
    if (d->x != p.d->x
        || d->y != p.d->y
        || d->width != p.d->width
        || d->height != p.d->height
        || d->feather != p.d->feather
        || d->angle != p.d->angle
        || d->lineWidth != p.d->lineWidth
        || d->lineColor != p.d->lineColor
        || d->backgroundColor != p.d->backgroundColor)
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCGridPattern &p)
{
    QDebugStateSaver saver(dbg);
    const auto &sp = p.startPosition();
    dbg.nospace() << "QCGridPattern(" << sp << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCGridPattern stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCGridPattern &pattern)
    \relates QCGridPattern

    Writes the given \a pattern to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCGridPattern &p)
{
    const auto sp = p.startPosition();
    const auto size = p.cellSize();
    s << sp.x() << sp.y();
    s << size.width() << size.height();
    s << p.lineWidth() << p.feather() << p.rotation();
    s << p.lineColor() << p.backgroundColor();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCGridPattern &pattern)
    \relates QCGridPattern

    Reads the given \a pattern from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCGridPattern &p)
{
    float x, y, width, height, lineWidth, feather, angle;
    QColor lineColor, backgroundColor;
    s >> x >> y >> width >> height;
    s >> lineWidth >> feather >> angle;
    s >> lineColor >> backgroundColor;
    p.setStartPosition(x, y);
    p.setCellSize(width, height);
    p.setLineWidth(lineWidth);
    p.setFeather(feather);
    p.setRotation(angle);
    p.setLineColor(lineColor);
    p.setBackgroundColor(backgroundColor);
    return s;
}

#endif // QT_NO_DATASTREAM

/*!
    Returns the type of brush, \c QCBrush::BrushType::GridPattern.
*/

QCBrush::BrushType QCGridPattern::type() const
{
    return QCBrush::BrushType::GridPattern;
}

/*!
    Returns the start point of grid pattern.
    \sa setStartPosition()
*/

QPointF QCGridPattern::startPosition() const
{
    return QPointF(d->x, d->y);
}

/*!
    Sets the start point of grid pattern to (\a x, \a y).
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/

void QCGridPattern::setStartPosition(float x, float y)
{
    detach();
    d->x = x;
    d->y = y;
    d->changed = true;
}

/*!
    \overload
    Sets the start point of grid pattern to \a point.
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/

void QCGridPattern::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()),
                     float(point.y()));
}

/*!
    Returns the size of a single cell in grid pattern.
    \sa setCellSize()
*/

QSizeF QCGridPattern::cellSize() const
{
    return QSizeF(d->width, d->height);
}

/*!
    Sets the size of a single cell in grid pattern to \a width, \a height.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

void QCGridPattern::setCellSize(float width, float height)
{
    detach();
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    \overload
    Sets the size of a single cell in grid pattern to \a size.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

void QCGridPattern::setCellSize(QSizeF size)
{
    setCellSize(float(size.width()), float(size.height()));
}

/*!
    Returns the width of a stroke line in grid pattern.
    \sa setLineWidth()
*/

float QCGridPattern::lineWidth() const
{
    return d->lineWidth;
}

/*!
    Sets the width of a stroke line in grid pattern to \a width.
    The default value is \c 1.0.
*/

void QCGridPattern::setLineWidth(float width)
{
    detach();
    d->lineWidth = width;
    d->changed = true;
}

/*!
    Returns the pattern feather in pixels.
*/

float QCGridPattern::feather() const
{
    return d->feather;
}

/*!
    Sets the pattern feather to \a feather in pixels.
    The default value is \c 1.0, meaning a single pixel antialiasing.
*/

void QCGridPattern::setFeather(float feather)
{
    detach();
    d->feather = feather;
    d->changed = true;
}

/*!
    Returns the pattern rotation in radians.
*/

float QCGridPattern::rotation() const
{
    return d->angle;
}

/*!
    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the grid startPosition().
    The default value is \c 0.0, meaning the grid is not rotated.
*/

void QCGridPattern::setRotation(float rotation)
{
    detach();
    d->angle = rotation;
    d->changed = true;
}

/*!
    Returns the pattern grid line color.
*/

QColor QCGridPattern::lineColor() const
{
    return d->lineColor;
}

/*!
    Sets the pattern grid line color to \a color.
    The default value is white.
*/

void QCGridPattern::setLineColor(const QColor &color)
{
    detach();
    d->lineColor = color;
    d->changed = true;
}

/*!
    Returns the pattern grid background color.
*/

QColor QCGridPattern::backgroundColor() const
{
    return d->backgroundColor;
}

/*!
    Sets the pattern grid background color to \a color.
    The default value is black.
*/

void QCGridPattern::setBackgroundColor(const QColor &color)
{
    detach();
    d->backgroundColor = color;
    d->changed = true;
}

/*!
    \internal
*/

void QCGridPattern::detach()
{
    if (d)
        d.detach();
    else
        d = new QCGridPatternPrivate;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCGridPattern::createPaint(QCPainter *painter) const
{
    auto *painterPriv = QCPainterPrivate::get(painter);
    if (d->changed) {
        auto *e = painterPriv->engine();
        d->paint = e->createGridPattern(d->x, d->y, d->width, d->height,
                                        d->lineWidth, d->angle, d->feather,
                                        d->lineColor, d->backgroundColor);
        d->changed = false;
    }
    return d->paint;
}

QT_END_NAMESPACE
