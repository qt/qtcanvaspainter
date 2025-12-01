// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ITEMTEST_H
#define ITEMTEST_H

#include "qquickcpainteritem.h"
#include "qquickcpainterrenderer.h"
#include "../shared/painthelper.h"

class HelloItemRenderer : public QQuickCPainterRenderer
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

    void initializeResources(QCPainter *) override
    {
        qDebug() << "initializeResources" << this;
    }

    void synchronize(QQuickCPainterItem *) override
    {
        //qDebug() << "synchronize";
    }

    void paint(QCPainter *p) override
    {
        // Provide our own QCImage, to verify that a "load-if-not-yet-done"
        // logic works as expected, and it does not break down when the item
        // is moved between windows (and so changes QRhis, losing all graphics
        // resources in the process).
        static QImage logoImage(":/quitlogo.png");
        if (logo.isNull())
            logo = p->addImage(logoImage, QCPainter::ImageFlag::Repeat);

        paintHelloItem(p, width(), height(), &logo);
    }

    QCImage logo;
};

class HelloItem : public QQuickCPainterItem
{
    Q_OBJECT

public:
    HelloItem(QQuickItem *parent = nullptr)
        :  QQuickCPainterItem(parent)
    {
    }

    QQuickCPainterRenderer *createItemRenderer() const override
    {
        return new HelloItemRenderer;
    }
};

#endif // ITEMTEST_H
