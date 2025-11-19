// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCGRIDPATTERN_H
#define QCGRIDPATTERN_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcbrush.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCGridPatternPrivate;
class QCGridPattern;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCGridPattern &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCGridPattern &);
#endif

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCGridPatternPrivate)

class Q_CANVASPAINTER_EXPORT QCGridPattern : public QCBrush
{
public:
    QCGridPattern();
    QCGridPattern(const QRectF &rect,
                  const QColor &lineColor = QColorConstants::White,
                  const QColor &backgroundColor = QColorConstants::Black,
                  float lineWidth = 1.0f, float feather = 1.0f, float angle = 0.0f);
    QCGridPattern(float x, float y, float width, float height,
                  const QColor &lineColor = QColorConstants::White,
                  const QColor &backgroundColor = QColorConstants::Black,
                  float lineWidth = 1.0f, float feather = 1.0f, float angle = 0.0f);
    QCGridPattern(const QCGridPattern &pattern) noexcept;
    ~QCGridPattern();

    QCGridPattern &operator=(const QCGridPattern &pattern) noexcept;
    QCGridPattern(QCGridPattern &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCGridPattern)
    void swap(QCGridPattern &other) noexcept { d.swap(other.d); }

    bool operator==(const QCGridPattern &pattern) const;
    inline bool operator!=(const QCGridPattern &pattern) const { return !(operator==(pattern)); }
    operator QVariant() const;

    void detach();
    BrushType type() const override;

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    void setStartPosition(QPointF point);
    QSizeF cellSize() const;
    void setCellSize(float width, float height);
    void setCellSize(QSizeF size);
    float lineWidth() const;
    void setLineWidth(float width);
    float feather() const;
    void setFeather(float feather);
    float rotation() const;
    void setRotation(float rotation);
    QColor lineColor() const;
    void setLineColor(const QColor &color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

private:
    QCPaint createPaint(QCPainter *painter) const final;

private:
    QExplicitlySharedDataPointer<QCGridPatternPrivate> d;
};

Q_DECLARE_SHARED(QCGridPattern)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCGridPattern &);
#endif

QT_END_NAMESPACE

#endif // QCGRIDPATTERN_H
