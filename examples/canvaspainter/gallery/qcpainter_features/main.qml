// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.4
import GalleryItem 1.0

Item {
    id: mainView

    property real animationTime: 0
    property real animationSine: 0
    property bool animatePainting: true

    NumberAnimation on animationTime {
        id: animationTimeAnimation
        running: mainView.animatePainting
        from: 0
        to: 360
        duration: 1000*360
        loops: Animation.Infinite
    }
    SequentialAnimation on animationSine {
        running: mainView.animatePainting
        loops: Animation.Infinite
        NumberAnimation {
            from: 0
            to: 1
            duration: 1000
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            from: 1
            to: 0
            duration: 1000
            easing.type: Easing.InOutSine
        }
    }

    TopBar {
        id: topBar
        currentIndex: listView.currentIndex
        itemCount: listView.count
    }

/*
    // This can be used for autoscrolling gallery to see memory usage
    // while (un)loading items
    SequentialAnimation {
        running: true
        loops: Animation.Infinite
        ScriptAction {
            script: listView.currentIndex = listView.count-1;
        }
        PauseAnimation {
            duration: 5000
        }
        ScriptAction {
            script: listView.currentIndex = 0;
        }
        PauseAnimation {
            duration: 5000
        }
    }
*/

    ListView {
        id: listView
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        orientation: ListView.Horizontal
        snapMode: ListView.SnapToItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        maximumFlickVelocity: 10000
        highlightMoveDuration: 2500
        model: 13
        delegate: GalleryItem {
            width: listView.width
            height: listView.height
            animationTime: mainView.animationTime
            animationSine: mainView.animationSine
            galleryView: index
            fillColor: "transparent"
            alphaBlending: true
            Connections {
                // Fixing QCPainter issue #22
                target: listView
                function onContentXChanged() {
                    update();
                }
            }
        }
    }
}
