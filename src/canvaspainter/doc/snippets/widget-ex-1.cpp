// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
class MyWidget : public QCPainterWidget
{
public:
    void paint(QCPainter *p) override
    {
        if (m_image.isNull())
            m_image = p->addImage(QImage("image.png"), QCPainter::ImageFlag::Repeat);

        // ... draw using m_image
    }

    void graphicsResourcesInvalidated() override
    {
        // textures are lost, indicate the need for reload
        m_image = {};
    }

    QCImage m_image;
};
//![0]
