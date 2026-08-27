// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasgridpattern.h"
#include "qcanvasgridpattern_p.h"
#include "qcanvaspainter_p.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasGridPattern
    \since 6.11
    \brief QCanvasGridPattern is a brush for painting grid patterns.
    \inmodule QtCanvasPainter

    QCanvasGridPattern is a brush for painting grid and bar patterns.

    Here is a simple example:
    \code
    // Rotated grid into background
    QRectF rect(20, 20, 180, 180);
    QCanvasGridPattern gp1(rect.topLeft(), QSizeF(16, 16));
    gp1.setLineColor("#DBEB00");
    gp1.setBackgroundColor("#373F26");
    gp1.setLineWidth(2.0);
    gp1.setRotation(M_PI / 4);
    gp1.setFeather(5.0);
    painter.setFillStyle(gp1);
    painter.fillRect(rect);
    // Rounded rectangle, stroked with
    // grid pattern for dashes.
    qreal strokeW = 10;
    QRectF rect2(40, 40, 140, 140);
    QCanvasGridPattern gp2;
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
           {Rounded square filled with a yellow diagonal grid and bordered by
           a dashed white stroke}

*/

/*!
    Constructs a default grid pattern.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0, grid line color white and background color black.
*/

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanvasGridPatternPrivate)

QCanvasGridPattern::QCanvasGridPattern(QCanvasGridPatternPrivate *p)
    : d(p)
{
}

void QCanvasGridPattern::detach()
{
    if (d)
        d.detach();
}

QCanvasGridPattern::QCanvasGridPattern()
    : d(new QCanvasGridPatternPrivate)
{
}


/*!
    \fn QCanvasGridPattern::QCanvasGridPattern(QPointF startPosition, QSizeF cellSize)

    Constructs a grid pattern starting at \a startPosition, with a single
    cell sized \a cellSize.

    \sa setStartPosition(), setCellSize()
*/

/*!
    Constructs a grid pattern starting at \a startX, \a startY, with a single
    cell sized \a cellWidth, \a cellHeight.

    \sa setLineColor(), setBackgroundColor(), setLineWidth(), setFeather(),
        setRotation()
*/

QCanvasGridPattern::QCanvasGridPattern(qreal startX, qreal startY,
                                       qreal cellWidth, qreal cellHeight)
    : d(new QCanvasGridPatternPrivate)
{
    d->x = startX;
    d->y = startY;
    d->width = cellWidth;
    d->height = cellHeight;
}

/*!
    Destroys the grid pattern.
*/

QCanvasGridPattern::QCanvasGridPattern(const QCanvasGridPattern &) = default;
QCanvasGridPattern &QCanvasGridPattern::operator=(const QCanvasGridPattern &) = default;
QCanvasGridPattern::~QCanvasGridPattern() = default;

QCanvasGridPattern::operator QCanvasBrush() const
{
    return QCanvasBrushPrivate::create(d.get());
}

/*!
   Returns the grid pattern as a QVariant.
*/

QCanvasGridPattern::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasGridPattern::operator!=(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs)

    \return \c true if the grid pattern \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasGridPattern::operator==(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs)

    \return \c true if the grid pattern \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs) noexcept
{
    auto *d = QCanvasGridPatternPrivate::get(&lhs);
    auto *pd = QCanvasGridPatternPrivate::get(&rhs);
    if (pd == d)
        return true;

    return d->equals(*pd);
}

