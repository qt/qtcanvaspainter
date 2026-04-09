// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dcommandbuffer_p.h"
#include "qcanvas2ditem_p.h"

QT_BEGIN_NAMESPACE

QCanvas2DCommandBuffer::QCanvas2DCommandBuffer()
{
    static bool registered = false;
    if (!registered) {
        qRegisterMetaType<QCanvas2DCommandBuffer*>("QCanvas2DCommandBuffer*");
        registered = true;
    }
}

QCanvas2DCommandBuffer::~QCanvas2DCommandBuffer()
{
}

void QCanvas2DCommandBuffer::clearBuffers()
{
    commands.clear();
    ints.clear();
    bools.clear();
    reals.clear();
    rects.clear();
    colors.clear();
    matrixes.clear();
    brushes.clear();
    paths.clear();
    canvasPaths.clear();
    images.clear();
    fonts.clear();
    pixmaps.clear();
    strings.clear();
}

QT_END_NAMESPACE
