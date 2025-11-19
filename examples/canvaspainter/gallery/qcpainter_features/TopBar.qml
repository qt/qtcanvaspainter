// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0

Item {
    id: root

    property int currentIndex: 0
    property int itemCount: 0
    property string title

    onCurrentIndexChanged: {
        switch (root.currentIndex) {
        case 0:
            title = "Colors and Brushes";
            break;
        case 1:
            title = "Paths, Caps & Joins";
            break;
        case 2:
            title = "Painter Paths";
            break;
        case 3:
            title = "States, Transitions and Clipping";
            break;
        case 4:
            title = "Antialiasing and Line Width";
            break;
        case 5:
            title = "Composite Modes";
            break;
        case 6:
            title = "Grid Patterns";
            break;
        case 7:
            title = "Shadows";
            break;
        case 8:
            title = "Custom Brushes";
            break;
        case 9:
            title = "Text: Fonts and Styles";
            break;
        case 10:
            title = "Text: Brushes";
            break;
        case 11:
            title = "Text: Alignment";
            break;
        case 12:
            title = "Text: Wrapping";
            break;
        case 13:
            title = "Images";
            break;
        case 14:
            title = "Frame Buffers";
            break;
        }
    }

    width: parent.width
    height: mainWindow.topbarHeight

    /*Rectangle {
        anchors.fill: parent
        color: "#202020"
        //opacity: 0.2
    }*/
    /*Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: titleTextItem.top
        font.pixelSize: 12 * dp
        color: "#909090"
        text: "QCPainter Gallery"
    }*/
    Text {
        id: titleTextItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -8 * dp
        font.pixelSize: 20 * dp
        color: "#f0f0f0"
        text: title
    }
    Row {
        anchors.top: titleTextItem.bottom
        anchors.topMargin: 4 * dp
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 4 * dp
        Repeater {
            model: root.itemCount
            Rectangle {
                width: 8 * dp
                height: width
                radius: width/2
                color: "#e0e0e0"
                border.width: 1
                border.color: "#000000"
                opacity: index == root.currentIndex ? 0.8 : 0.2
                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }
}

