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

struct QCanvasGradientStop
{
    float position;
    QColor color;

private:
    friend constexpr bool comparesEqual(const QCanvasGradientStop &lhs, const QCanvasGradientStop &rhs) noexcept
    {
        return qFuzzyCompare(1 + lhs.position, 1 + rhs.position) && lhs.color == rhs.color;
    }
    Q_DECLARE_EQUALITY_COMPARABLE_LITERAL_TYPE(QCanvasGradientStop)

#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradientStop &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradientStop &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradient &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradient &);
#endif
};

typedef QList<QCanvasGradientStop> QCanvasGradientStops;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradientStop &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradientStop &);
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGradient &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGradient &);
#endif

class QCanvasGradient
{
public:
    QCanvasGradient() = default;
    QCanvasGradient(const QCanvasGradient &) = default;
    QCanvasGradient &operator=(const QCanvasGradient &) = default;
    QCanvasGradient(QCanvasGradient &&) = default;
    QCanvasGradient &operator=(QCanvasGradient &&) = default;

    Q_CANVASPAINTER_EXPORT QCanvasBrush::BrushType type() const;

    Q_CANVASPAINTER_EXPORT QColor startColor() const;
    Q_CANVASPAINTER_EXPORT void setStartColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT QColor endColor() const;
    Q_CANVASPAINTER_EXPORT void setEndColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT void setColorAt(float position, const QColor &color);
    Q_CANVASPAINTER_EXPORT void setStops(const QCanvasGradientStops &stops);
    Q_CANVASPAINTER_EXPORT QCanvasGradientStops stops() const;
    Q_CANVASPAINTER_EXPORT void setImage(const QCanvasImage &image, int index = 0);
    inline void addColorStop(float position, const QColor &color);

    Q_CANVASPAINTER_EXPORT operator QCanvasBrush() const;
    Q_CANVASPAINTER_EXPORT operator QVariant() const;

protected:
    Q_CANVASPAINTER_EXPORT explicit QCanvasGradient(QCanvasBrush::BrushType type);
    ~QCanvasGradient() = default;

    QCanvasBrush::BrushType m_type = QCanvasBrush::BrushType::Invalid;
    QCanvasGradientStops m_stops;
    int m_imageId = 0;
    float m_imageY = 0.5f;
    union {
        struct { float sx, sy, ex, ey; } linear;
        struct { float icx, icy, iRadius, ocx, ocy, oRadius; } radial;
        struct { float cx, cy, angle; } conical;
        struct { float x, y, width, height, feather, radius; } box;
    } m_data {};

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasGradient &lhs, const QCanvasGradient &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasGradient)
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug dbg, const QCanvasGradientStop &stop);
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGradient &);
#endif
};

inline void QCanvasGradient::addColorStop(float position, const QColor &color)
{
    setColorAt(position, color);
}

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug dbg, const QCanvasGradientStop &stop);
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGradient &);
#endif

QT_END_NAMESPACE

#endif // QCANVASGRADIENT_H
