// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCGRADIENT_P_H
#define QCGRADIENT_P_H

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

#include <QtCore/qshareddata.h>
#include "qcgradient.h"
#include "engine/qcpainterengineutils_p.h"
#include <QImage>
#include <QColor>

QT_BEGIN_NAMESPACE

class QCGradientPrivate : public QSharedData
{
public:
    QCGradientPrivate(QCBrush::BrushType type);
    qint64 generateGradientId() const;
    static void gradientColorSpan(quint32 *data, const QColor &color0, const QColor &color1, float offset0, float offset1);
    void updateGradientTexture(QCPainter *painter);

    enum class DirtyFlag {
        Stops = 0x01,
        Values = 0x02,
        All = 0xFF
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    // Variables specific to gradient types
    union QCGradientData {
        QCGradientData() {}
        struct {
            float sx, sy, ex, ey;
        } linear;
        struct {
            float cx, cy, oRadius, iRadius;
        } radial;
        struct {
            float cx, cy, angle;
        } conical;
        struct {
            float x, y, width, height, feather, radius;
        } box;
    };
    QCGradientStops gradientStops;
    QCGradientData data;
    QCBrush::BrushType type;
    QCPaint paint;
    DirtyFlags dirty;
    int imageId;
};

QT_END_NAMESPACE

#endif // QCGRADIENT_P_H
