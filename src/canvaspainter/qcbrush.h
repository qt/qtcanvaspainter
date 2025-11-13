// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCBRUSH_H
#define QCBRUSH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QCPainter;
struct QCPaint;

class Q_CANVASPAINTER_EXPORT QCBrush
{
    Q_GADGET
public:
    virtual ~QCBrush();

    enum class BrushType {
        Brush,
        LinearGradient,
        RadialGradient,
        ConicalGradient,
        BoxGradient,
        BoxShadow,
        ImagePattern,
        Custom
    };
    Q_ENUM(BrushType)

    virtual BrushType type() const;

private:
    // Reimplement this in brush sub-classes
    virtual QCPaint createPaint(QCPainter *painter) const;
private:
    friend class QCPainter;
};

QT_END_NAMESPACE

#endif // QCBRUSH_H
