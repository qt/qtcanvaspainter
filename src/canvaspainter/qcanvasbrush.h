// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBRUSH_H
#define QCANVASBRUSH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QCanvasPainter;

class QCanvasGradient;
class QCanvasBoxShadow;
class QCanvasImagePattern;
class QCanvasGridPattern;
class QCanvasCustomBrush;

class QCanvasBrushPrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasBrushPrivate)

class QCanvasBrush
{
    Q_GADGET_EXPORT(Q_CANVASPAINTER_EXPORT)
public:
    Q_CANVASPAINTER_EXPORT QCanvasBrush();
    Q_CANVASPAINTER_EXPORT QCanvasBrush(const QCanvasBrush &brush);
    Q_CANVASPAINTER_EXPORT ~QCanvasBrush();

    Q_CANVASPAINTER_EXPORT QCanvasBrush &operator=(const QCanvasBrush &brush);
    QCanvasBrush(QCanvasBrush &&other) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasBrush)
    void swap(QCanvasBrush &other) noexcept { baseData.swap(other.baseData); }

    enum class BrushType {
        Invalid,
        LinearGradient,
        RadialGradient,
        ConicalGradient,
        BoxGradient,
        BoxShadow,
        ImagePattern,
        GridPattern,
        // Add new brush types here
        Custom = 1000
    };
    Q_ENUM(BrushType)

    Q_CANVASPAINTER_EXPORT BrushType type() const;

    template<typename T>
    T as() const
    {
        static_assert(!sizeof(T), "QCanvasBrush::as<T>() is not supported for this brush type");
        return T{};
    }

protected:
    Q_CANVASPAINTER_EXPORT explicit QCanvasBrush(QCanvasBrushPrivate *priv);
    QExplicitlySharedDataPointer<QCanvasBrushPrivate> baseData;
private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasBrush &lhs, const QCanvasBrush &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasBrush)

    friend class QCanvasBrushPrivate;
    friend class QCanvasPainter;
    friend class QCanvasPainterPrivate;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBrush &);
#endif
};

Q_DECLARE_SHARED(QCanvasBrush)

QT_END_NAMESPACE

#endif // QCANVASBRUSH_H
