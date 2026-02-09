// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCPAINTERPATH_H
#define QCPAINTERPATH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtCanvasPainter/qcpainter.h>

QT_BEGIN_NAMESPACE

class QCPainterPathPrivate;
class QTransform;

class Q_CANVASPAINTER_EXPORT QCPainterPath
{
public:

    QCPainterPath();
    explicit QCPainterPath(qsizetype commandsSize, qsizetype commandsDataSize = -1);
    QCPainterPath(const QCPainterPath &path);
    ~QCPainterPath();

    QCPainterPath &operator=(const QCPainterPath &path) noexcept;
    QCPainterPath(QCPainterPath &&other) : d_ptr(std::exchange(other.d_ptr, nullptr)) { }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCPainterPath)
    void swap(QCPainterPath &other) noexcept { qt_ptr_swap(d_ptr, other.d_ptr); }

    operator QVariant() const;

    // Path commands
    // These should match to path methods of QCPainter for consistency.
    void closePath();
    void moveTo(float x, float y);
    inline void moveTo(QPointF point);
    void lineTo(float x, float y);
    inline void lineTo(QPointF point);
    void bezierCurveTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    inline void bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    void quadraticCurveTo(float cx, float cy, float x, float y);
    inline void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    void arcTo(float c1x, float c1y, float c2x, float c2y, float radius);
    inline void arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius);
    void arc(
        float centerX,
        float centerY,
        float radius,
        float a0,
        float a1,
        QCPainter::PathWinding direction = QCPainter::PathWinding::ClockWise,
        QCPainter::PathConnection connection = QCPainter::PathConnection::Connected);
    inline void arc(
        QPointF centerPoint,
        float radius,
        float a0,
        float a1,
        QCPainter::PathWinding direction = QCPainter::PathWinding::ClockWise,
        QCPainter::PathConnection connection = QCPainter::PathConnection::Connected);
    void rect(float x, float y, float width, float height);
    inline void rect(const QRectF &rect);
    void roundRect(float x, float y, float width, float height, float radius);
    inline void roundRect(const QRectF &rect, float radius);
    void roundRect(
        float x,
        float y,
        float width,
        float height,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    inline void roundRect(
        const QRectF &rect,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    void ellipse(float x, float y, float radiusX, float radiusY);
    inline void ellipse(const QRectF &rect);
    void circle(float x, float y, float radius);
    inline void circle(QPointF centerPoint, float radius);

    void setPathWinding(QCPainter::PathWinding winding);
    void beginSolidSubPath();
    void beginHoleSubPath();

    void addPath(const QCPainterPath &path, const QTransform &transform = QTransform());
    void addPath(const QCPainterPath &path, qsizetype start, qsizetype count, const QTransform &transform = QTransform());

    // Memory and size management
    bool isEmpty() const;
    void clear();
    void squeeze();
    qsizetype commandsSize() const;
    qsizetype commandsDataSize() const;
    qsizetype commandsCapacity() const;
    qsizetype commandsDataCapacity() const;
    void reserve(qsizetype commandsSize);
    void reserve(qsizetype commandsSize, qsizetype commandsDataSize);

    // Other
    QPointF currentPosition() const;
    QPointF positionAt(qsizetype index) const;
    [[nodiscard]] QCPainterPath sliced(qsizetype start, qsizetype count, const QTransform &transform = QTransform()) const;

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCPainterPath &lhs, const QCPainterPath &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCPainterPath)

    friend class QCPainterEngine;
    friend class QCPainterRhiRenderer;

    Q_DECLARE_PRIVATE(QCPainterPath)
    QCPainterPathPrivate *d_ptr;

};

inline void QCPainterPath::moveTo(QPointF point)
{
    moveTo(float(point.x()), float(point.y()));
}

inline void QCPainterPath::lineTo(QPointF point)
{
    lineTo(float(point.x()), float(point.y()));
}

inline void QCPainterPath::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(float(controlPoint1.x()), float(controlPoint1.y()),
                  float(controlPoint2.x()), float(controlPoint2.y()),
                  float(endPoint.x()), float(endPoint.y()));
}

inline void QCPainterPath::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(float(controlPoint.x()), float(controlPoint.y()),
                     float(endPoint.x()), float(endPoint.y()));
}

inline void QCPainterPath::arcTo(QPointF point1, QPointF point2, float radius)
{
    arcTo(float(point1.x()), float(point1.y()),
          float(point2.x()), float(point2.y()),
          radius);
}

inline void QCPainterPath::arc(
    QPointF centerPoint,
    float radius,
    float a0,
    float a1,
    QCPainter::PathWinding direction,
    QCPainter::PathConnection connection)
{
    arc(float(centerPoint.x()), float(centerPoint.y()),
        radius, a0, a1, direction, connection);
}

inline void QCPainterPath::rect(const QRectF &rect)
{
    this->rect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

inline void QCPainterPath::roundRect(const QRectF &rect, float radius)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radius);
}

inline void QCPainterPath::roundRect(
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

inline void QCPainterPath::ellipse(const QRectF &rect)
{
    ellipse(float(rect.x() + rect.width() * 0.5),
            float(rect.y() + rect.height() * 0.5),
            float(rect.width() * 0.5),
            float(rect.height() * 0.5));
}

inline void QCPainterPath::circle(QPointF centerPoint, float radius)
{
    circle(float(centerPoint.x()), float(centerPoint.y()), radius);
}

QT_END_NAMESPACE

#endif // QCPAINTERPATH_H
