// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dcommandbuffer_p.h"
#include "qcanvas2ditem_p.h"
#include <QtCanvasPainter/qcanvaslineargradient.h>
#include <QtCanvasPainter/qcanvasradialgradient.h>
#include <QtCanvasPainter/qcanvasconicalgradient.h>
#include <QtCanvasPainter/qcanvasboxgradient.h>
#include <QtCanvasPainter/qcanvasboxshadow.h>
#include <QtCanvasPainter/qcanvasimagepattern.h>
#include <QtCanvasPainter/qcanvasgridpattern.h>
#include <QtCanvasPainter/qcanvascustombrush.h>

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
    paths.clear();
    canvasPaths.clear();
    images.clear();
    fonts.clear();
    pixmaps.clear();
    strings.clear();
    brushes.clear();
}

QT_END_NAMESPACE
