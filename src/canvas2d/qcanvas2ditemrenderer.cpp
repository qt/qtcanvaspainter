// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2ditemrenderer_p.h"
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

QCanvas2DItemRenderer::QCanvas2DItemRenderer()
{
}

QCanvas2DItemRenderer::~QCanvas2DItemRenderer()
{
    if (m_painter) {
        for (auto n : std::as_const(qcImages))
            m_painter->removeImage(n);
    }
    qcImages.clear();
}

void QCanvas2DItemRenderer::synchronizeData(QCanvasPainterItem *item)
{
    Q_ASSERT(item);
    QCanvas2DItem *realItem = static_cast<QCanvas2DItem*>(item);

    imageData = realItem->imageData();

    if (auto ccb = realItem->ccb()) {
        commands = ccb->commands;
        ints = ccb->ints;
        bools = ccb->bools;
        reals = ccb->reals;
        rects = ccb->rects;
        colors = ccb->colors;
        matrixes = ccb->matrixes;
        paths = ccb->paths;
        canvasPaths = ccb->canvasPaths;
        images = ccb->images;
        fonts = ccb->fonts;
        pixmaps = ccb->pixmaps;
        strings = ccb->strings;

        copyBrushes(ccb->brushes);

        // At this point ccb can be cleared
        ccb->clearBuffers();
    }

    // Clear per-frame image data cache
    realItem->clearImageDataCache();
}

