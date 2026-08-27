// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCPAINT_P_H
#define QCPAINT_P_H

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

#include <QTransform>

QT_BEGIN_NAMESPACE

enum QCanvasBrushType {
    BrushColor,
    BrushLinearGradient,
    BrushRadialGradient,
    BrushConicalGradient,
    BrushBoxGradient,
    BrushBoxShadow,
    BrushImage,
    BrushGrid,
    BrushRadialGradientExtended,
};

struct QCColor {
    float r, g, b, a;
};

struct QCPaint {
    QTransform transform;
    QCColor innerColor = {0.0f, 0.0f, 0.0f, 1.0f};
    QCColor outerColor = {0.0f, 0.0f, 0.0f, 1.0f};
    QCanvasBrushType brushType = BrushColor;
    int imageId = 0;
    float extent[2];
    float radius = 0.0f;
    float feather = 1.0f;
    float alpha = 1.0f;
};

QT_END_NAMESPACE

#endif // QCPAINT_P_H
