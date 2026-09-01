// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPATH_H
#define QCANVASPATH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstringview.h>
#include <QtCanvasPainter/qcanvaspainter.h>

QT_BEGIN_NAMESPACE

class QCanvasPathPrivate;
class QTransform;

class QCanvasPath
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasPath();
    Q_CANVASPAINTER_EXPORT explicit QCanvasPath(qsizetype commandsSize, qsizetype commandsDataSize = -1);
    Q_CANVASPAINTER_EXPORT QCanvasPath(const QCanvasPath &path);
    Q_CANVASPAINTER_EXPORT ~QCanvasPath();

    Q_CANVASPAINTER_EXPORT QCanvasPath &operator=(const QCanvasPath &path);
    QCanvasPath(QCanvasPath &&other) noexcept : d_ptr(std::exchange(other.d_ptr, nullptr)) { }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasPath)
    void swap(QCanvasPath &other) noexcept { qt_ptr_swap(d_ptr, other.d_ptr); }

    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QVariant() const;

    // Path commands
    // These should match to path methods of QCanvasPainter for consistency.
    Q_CANVASPAINTER_EXPORT void closePath();
    Q_CANVASPAINTER_EXPORT void moveTo(qreal x, qreal y);
    inline void moveTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void lineTo(qreal x, qreal y);
    inline void lineTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void bezierCurveTo(qreal c1x, qreal c1y, qreal c2x, qreal c2y, qreal x, qreal y);
    inline void bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void quadraticCurveTo(qreal cx, qreal cy, qreal x, qreal y);
    inline void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void arcTo(qreal c1x, qreal c1y, qreal c2x, qreal c2y, qreal radius);
    inline void arcTo(QPointF controlPoint1, QPointF controlPoint2, qreal radius);
    Q_CANVASPAINTER_EXPORT void arc(
        qreal centerX,
        qreal centerY,
        qreal radius,
        qreal a0,
        qreal a1,
        QCanvasPainter::PathWinding direction = QCanvasPainter::PathWinding::ClockWise,
        QCanvasPainter::PathConnection connection = QCanvasPainter::PathConnection::Connected);
    inline void arc(
        QPointF centerPoint,
        qreal radius,
        qreal a0,
        qreal a1,
        QCanvasPainter::PathWinding direction = QCanvasPainter::PathWinding::ClockWise,
        QCanvasPainter::PathConnection connection = QCanvasPainter::PathConnection::Connected);
    Q_CANVASPAINTER_EXPORT void rect(qreal x, qreal y, qreal width, qreal height);
    inline void rect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void roundRect(qreal x, qreal y, qreal width, qreal height, qreal radius);
    inline void roundRect(const QRectF &rect, qreal radius);
    Q_CANVASPAINTER_EXPORT void roundRect(
        qreal x,
        qreal y,
        qreal width,
        qreal height,
        qreal radiusTopLeft,
        qreal radiusTopRight,
        qreal radiusBottomRight,
        qreal radiusBottomLeft);
    inline void roundRect(
        const QRectF &rect,
        qreal radiusTopLeft,
        qreal radiusTopRight,
        qreal radiusBottomRight,
        qreal radiusBottomLeft);
    Q_CANVASPAINTER_EXPORT void ellipse(qreal x, qreal y, qreal radiusX, qreal radiusY);
    inline void ellipse(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void circle(qreal x, qreal y, qreal radius);
    inline void circle(QPointF centerPoint, qreal radius);

    Q_CANVASPAINTER_EXPORT void setPathWinding(QCanvasPainter::PathWinding winding);
    Q_CANVASPAINTER_EXPORT void beginSolidSubPath();
    Q_CANVASPAINTER_EXPORT void beginHoleSubPath();

    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path, const QTransform &transform = QTransform());
    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path, qsizetype start, qsizetype count, const QTransform &transform = QTransform());
    Q_CANVASPAINTER_EXPORT void addPath(QStringView svgPath, const QTransform &transform = QTransform());
    Q_CANVASPAINTER_EXPORT void addPath(const QPainterPath &path);

    // Memory and size management
    Q_CANVASPAINTER_EXPORT bool isEmpty() const;
    Q_CANVASPAINTER_EXPORT void clear();
    Q_CANVASPAINTER_EXPORT void squeeze();
    Q_CANVASPAINTER_EXPORT qsizetype commandsSize() const;
    Q_CANVASPAINTER_EXPORT qsizetype commandsDataSize() const;
    Q_CANVASPAINTER_EXPORT qsizetype commandsCapacity() const;
    Q_CANVASPAINTER_EXPORT qsizetype commandsDataCapacity() const;
    Q_CANVASPAINTER_EXPORT void reserve(qsizetype commandsSize, qsizetype commandsDataSize = -1);

    // Other
    Q_CANVASPAINTER_EXPORT QPointF currentPosition() const;
    Q_CANVASPAINTER_EXPORT QPointF positionAt(qsizetype index) const;
    [[nodiscard]] Q_CANVASPAINTER_EXPORT QCanvasPath sliced(qsizetype start, qsizetype count, const QTransform &transform = QTransform()) const;

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasPath &lhs, const QCanvasPath &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasPath)

    friend class QCPainterEngine;
    friend class QCPainterRhiRenderer;
    friend size_t qHash(const QCanvasPath &, size_t seed) = delete;

    Q_DECLARE_PRIVATE(QCanvasPath)
    QCanvasPathPrivate *d_ptr;
};

void QCanvasPath::moveTo(QPointF point)
{
    moveTo(point.x(), point.y());
}

void QCanvasPath::lineTo(QPointF point)
{
    lineTo(point.x(), point.y());
}

void QCanvasPath::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(controlPoint1.x(), controlPoint1.y(),
                  controlPoint2.x(), controlPoint2.y(),
                  endPoint.x(), endPoint.y());
}

void QCanvasPath::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(controlPoint.x(), controlPoint.y(),
                     endPoint.x(), endPoint.y());
}

void QCanvasPath::arcTo(QPointF point1, QPointF point2, qreal radius)
{
    arcTo(point1.x(), point1.y(),
          point2.x(), point2.y(),
          radius);
}

void QCanvasPath::arc(
    QPointF centerPoint,
    qreal radius,
    qreal a0,
    qreal a1,
    QCanvasPainter::PathWinding direction,
    QCanvasPainter::PathConnection connection)
{
    arc(centerPoint.x(), centerPoint.y(),
        radius, a0, a1, direction, connection);
}

void QCanvasPath::rect(const QRectF &rect)
{
    this->rect(rect.x(),
               rect.y(),
               rect.width(),
               rect.height());
}

void QCanvasPath::roundRect(const QRectF &rect, qreal radius)
{
    roundRect(rect.x(),
              rect.y(),
              rect.width(),
              rect.height(),
              radius);
}

void QCanvasPath::roundRect(
    const QRectF &rect,
    qreal radiusTopLeft,
    qreal radiusTopRight,
    qreal radiusBottomRight,
    qreal radiusBottomLeft)
{
    roundRect(rect.x(),
              rect.y(),
              rect.width(),
              rect.height(),
              radiusTopLeft, radiusTopRight,
              radiusBottomRight, radiusBottomLeft);
}

void QCanvasPath::ellipse(const QRectF &rect)
{
    ellipse(rect.x() + rect.width() * 0.5,
            rect.y() + rect.height() * 0.5,
            rect.width() * 0.5,
            rect.height() * 0.5);
}

void QCanvasPath::circle(QPointF centerPoint, qreal radius)
{
    circle(centerPoint.x(), centerPoint.y(), radius);
}

QT_END_NAMESPACE

#endif // QCANVASPATH_H
