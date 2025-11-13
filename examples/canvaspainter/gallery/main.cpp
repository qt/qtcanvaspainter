// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "qcpainter_features/src/galleryitem.h"
#include "mouse_events/src/eventitem.h"
#include "painting/src/paintingitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    qmlRegisterType<GalleryItem>("GalleryItem", 1, 0, "GalleryItem");
    qmlRegisterType<EventItem>("EventItem", 1, 0, "EventItem");
    qmlRegisterType<PaintingItem>("PaintingItem", 1, 0, "PaintingItem");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
