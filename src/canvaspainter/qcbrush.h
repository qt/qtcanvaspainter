// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCBRUSH_H
#define QCBRUSH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QCPainter;
struct QCPaint;

class QCGradient;
class QCBoxShadow;
class QCImagePattern;
class QCGridPattern;
class QCCustomBrush;

class QCBrushPrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCBrushPrivate)

class Q_CANVASPAINTER_EXPORT QCBrush
{
    Q_GADGET
public:
    QCBrush();
    QCBrush(const QCBrush &brush);
    ~QCBrush();

    QCBrush &operator=(const QCBrush &brush) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCBrush)
    void swap(QCBrush &other) noexcept { baseData.swap(other.baseData); }

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
    explicit QCBrush(QCBrushPrivate *priv);
    QExplicitlySharedDataPointer<QCBrushPrivate> baseData;
private:
    QCPaint createPaint(QCPainter *painter) const;
    friend class QCPainter;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCBrush &);
#endif
};

Q_DECLARE_SHARED(QCBrush)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCBrush &);
#endif

QT_END_NAMESPACE

#endif // QCBRUSH_H
