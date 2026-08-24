// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASGRIDPATTERN_H
#define QCANVASGRIDPATTERN_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCanvasGridPatternPrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasGridPatternPrivate)

class QCanvasGridPattern
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern();
    QCanvasGridPattern(QPointF startPosition, QSizeF cellSize)
        : QCanvasGridPattern(float(startPosition.x()), float(startPosition.y()),
                             float(cellSize.width()), float(cellSize.height()))
    {}
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern(float startX, float startY,
                                              float cellWidth, float cellHeight);
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern(const QCanvasGridPattern &);
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern &operator=(const QCanvasGridPattern &);
    QCanvasGridPattern(QCanvasGridPattern &&) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasGridPattern)
    Q_CANVASPAINTER_EXPORT ~QCanvasGridPattern();

    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QCanvasBrush() const;
    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QVariant() const;

    Q_CANVASPAINTER_EXPORT QPointF startPosition() const;
    Q_CANVASPAINTER_EXPORT void setStartPosition(float x, float y);
    inline void setStartPosition(QPointF point);
    Q_CANVASPAINTER_EXPORT QSizeF cellSize() const;
    Q_CANVASPAINTER_EXPORT void setCellSize(float width, float height);
    inline void setCellSize(QSizeF size);
    Q_CANVASPAINTER_EXPORT float lineWidth() const;
    Q_CANVASPAINTER_EXPORT void setLineWidth(float width);
    Q_CANVASPAINTER_EXPORT float feather() const;
    Q_CANVASPAINTER_EXPORT void setFeather(float feather);
    Q_CANVASPAINTER_EXPORT float rotation() const;
    Q_CANVASPAINTER_EXPORT void setRotation(float rotation);
    Q_CANVASPAINTER_EXPORT QColor lineColor() const;
    Q_CANVASPAINTER_EXPORT void setLineColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT QColor backgroundColor() const;
    Q_CANVASPAINTER_EXPORT void setBackgroundColor(const QColor &color);
    void swap(QCanvasGridPattern &other) noexcept { d.swap(other.d); }

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasGridPattern)

    void detach();
    explicit QCanvasGridPattern(QCanvasGridPatternPrivate *p);
    friend class QCanvasGridPatternPrivate;
    QExplicitlySharedDataPointer<QCanvasGridPatternPrivate> d;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGridPattern &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGridPattern &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGridPattern &);
#endif
    friend size_t qHash(const QCanvasGridPattern &, size_t seed) = delete;
};

void QCanvasGridPattern::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()), float(point.y()));
}

void QCanvasGridPattern::setCellSize(QSizeF size)
{
    setCellSize(float(size.width()), float(size.height()));
}

template<> Q_CANVASPAINTER_EXPORT QCanvasGridPattern QCanvasBrush::as<QCanvasGridPattern>() const;

QT_END_NAMESPACE

#endif // QCANVASGRIDPATTERN_H
