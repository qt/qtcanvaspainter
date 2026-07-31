// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASGRADIENT_H
#define QCANVASGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtGui/qcolor.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QCanvasImage;
class QCanvasGradientBrushPrivate;

struct QCanvasGradientStop
{
    qreal position;
    QColor color;

private:
    friend constexpr bool comparesEqual(const QCanvasGradientStop &lhs, const QCanvasGradientStop &rhs) noexcept
    {
QT_WARNING_PUSH
QT_WARNING_DISABLE_FLOAT_COMPARE
        return lhs.position == rhs.position && lhs.color == rhs.color;
QT_WARNING_POP
    }
    Q_DECLARE_EQUALITY_COMPARABLE_LITERAL_TYPE(QCanvasGradientStop)

    friend bool qFuzzyCompare(const QCanvasGradientStop &lhs, const QCanvasGradientStop &rhs) noexcept
    {
        return qFuzzyCompare(1 + lhs.position, 1 + rhs.position) && lhs.color == rhs.color;
    }

#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug dbg, const QCanvasGradientStop &stop);
#endif

#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradientStop &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradientStop &);
#endif
    friend size_t qHash(const QCanvasGradientStop &, size_t seed) = delete;
};

typedef QList<QCanvasGradientStop> QCanvasGradientStops;

class QCanvasGradient
{
public:
    QCanvasGradient() = delete;

    Q_CANVASPAINTER_EXPORT QCanvasBrush::BrushType type() const;

    Q_CANVASPAINTER_EXPORT QColor startColor() const;
    Q_CANVASPAINTER_EXPORT void setStartColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT QColor endColor() const;
    Q_CANVASPAINTER_EXPORT void setEndColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT void setColorAt(qreal position, const QColor &color);
    Q_CANVASPAINTER_EXPORT void setStops(const QCanvasGradientStops &stops);
    Q_CANVASPAINTER_EXPORT QCanvasGradientStops stops() const;
    Q_CANVASPAINTER_EXPORT void setImage(const QCanvasImage &image, int index = 0);
    void addColorStop(qreal position, const QColor &color)
    {
        setColorAt(position, color);
    }

    Q_IMPLICIT operator QCanvasBrush() const & { return m_brush; }
    Q_IMPLICIT operator QCanvasBrush() && noexcept { return std::move(m_brush); }
    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QVariant() const;

protected:
    Q_CANVASPAINTER_EXPORT explicit QCanvasGradient(QCanvasBrush::BrushType type);
    Q_CANVASPAINTER_EXPORT explicit QCanvasGradient(const QCanvasBrush &brush);

    QCanvasGradient(const QCanvasGradient &) = default;
    QCanvasGradient &operator=(const QCanvasGradient &) = default;
    QCanvasGradient(QCanvasGradient &&) = default;
    QCanvasGradient &operator=(QCanvasGradient &&) = default;

    ~QCanvasGradient() = default;

private:
    QCanvasBrush m_brush;

    friend class QCanvasGradientBrushPrivate;
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasGradient &lhs, const QCanvasGradient &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasGradient)

#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGradient &);
#endif

#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradient &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradient &);
#endif
    friend size_t qHash(const QCanvasGradient &, size_t seed) = delete;
};

QT_END_NAMESPACE

#endif // QCANVASGRADIENT_H
