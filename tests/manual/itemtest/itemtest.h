// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ITEMTEST_H
#define ITEMTEST_H

#include "qcanvaspainteritem.h"
#include "qcanvaspainteritemrenderer.h"
#include "../shared/painthelper.h"

class HelloItemRenderer : public QCanvasPainterItemRenderer
{

public:
    HelloItemRenderer()
    {
        // why this is interesting: because moving an item to a new QQuickWindow
        // automatically destroys and creates a new renderer object, thus
        // ensuring that no graphicsResourcesInvalidated() and similar are
        // needed, unlike the widget that has no dedicated renderer object.
        qDebug() << "HelloItemRenderer" << this << "created";
    }

    void initializeResources(QCanvasPainter *p) override
    {
        qDebug() << "initializeResources" << this;

        // Provide our own QCanvasImage, to verify that a "load-if-not-yet-done"
        // logic works as expected, and it does not break down when the item
        // is moved between windows (and so changes QRhis, losing all graphics
        // resources in the process).
        static QImage logoImage(":/quitlogo.png");
        if (logo.isNull())
            logo = p->addImage(logoImage, QCanvasPainter::ImageFlag::Repeat);
    }

    void synchronize(QCanvasPainterItem *) override
    {
        //qDebug() << "synchronize";
    }

    void paint(QCanvasPainter *p) override
    {
        paintHelloItem(p, width(), height(), &logo);
    }

    QCanvasImage logo;
};

class HelloItem : public QCanvasPainterItem
{
    Q_OBJECT

public:
    HelloItem(QQuickItem *parent = nullptr)
        :  QCanvasPainterItem(parent)
    {
    }

    QCanvasPainterItemRenderer *createItemRenderer() const override
    {
        return new HelloItemRenderer;
    }
};

#endif // ITEMTEST_H