bool QCanvasGridPatternPrivate::equals(const QCanvasBrushPrivate &other) const noexcept
{
    Q_ASSERT(other.type == type);
    const auto &pd = static_cast<const QCanvasGridPatternPrivate &>(other);

    if (x != pd.x
        || y != pd.y
        || width != pd.width
        || height != pd.height
        || feather != pd.feather
        || angle != pd.angle
        || lineWidth != pd.lineWidth
        || lineColor != pd.lineColor
        || backgroundColor != pd.backgroundColor)
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasGridPattern &p)
{
    QDebugStateSaver saver(dbg);
    const auto &sp = p.startPosition();
    dbg.nospace() << "QCanvasGridPattern(" << sp << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCanvasGridPattern stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasGridPattern &pattern)
    \relates QCanvasGridPattern

    Writes the given \a pattern to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCanvasGridPattern &p)
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
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasGridPattern &pattern)
    \relates QCanvasGridPattern

    Reads the given \a pattern from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCanvasGridPattern &p)
{
    qreal x, y, width, height, lineWidth, feather, angle;
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
    Returns the start point of grid pattern.
    \sa setStartPosition()
*/

QPointF QCanvasGridPattern::startPosition() const
{
    return QPointF(d->x, d->y);
}

/*!
    Sets the start point of grid pattern to (\a x, \a y).
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/

void QCanvasGridPattern::setStartPosition(qreal x, qreal y)
{
    detach();
    d->x = x;
    d->y = y;
    d->changed = true;
}

/*!
    \fn void QCanvasGridPattern::setStartPosition(QPointF point)
    \overload

    Sets the start point of grid pattern to \a point.
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/


/*!
    Returns the size of a single cell in grid pattern.
    \sa setCellSize()
*/

QSizeF QCanvasGridPattern::cellSize() const
{
    return QSizeF(d->width, d->height);
}

/*!
    Sets the size of a single cell in grid pattern to \a width, \a height.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

void QCanvasGridPattern::setCellSize(qreal width, qreal height)
{
    detach();
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    \fn void QCanvasGridPattern::setCellSize(QSizeF size)
    \overload

    Sets the size of a single cell in grid pattern to \a size.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

/*!
    Returns the width of a stroke line in grid pattern.
    \sa setLineWidth()
*/

qreal QCanvasGridPattern::lineWidth() const
{
    return d->lineWidth;
}

/*!
    Sets the width of a stroke line in grid pattern to \a width.
    The default value is \c 1.0.
*/

void QCanvasGridPattern::setLineWidth(qreal width)
{
    detach();
    d->lineWidth = width;
    d->changed = true;
}

/*!
    Returns the pattern feather in pixels.
*/

qreal QCanvasGridPattern::feather() const
{
    return d->feather;
}

/*!
    Sets the pattern feather to \a feather in pixels.
    The default value is \c 1.0, meaning a single pixel antialiasing.
*/

void QCanvasGridPattern::setFeather(qreal feather)
{
    detach();
    d->feather = feather;
    d->changed = true;
}

/*!
    Returns the pattern rotation in radians.
*/

qreal QCanvasGridPattern::rotation() const
{
    return d->angle;
}

/*!
    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the grid startPosition().
    The default value is \c 0.0, meaning the grid is not rotated.
*/

void QCanvasGridPattern::setRotation(qreal rotation)
{
    detach();
    d->angle = rotation;
    d->changed = true;
}

/*!
    Returns the pattern grid line color.
*/

QColor QCanvasGridPattern::lineColor() const
{
    return d->lineColor;
}

/*!
    Sets the pattern grid line color to \a color.
    The default value is white.
*/

void QCanvasGridPattern::setLineColor(const QColor &color)
{
    detach();
    d->lineColor = color;
    d->changed = true;
}

/*!
    Returns the pattern grid background color.
*/

QColor QCanvasGridPattern::backgroundColor() const
{
    return d->backgroundColor;
}

/*!
    Sets the pattern grid background color to \a color.
    The default value is black.
*/

void QCanvasGridPattern::setBackgroundColor(const QColor &color)
{
    detach();
    d->backgroundColor = color;
    d->changed = true;
}

// ***** Private *****

/*!
   \internal
*/
#define DECONST(d) const_cast<QCanvasGridPatternPrivate *>(d)

QCPaint QCanvasGridPatternPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    Q_UNUSED(painter);
    if (d->changed) {
        createGridPattern();
        DECONST(d)->changed = false;
    }
    return d->paint;
}

void QCanvasGridPatternPrivate::createGridPattern() const
{
    auto *d = this;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushGrid;
    p.transform = QTransform::fromTranslate(d->x, d->y);
    if (!qFuzzyIsNull(d->angle))
        p.transform = p.transform.rotateRadians(d->angle);

    p.extent[0] = float(d->width);
    p.extent[1] = float(d->height);

    p.feather = float(d->feather);
    p.radius = float(d->lineWidth);

    p.innerColor = { d->lineColor.redF(),
                     d->lineColor.greenF(),
                     d->lineColor.blueF(),
                     d->lineColor.alphaF() };
    p.outerColor = { d->backgroundColor.redF(),
                     d->backgroundColor.greenF(),
                     d->backgroundColor.blueF(),
                     d->backgroundColor.alphaF() };
}

QCanvasBrushPrivate *QCanvasGridPatternPrivate::clone()
{
    return new QCanvasGridPatternPrivate(*this);
}

#undef DECONST

QT_END_NAMESPACE
