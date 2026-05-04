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
    // We own all the brushes.
    qDeleteAll(brushes);
    brushes.clear();
}

// Creates and returns a new brush with the type and content of \a brush.
QCanvasBrush *QCanvas2DCommandBuffer::copyBrush(QCanvasBrush *brush)
{
    switch (brush->type()) {
    case QCanvasBrush::BrushType::LinearGradient:
    {
        auto *b = static_cast<QCanvasLinearGradient *>(brush);
        return new QCanvasLinearGradient(*b);
        break;
    }
    case QCanvasBrush::BrushType::RadialGradient:
    {
        auto *b = static_cast<QCanvasRadialGradient *>(brush);
        return new QCanvasRadialGradient(*b);
        break;
    }
    case QCanvasBrush::BrushType::ConicalGradient:
    {
        auto *b = static_cast<QCanvasConicalGradient *>(brush);
        return new QCanvasConicalGradient(*b);
        break;
    }
    case QCanvasBrush::BrushType::BoxGradient:
    {
        auto *b = static_cast<QCanvasBoxGradient *>(brush);
        return new QCanvasBoxGradient(*b);
        break;
    }
    case QCanvasBrush::BrushType::BoxShadow:
    {
        auto *b = static_cast<QCanvasBoxShadow *>(brush);
        return new QCanvasBoxShadow(*b);
        break;
    }
    case QCanvasBrush::BrushType::ImagePattern:
    {
        auto *b = static_cast<QCanvasImagePattern *>(brush);
        auto *ip = new QCanvasImagePattern(*b);
        ip->setImage(b->image());
        return ip;
        break;
    }
    case QCanvasBrush::BrushType::GridPattern:
    {
        auto *b = static_cast<QCanvasGridPattern *>(brush);
        return new QCanvasGridPattern(*b);
        break;
    }
    case QCanvasBrush::BrushType::Custom:
    {
        auto *b = static_cast<QCanvasCustomBrush *>(brush);
        return new QCanvasCustomBrush(*b);
        break;
    }
    case QCanvasBrush::BrushType::Invalid:
    default:
        break;
    }
    return nullptr;
}

QT_END_NAMESPACE
