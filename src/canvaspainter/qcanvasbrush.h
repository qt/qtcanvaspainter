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
struct QCPaint;

class QCanvasGradient;
class QCanvasBoxShadow;
class QCanvasImagePattern;
class QCanvasGridPattern;
class QCanvasCustomBrush;

class QCanvasBrushPrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasBrushPrivate)

class Q_CANVASPAINTER_EXPORT QCanvasBrush
{
    Q_GADGET
public:
    QCanvasBrush();
    QCanvasBrush(const QCanvasBrush &brush);
    ~QCanvasBrush();

    QCanvasBrush &operator=(const QCanvasBrush &brush) noexcept;
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

    BrushType type() const;
    void detach();

protected:
    explicit QCanvasBrush(QCanvasBrushPrivate *priv);
    QExplicitlySharedDataPointer<QCanvasBrushPrivate> baseData;
private:
    QCPaint createPaint(QCanvasPainter *painter) const;
    friend class QCanvasPainter;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBrush &);
#endif
};

Q_DECLARE_SHARED(QCanvasBrush)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBrush &);
#endif

QT_END_NAMESPACE

#endif // QCANVASBRUSH_H
