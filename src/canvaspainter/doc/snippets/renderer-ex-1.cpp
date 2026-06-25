// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCanvasPainter/qcanvaspainteritemrenderer.h>
#include <QtCanvasPainter/qcanvasimage.h>
#include <QtGui/qimage.h>

class MyItem : public QCanvasPainterItem {
    Q_OBJECT
public:
    using QCanvasPainterItem::QCanvasPainterItem;
    float value() const { return 0.0f; }
};

//![0]
class MyRenderer : public QCanvasPainterItemRenderer
{
public:
    void synchronizeData(QCanvasPainterItem *item) override
    {
        // copy a custom property value from item in a thread-safe manner
        m_value = static_cast<MyItem *>(item)->value();
    }

    void initializeResources(QCanvasPainter *p) override
    {
        // load assets
        if (m_image.isNull())
            m_image = p->addImage(QImage("image.png"), QCanvasPainter::ImageFlag::Repeat);
    }

    void prePaint(QCanvasPainter *p) override
    {
        // this is where offscreen canvases are drawn into
        if (m_canvas.isNull()) {
            m_canvas = p->createCanvas(QSize(640, 480));
            beginCanvasPainting(m_canvas);
            // ... draw into the offscreen canvas
            endCanvasPainting();
            m_canvasImage = p->addImage(m_canvas);
        }
    }

    void paint(QCanvasPainter *p) override
    {
        QPointF center(width() / 2, height() / 2);
        // ... draw using m_value, m_image, and m_canvasImage
    }

    QCanvasImage m_image;
    QCanvasOffscreenCanvas m_canvas;
    QCanvasImage m_canvasImage;
    float m_value;
};
//![0]
