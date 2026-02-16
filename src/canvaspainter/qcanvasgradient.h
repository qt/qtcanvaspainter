// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASGRADIENT_H
#define QCANVASGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtGui/qcolor.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qtguiglobal.h>

QT_BEGIN_NAMESPACE

class QCanvasGradientPrivate;
class QCanvasGradient;

struct QCanvasGradientStop
{
    float position;
    QColor color;

    friend constexpr bool comparesEqual(const QCanvasGradientStop &lhs, const QCanvasGradientStop &rhs) noexcept
    {
        return qFuzzyCompare(lhs.position, rhs.position) && lhs.color == rhs.color;
    }
    Q_DECLARE_EQUALITY_COMPARABLE_LITERAL_TYPE(QCanvasGradientStop)
};

typedef QList<QCanvasGradientStop> QCanvasGradientStops;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradientStop &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradientStop &);
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradient &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradient &);
#endif

class Q_CANVASPAINTER_EXPORT QCanvasGradient : public QCanvasBrush
{
public:
    operator QVariant() const;

    QCanvasBrush::BrushType type() const;

    QColor startColor() const;
    void setStartColor(const QColor &color);
    QColor endColor() const;
    void setEndColor(const QColor &color);
    void setColorAt(float position, const QColor &color);
    void setStops(const QCanvasGradientStops &stops);
    QCanvasGradientStops stops() const;

protected:
    QCanvasGradient(QCanvasGradientPrivate *);

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasGradient &lhs, const QCanvasGradient &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasGradient)

    friend class QCanvasGradientPrivate;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug dbg, const QCanvasGradientStop &stop);
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGradient &);
#endif
};

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug dbg, const QCanvasGradientStop &stop);
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGradient &);
#endif

QT_END_NAMESPACE

#endif // QCANVASGRADIENT_H
