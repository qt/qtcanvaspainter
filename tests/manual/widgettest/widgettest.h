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

    void paint(QCPainter *p) override
    {
        // Painting code is shared as it's identical in all hello* examples
        paintHelloItem(p, width(), height());
    }

    void graphicsResourcesInvalidated() override
    {
        qWarning("graphicsResourcesInvalidated");
    }
};

#endif // WIDGETTEST_H
