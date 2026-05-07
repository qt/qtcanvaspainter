// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASGRADIENT_P_H
#define QCANVASGRADIENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qcanvasbrush_p.h"
#include "qcanvasgradient.h"
#include "engine/qcpainterengineutils_p.h"
#include <QImage>
#include <QColor>

QT_BEGIN_NAMESPACE

class QCanvasGradientBrushPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasGradientBrushPrivate(QCanvasBrush::BrushType type);

    static QCanvasGradient reconstruct(const QCanvasGradientBrushPrivate *p);

    qint64 generateGradientKey() const;
    void updateGradientTexture(QCanvasPainter *painter);

    enum class DirtyFlag {
        Stops = 0x01,
        Values = 0x02,
        All = 0xFF
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    // Variables specific to gradient types
    union QCanvasGradientData {
        QCanvasGradientData() {}
        struct {
            float sx, sy, ex, ey;
        } linear;
        struct {
            float icx, icy, iRadius, ocx, ocy, oRadius;
        } radial;
        struct {
            float cx, cy, angle;
        } conical;
        struct {
            float x, y, width, height, feather, radius;
        } box;
    };
    QCanvasGradientStops gradientStops;
    QCanvasGradientData data;
    QCPaint paint;
    DirtyFlags dirty;
    int imageId;
    float imageY;
};

class QCanvasLinearGradientBrushPrivate : public QCanvasGradientBrushPrivate
{
public:
    QCanvasLinearGradientBrushPrivate() : QCanvasGradientBrushPrivate(QCanvasBrush::BrushType::LinearGradient) {}
    QCanvasLinearGradientBrushPrivate(const QCanvasLinearGradientBrushPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createLinearGradient(const QColor &iColor, const QColor &oColor, int imageId) const;
    QCanvasBrushPrivate *clone() override { return new QCanvasLinearGradientBrushPrivate(*this); }
};

class QCanvasRadialGradientBrushPrivate : public QCanvasGradientBrushPrivate
{
public:
    QCanvasRadialGradientBrushPrivate() : QCanvasGradientBrushPrivate(QCanvasBrush::BrushType::RadialGradient) {}
    QCanvasRadialGradientBrushPrivate(const QCanvasRadialGradientBrushPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createRadialGradient(const QColor &iColor, const QColor &oColor, int imageId) const;
    QCanvasBrushPrivate *clone() override { return new QCanvasRadialGradientBrushPrivate(*this); }
};

class QCanvasConicalGradientBrushPrivate : public QCanvasGradientBrushPrivate
{
public:
    QCanvasConicalGradientBrushPrivate() : QCanvasGradientBrushPrivate(QCanvasBrush::BrushType::ConicalGradient) {}
    QCanvasConicalGradientBrushPrivate(const QCanvasConicalGradientBrushPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createConicalGradient(const QColor &iColor, const QColor &oColor, int imageId) const;
    QCanvasBrushPrivate *clone() override { return new QCanvasConicalGradientBrushPrivate(*this); }
};

class QCanvasBoxGradientBrushPrivate : public QCanvasGradientBrushPrivate
{
public:
    QCanvasBoxGradientBrushPrivate() : QCanvasGradientBrushPrivate(QCanvasBrush::BrushType::BoxGradient) {}
    QCanvasBoxGradientBrushPrivate(const QCanvasBoxGradientBrushPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createBoxGradient(const QColor &iColor, const QColor &oColor, int imageId) const;
    QCanvasBrushPrivate *clone() override { return new QCanvasBoxGradientBrushPrivate(*this); }
};

QT_END_NAMESPACE

#endif // QCANVASGRADIENT_P_H
