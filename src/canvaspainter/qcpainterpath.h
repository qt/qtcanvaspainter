// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
    // TODO: Copy constructor etc.

    // Path commands
    // These should match to path methods of QCPainter for consistency.
    void closePath();
    void moveTo(float x, float y);
    void moveTo(QPointF point);
    void lineTo(float x, float y);
    void lineTo(QPointF point);
    void bezierCurveTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    void bezierCurveTo(
        QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    void quadraticCurveTo(float cx, float cy, float x, float y);
    void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    void arcTo(float c1x, float c1y, float c2x, float c2y, float radius);
    void arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius);
    void arc(
        float centerX,
        float centerY,
        float radius,
        float a0,
        float a1,
        QCPainter::PathWinding direction = QCPainter::PathWinding::ClockWise,
        bool isConnected = true);
    void arc(
        QPointF centerPoint,
        float radius,
        float a0,
        float a1,
        QCPainter::PathWinding direction = QCPainter::PathWinding::ClockWise,
        bool isConnected = true);
    void rect(float x, float y, float width, float height);
    void rect(const QRectF &rect);
    void roundRect(float x, float y, float width, float height, float radius);
    void roundRect(const QRectF &rect, float radius);
    void roundRect(
        float x,
        float y,
        float width,
        float height,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    void roundRect(
        const QRectF &rect,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    void ellipse(float x, float y, float radiusX, float radiusY);
    void ellipse(const QRectF &rect);
    void circle(float x, float y, float radius);
    void circle(QPointF centerPoint, float radius);

    void setPathWinding(QCPainter::PathWinding winding);
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
    void reserveCommands(qsizetype size);
    void reserveCommandsData(qsizetype size);

    // Other
    QPointF currentPosition() const;
    QPointF positionAt(qsizetype index) const;
    [[nodiscard]] QCPainterPath sliced(qsizetype start, qsizetype count, const QTransform &transform = QTransform()) const &;

private:
    friend class QCPainterEngine;
    friend class QCPainterRhiRenderer;

    Q_DECLARE_PRIVATE(QCPainterPath)
    QCPainterPathPrivate *d_ptr;

};

QT_END_NAMESPACE

#endif // QCPAINTERPATH_H