void QCanvas2DItemRenderer::paint(QCanvasPainter *painter)
{
    m_painter = painter;

    // Add images as needed.
    for (const auto &iData : std::as_const(imageData))
        qcImages[iData.url] = painter->addImage(iData.image, iData.flags);

    reset();

    while (hasNext()) {
        QCanvas2DContext::PaintCommand cmd = takeNextCommand();
        switch (cmd) {
        case QCanvas2DContext::BeginPath:
        {
            m_painter->beginPath();
            break;
        }
        case QCanvas2DContext::BeginSolidSubPath:
        {
            m_painter->beginSolidSubPath();
            break;
        }
        case QCanvas2DContext::BeginHoleSubPath:
        {
            m_painter->beginHoleSubPath();
            break;
        }
        case QCanvas2DContext::ClosePath:
        {
            m_painter->closePath();
            break;
        }
        case QCanvas2DContext::MoveTo:
        {
            auto x = takeReal();
            auto y = takeReal();
            m_painter->moveTo(x, y);
            break;
        }
        case QCanvas2DContext::LineTo:
        {
            auto x = takeReal();
            auto y = takeReal();
            m_painter->lineTo(x, y);
            break;
        }
        case QCanvas2DContext::QuadraticCurveTo:
        {
            auto cpx = takeReal();
            auto cpy = takeReal();
            auto x = takeReal();
            auto y = takeReal();
            m_painter->quadraticCurveTo(cpx, cpy, x, y);
            break;
        }
        case QCanvas2DContext::BezierCurveTo:
        {
            auto cp1x = takeReal();
            auto cp1y = takeReal();
            auto cp2x = takeReal();
            auto cp2y = takeReal();
            auto x = takeReal();
            auto y = takeReal();
            m_painter->bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
            break;
        }
        case QCanvas2DContext::ArcTo:
        {
            auto x1 = takeReal();
            auto y1 = takeReal();
            auto x2 = takeReal();
            auto y2 = takeReal();
            auto radius = takeReal();
            m_painter->arcTo(x1, y1, x2, y2, radius);
            break;
        }
        case QCanvas2DContext::Rect:
        {
            auto x = takeReal();
            auto y = takeReal();
            auto w = takeReal();
            auto h = takeReal();
            m_painter->rect(x, y, w, h);
            break;
        }
        case QCanvas2DContext::RoundRect:
        {
            auto x = takeReal();
            auto y = takeReal();
            auto w = takeReal();
            auto h = takeReal();
            auto radiusTopLeft = takeReal();
            auto radiusTopRight = takeReal();
            auto radiusBottomRight = takeReal();
            auto radiusBottomLeft = takeReal();
            if (radiusTopRight >= 0) {
                m_painter->roundRect(x, y, w, h,
                                     radiusTopLeft,
                                     radiusTopRight,
                                     radiusBottomRight,
                                     radiusBottomLeft);
            } else {
                // Use single radius for all corners
                m_painter->roundRect(x, y, w, h,
                                     radiusTopLeft);
            }
            break;
        }
        case QCanvas2DContext::Ellipse:
        {
            auto cX = takeReal();
            auto cY = takeReal();
            auto rX = takeReal();
            auto rY = takeReal();
            m_painter->ellipse(cX, cY, rX, rY);
            break;
        }
        case QCanvas2DContext::EllipseRect:
        {
            auto x = takeReal();
            auto y = takeReal();
            auto w = takeReal();
            auto h = takeReal();
            m_painter->ellipse(QRectF(x, y, w, h));
            break;
        }
        case QCanvas2DContext::Circle:
        {
            auto cX = takeReal();
            auto cY = takeReal();
            auto r = takeReal();
            m_painter->circle(cX, cY, r);
            break;
        }
        case QCanvas2DContext::Arc:
        {
            auto xc = takeReal();
            auto yc = takeReal();
            auto radius = takeReal();
            auto sar = takeReal();
            auto ear = takeReal();
            bool antiClockWise = takeBool();
            auto direction = antiClockWise ? QCanvasPainter::PathWinding::CounterClockWise :
                    QCanvasPainter::PathWinding::ClockWise;
            m_painter->arc(xc, yc, radius, sar, ear, direction);
            break;
        }
        case QCanvas2DContext::FillRect:
        {
            QRectF r = takeRect();
            painter->fillRect(r);
            break;
        }
        case QCanvas2DContext::StrokeRect:
        {
            QRectF r = takeRect();
            painter->strokeRect(r);
            break;
        }
        case QCanvas2DContext::ClearRect:
        {
            QRectF r = takeRect();
            painter->clearRect(r);
            break;
        }
        case QCanvas2DContext::StrokeColor:
        {
            m_state.strokeColor = takeColor();
            painter->setStrokeStyle(m_state.strokeColor);
            break;
        }
        case QCanvas2DContext::FillColor:
        {
            m_state.fillColor = takeColor();
            painter->setFillStyle(m_state.fillColor);
            break;
        }
        case QCanvas2DContext::StrokeStyle:
        {
            m_state.strokeStyle = takeBrush();
            setPaintStyle(m_state.strokeStyle, false);
            break;
        }
        case QCanvas2DContext::FillStyle:
        {
            m_state.fillStyle = takeBrush();
            setPaintStyle(m_state.fillStyle, true);
            break;
        }

        case QCanvas2DContext::Fill:
        {
            m_painter->fill();
            break;
        }
        case QCanvas2DContext::FillPath:
        {
            auto path = takePath();
            path.closeSubpath();
            fillPath(path);
            break;
        }
        case QCanvas2DContext::FillCanvasPath:
        {
            const QCanvasPath &p = takeCanvasPath();
            int pathGroup = takeInt();
            m_painter->fill(p, pathGroup);
            break;
        }
        case QCanvas2DContext::Stroke:
        {
            m_painter->stroke();
            break;
        }
        case QCanvas2DContext::StrokePath:
        {
            auto p = takePath();
            strokePath(p);
            break;
        }
        case QCanvas2DContext::StrokeCanvasPath:
        {
            const QCanvasPath &p = takeCanvasPath();
            int pathGroup = takeInt();
            m_painter->stroke(p, pathGroup);
            break;
        }
        case QCanvas2DContext::AddCanvasPath:
        {
            const QCanvasPath &p = takeCanvasPath();
            const QTransform t = takeMatrix();
            m_painter->addPath(p, t);
            break;
        }
        case QCanvas2DContext::AddCanvasPathRange:
        {
            const QCanvasPath &p = takeCanvasPath();
            const QTransform t = takeMatrix();
            int start = takeInt();
            int count = takeInt();
            m_painter->addPath(p, start, count, t);
            break;
        }
        case QCanvas2DContext::Clip:
        {
            m_state.clip = takeBool();
            m_state.clipPath = takePath();
            if (m_state.clip)
                m_painter->setClipRect(m_state.clipPath.controlPointRect());
            else
                m_painter->resetClipping();
            break;
        }
        case QCanvas2DContext::ClipRect:
        {
            QRectF r = takeRect();
            painter->setClipRect(r);
            break;
        }
        case QCanvas2DContext::ResetClipping:
        {
            painter->resetClipping();
            break;
        }
        case QCanvas2DContext::GlobalAlpha:
        {
            m_state.globalAlpha = takeReal();
            m_painter->setGlobalAlpha(m_state.globalAlpha);
            break;
        }
        case QCanvas2DContext::GlobalBrightness:
        {
            m_state.globalBrightness = takeReal();
            m_painter->setGlobalBrightness(m_state.globalBrightness);
            break;
        }
        case QCanvas2DContext::GlobalContrast:
        {
            m_state.globalContrast = takeReal();
            m_painter->setGlobalContrast(m_state.globalContrast);
            break;
        }
        case QCanvas2DContext::GlobalSaturate:
        {
            m_state.globalSaturate = takeReal();
            m_painter->setGlobalSaturate(m_state.globalSaturate);
            break;
        }
        case QCanvas2DContext::GlobalCompositeOperation:
        {
            m_state.globalCompositeOperation = static_cast<QCanvasPainter::CompositeOperation>(takeInt());
            m_painter->setGlobalCompositeOperation(m_state.globalCompositeOperation);
            break;
        }
        case QCanvas2DContext::LineWidth: {
            m_state.lineWidth = takeReal();
            m_painter->setLineWidth(m_state.lineWidth);
            break;
        }
        case QCanvas2DContext::LineCap:
        {
            m_state.lineCap = takeLineCap();
            m_painter->setLineCap(m_state.lineCap);
            break;
        }
        case QCanvas2DContext::LineJoin:
        {
            m_state.lineJoin = takeLineJoin();
            m_painter->setLineJoin(m_state.lineJoin);
            break;
        }
        case QCanvas2DContext::MiterLimit:
        {
            m_state.miterLimit = takeReal();
            m_painter->setMiterLimit(m_state.miterLimit);
            break;
        }
        case QCanvas2DContext::TextAlign:
        {
            auto a = takeAlign();
            m_painter->setTextAlign(a);
            break;
        }
        case QCanvas2DContext::TextBaseline:
        {
            auto b = takeBaseline();
            m_painter->setTextBaseline(b);
            break;
        }
        case QCanvas2DContext::Font:
        {
            if (fonts.size() <= fontIdx) {
                qDebug() << "No font available!";
                break;
            }
            const QFont font = takeFont();
            m_painter->setFont(font);
            break;
        }
        case QCanvas2DContext::FillText:
        {
            QString text = takeString();
            auto x = takeReal();
            auto y = takeReal();
            m_painter->fillText(text, x, y);
            break;
        }
        case QCanvas2DContext::DrawPixmap:
        {
            // TODO: Add also simpler version without sr & dr
            QRectF sr = takeRect();
            QRectF dr = takeRect();
            auto pix = takePixmap();
            auto filename = takeString();
            Q_ASSERT(!pix.isNull());
            drawImage(pix->image(), filename, sr, dr);
            break;
        }
        case QCanvas2DContext::Scale:
        {
            auto x = takeReal();
            auto y = takeReal();
            m_painter->scale(x, y);
            break;
        }
        case QCanvas2DContext::Rotate:
        {
            auto angle = takeReal();
            m_painter->rotate(angle);
            break;
        }
        case QCanvas2DContext::Shear: // TODO: Should this and the command be called "skew"?
        {
            auto h = takeReal();
            auto v = takeReal();
            m_painter->skew(h, v);
            break;
        }
        case QCanvas2DContext::Translate:
        {
            auto x = takeReal();
            auto y = takeReal();
            m_painter->translate(x, y);
            break;
        }
        case QCanvas2DContext::Transform:
        {
            const QTransform t = takeMatrix();
            m_painter->transform(t);
            break;
        }
        case QCanvas2DContext::SetTransform:
        {
            const QTransform t = takeMatrix();
            m_painter->setTransform(t);
            break;
        }
        case QCanvas2DContext::Save:
        {
            m_painter->save();
            break;
        }
        case QCanvas2DContext::Restore:
        {
            m_painter->restore();
            break;
        }
        case QCanvas2DContext::Reset:
        {
            m_painter->reset();
            break;
        }
        case QCanvas2DContext::DrawBoxShadow:
        {
            const auto &brush = takeBrush();
            if (brush.type() == QCanvasBrush::BrushType::BoxShadow)
                m_painter->drawBoxShadow(brush.as<QCanvasBoxShadow>());
            break;
        }
        case QCanvas2DContext::Antialias: {
            m_state.antialias = takeReal();
            m_painter->setAntialias(m_state.antialias);
            break;
        }
        default:
            qWarning() << "Unhandled command:" << cmd;
        }
    }
    //m_state.clip = false;

    Q_EMIT painted();
}

