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

namespace {
template <typename GradientData>
bool qCanvasGradientDataEquals(QCanvasBrush::BrushType type,
                               const GradientData &lhs,
                               const GradientData &rhs) noexcept
{
    switch (type) {
    case QCanvasBrush::BrushType::LinearGradient:
        return lhs.linear.sx == rhs.linear.sx
            && lhs.linear.sy == rhs.linear.sy
            && lhs.linear.ex == rhs.linear.ex
            && lhs.linear.ey == rhs.linear.ey;
    case QCanvasBrush::BrushType::RadialGradient:
        return lhs.radial.icx == rhs.radial.icx
            && lhs.radial.icy == rhs.radial.icy
            && lhs.radial.iRadius == rhs.radial.iRadius
            && lhs.radial.ocx == rhs.radial.ocx
            && lhs.radial.ocy == rhs.radial.ocy
            && lhs.radial.oRadius == rhs.radial.oRadius;
    case QCanvasBrush::BrushType::ConicalGradient:
        return lhs.conical.cx == rhs.conical.cx
            && lhs.conical.cy == rhs.conical.cy
            && lhs.conical.angle == rhs.conical.angle;
    case QCanvasBrush::BrushType::BoxGradient:
        return lhs.box.x == rhs.box.x
            && lhs.box.y == rhs.box.y
            && lhs.box.width == rhs.box.width
            && lhs.box.height == rhs.box.height
            && lhs.box.feather == rhs.box.feather
            && lhs.box.radius == rhs.box.radius;
    default:
        return true;
    }
}
} // namespace

class QCanvasGradientBrushPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasGradientBrushPrivate(QCanvasBrush::BrushType type);

    bool equals(const QCanvasBrushPrivate &other) const noexcept override;

    qint64 generateGradientKey() const;
    void updateGradientTexture(QCanvasPainter *painter);

    enum class DirtyFlag {
        Stops = 0x01,
        Values = 0x02,
        All = 0xFF
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    // Returns the data of gradient for reading.
    static const QCanvasGradientBrushPrivate *get(const QCanvasGradient &gradient)
    {
        return static_cast<const QCanvasGradientBrushPrivate *>(
                    QCanvasBrushPrivate::get(gradient.m_brush));
    }

    // Returns the data of gradient for writing, detaching it from any copy or
    // brush it might share the data with.
    static QCanvasGradientBrushPrivate *get(QCanvasGradient &gradient)
    {
        return static_cast<QCanvasGradientBrushPrivate *>(
                    QCanvasBrushPrivate::get(gradient.m_brush));
    }

    // Constructs a gradient of type Gradient sharing the data of brush.
    template <typename Gradient>
    static Gradient create(const QCanvasBrush &brush) { return Gradient(brush); }

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

    // The gradient's data; all of it participates in equality comparison.
    QCanvasGradientStops gradientStops;
    QCanvasGradientData data;
    int imageId;
    float imageY;

    // Rendering cache, generated from the data above.
    QCPaint paint;
    DirtyFlags dirty;
    int textureId;
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
