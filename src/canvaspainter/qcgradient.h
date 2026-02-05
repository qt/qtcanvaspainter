// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCGRADIENT_H
#define QCGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcbrush.h>
#include <QtGui/qcolor.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qtguiglobal.h>

QT_BEGIN_NAMESPACE

class QCGradientPrivate;
class QCGradient;

struct QCGradientStop
{
    float position;
    QColor color;

    friend bool operator==(const QCGradientStop &a, const QCGradientStop &b) noexcept
    {
        return qFuzzyCompare(a.position, b.position) && a.color == b.color;
    }

    friend bool operator!=(const QCGradientStop &a, const QCGradientStop &b) noexcept
    {
        return !(a == b);
    }
};

typedef QList<QCGradientStop> QCGradientStops;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCGradientStop &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCGradientStop &);
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCGradient &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCGradient &);
#endif

class Q_CANVASPAINTER_EXPORT QCGradient : public QCBrush
{
public:
    bool operator==(const QCGradient &gradient) const;
    bool operator!=(const QCGradient &gradient) const { return !(operator==(gradient)); }
    operator QVariant() const;

    QCBrush::BrushType type() const;

    QColor startColor() const;
    void setStartColor(const QColor &color);
    QColor endColor() const;
    void setEndColor(const QColor &color);
    void setColorAt(float position, const QColor &color);
    void setStops(const QCGradientStops &stops);
    QCGradientStops stops() const;

protected:
    QCGradient(QCGradientPrivate *);
private:
    friend class QCGradientPrivate;
};

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug dbg, const QCGradientStop &stop);
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCGradient &);
#endif

QT_END_NAMESPACE

#endif // QCGRADIENT_H
