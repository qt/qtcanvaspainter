// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCRHIPAINTPAINTDRIVER_P_H
#define QCRHIPAINTPAINTDRIVER_P_H

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

#include "qcrhipaintdriver.h"
#include "engine/qcpainterrhirenderer_p.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

class QCRhiPaintDriverPrivate
{
public:
    static const QCRhiPaintDriverPrivate *get(const QCRhiPaintDriver *obj) { return obj->d; }
    static QCRhiPaintDriverPrivate *get(QCRhiPaintDriver *obj) { return obj->d; }

    QCPainter *painter = nullptr;
    QCPainterRhiRenderer *renderer = nullptr;
    QRhiCommandBuffer *currentCb = nullptr;
    QRhiRenderTarget *currentRt = nullptr;
    float mainLogicalWidth = 0.0f;
    float mainLogicalHeight = 0.0f;
    float mainDpr = 1.0f;
    QColor mainFillColor;
    QCOffscreenCanvas currentCanvas;
};

QT_END_NAMESPACE

#endif // QCRHIPAINTPAINTDRIVER_P_H
