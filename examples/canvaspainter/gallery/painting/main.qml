// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import PaintingItem

Item {
    id: mainView

    property int margin: Math.min(width, height)*0.02

    Item {
        id: topArea
        width: parent.width
        height: mainWindow.topbarHeight
        Text {
            anchors.centerIn: parent
            font.pixelSize: 20 * dp
            color: "#f0f0f0"
            text: "Just draw below..."
        }
    }

    PaintingItem {
        id: paintingItem
        anchors.top: topArea.bottom
        anchors.topMargin: margin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: margin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - margin*2
        mouseEventsEnabled: true
    }

}
