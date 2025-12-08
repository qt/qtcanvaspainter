// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WIDGETTEST_H
#define WIDGETTEST_H

#include "qcpainterwidget.h"
#include "qcpainter.h"
#include "../shared/painthelper.h"

class HelloWidget : public QCPainterWidget
{

public:
    HelloWidget()
    {
        setFillColor("#000000");
    }

    void initializeResources(QCPainter *p) override
    {
        // Provide our own QCImage, to verify that a "load-if-not-yet-done"
        // logic works as expected, and it does not break down when the widget
        // is moved between windows (and so changes QRhis, losing all graphics
        // resources in the process).
        static QImage logoImage(":/quitlogo.png");
        if (logo.isNull())
            logo = p->addImage(logoImage, QCPainter::ImageFlag::Repeat);
    }

    void paint(QCPainter *p) override
    {
        paintHelloItem(p, width(), height(), &logo);
    }

    QCImage logo;

    void graphicsResourcesInvalidated() override
    {
        qWarning("graphicsResourcesInvalidated");

        logo = {}; // textures are lost, indicate the need for reload
    }
};

#endif // WIDGETTEST_H