void QCanvas2DItemRenderer::strokePath(const QPainterPath &path)
{
    m_painter->beginPath();
    m_painter->addPath(path);
    m_painter->stroke();
}

void QCanvas2DItemRenderer::fillPath(const QPainterPath &path)
{
    m_painter->beginPath();
    m_painter->addPath(path);
    m_painter->fill();
}

void QCanvas2DItemRenderer::reset()
{
    cmdIdx = 0;
    intIdx = 0;
    boolIdx = 0;
    realIdx = 0;
    rectIdx = 0;
    colorIdx = 0;
    matrixIdx = 0;
    brushIdx = 0;
    pathIdx = 0;
    canvasPathIdx = 0;
    imageIdx = 0;
    fontIdx = 0;
    pixmapIdx = 0;
    stringIdx = 0;
}

// Copies brushes from buffer locally as buffer can remove its
// buffers at any time.
void QCanvas2DItemRenderer::copyBrushes(const QList<QCanvasBrush> &newBrushes)
{
    brushes = newBrushes;
}

void QCanvas2DItemRenderer::setPaintStyle(const QCanvasBrush &brush, bool fill)
{
    if (brush.type() == QCanvasBrush::BrushType::Invalid)
        return;

    if (brush.type() == QCanvasBrush::BrushType::ImagePattern) {
        // Image patterns require setting the matching QCanvasImage.
        auto b = brush.as<QCanvasImagePattern>();
        for (const auto &iData : std::as_const(imageData)) {
            if (b.serialNumber() == iData.pattern.serialNumber()) {
                auto qcImage = getCachedImage(iData.image, iData.url, iData.flags);
                b.setImage(qcImage);
                if (fill)
                    m_painter->setFillStyle(b);
                else
                    m_painter->setStrokeStyle(b);
                break;
            }
        }
    } else {
        if (fill)
            m_painter->setFillStyle(brush);
        else
            m_painter->setStrokeStyle(brush);
    }
}

QCanvasImage QCanvas2DItemRenderer::getCachedImage(const QImage &image, const QString &url, QCanvasPainter::ImageFlags flags)
{
    if (!qcImages.contains(url))
        qcImages[url] = m_painter->addImage(image, flags);

    return qcImages.value(url);
}

void QCanvas2DItemRenderer::drawImage(const QImage &image, const QString &url, const QRectF &sr, const QRectF &dr)
{
    QCanvasPainter::ImageFlags imageFlags;
    auto qcImage = getCachedImage(image, url, imageFlags);
    m_painter->drawImage(qcImage, sr, dr);
}

QT_END_NAMESPACE
