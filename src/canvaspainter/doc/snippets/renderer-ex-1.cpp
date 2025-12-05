// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
class MyRenderer : public QQuickCPainterRenderer
{
public:
    void synchronize(QQuickCPainterItem *item) override
    {
        // copy a custom property value from item in a thread-safe manner
        m_value = static_cast<MyItem *>(item)->value();
    }

    void initializeResources(QCPainter *p) override
    {
        // load assets
        if (m_image.isNull())
            m_image = p->addImage(QImage("image.png"), QCPainter::ImageFlag::Repeat);
    }

    void prePaint(QCPainter *p) override
    {
        // this is where offscreen canvases are drawn into
        if (m_canvas.isNull()) {
            m_canvas = p->createCanvas(QSize(640, 480));
            beginCanvasPainting(m_canvas);
            // ... draw into the offscreen canvas
            endCanvasPainting(m_canvas);
            m_canvasImage = p->addImage(m_canvas);
        }
    }

    void paint(QCPainter *p) override
    {
        QPointF center(width() / 2, height() / 2);
        // ... draw using m_value, m_image, and m_canvasImage
    }

    QCImage m_image;
    QCOffscreenCanvas m_canvas;
    QCImage m_canvasImage;
    float m_value;
};
//![0]
