// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import GalleryItem 1.0

Window {
    id: mainView

    readonly property real dp: Math.min(width, height) / 400
    property real animationTime: 0
    property real animationSine: 0
    property bool animatePainting: true

    width: 480
    height: 800
    visible: true
    color: "#222831"

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
        width: parent.width
        height: Math.floor(60 * dp)
        currentIndex: listView.currentIndex
        itemCount: listView.count
        visibilityState: listView.contentX / listView.width
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
        anchors.topMargin: 20 * dp
        anchors.bottom: parent.bottom
        width: parent.width
        orientation: ListView.Horizontal
        snapMode: ListView.SnapToItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        maximumFlickVelocity: 10000
        highlightMoveDuration: 2500
        model: 14
        // Disable this to not preload all views.
        cacheBuffer: 10000
        delegate: GalleryItem {
            readonly property bool animationsOn: index >= listView.currentIndex - 1 &&
                                                 index <= listView.currentIndex + 1
            width: listView.width
            height: listView.height
            animationTime: animationsOn ? mainView.animationTime : 0
            animationSine: animationsOn ? mainView.animationSine : 0
            galleryView: index
            fillColor: "transparent"
            alphaBlending: true
        }
    }
}
