// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef IMAGETESTINGRENDERER_H
#define IMAGETESTINGRENDERER_H


#include "qquickcpainterrenderer.h"
#include "qcimage.h"
#include "qcoffscreencanvas.h"
#include <QImage>
#include <QList>
class ImageTestingRenderer : public QQuickCPainterRenderer
{

public:
    explicit ImageTestingRenderer();
    void synchronize(QQuickCPainterItem *item) override;
    void prePaint(QCPainter *painter) override;
    void paint(QCPainter *painter) override;

private:
    struct ImageData {
        QCImage image;
        bool visible = true;
    };
    struct CanvasData {
        QCOffscreenCanvas canvas;
        QCImage image;
    };
    void generateImage();

    QList<ImageData> m_images;
    QList<CanvasData> m_canvases;
    bool m_showGradient = false;
    bool m_animateGradient = false;
    int m_newCanvasPending = 0;
    bool m_reregisterPending = false;
};

#endif // IMAGETESTINGRENDERER_H
