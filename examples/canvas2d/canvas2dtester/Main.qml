// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Basic

ApplicationWindow {
    id: mainWindow

    // Requested canvas type
    property bool requestCanvas2D: canvasTypeSwitch.checked
    // Selected test case
    property int testCase: -1

    property bool forceRepaint: false

    property real fontSizeSmall: 12
    property real fontSizeLarge: 20
    property real iconSize: 64

    width: 1920 / 2 //1280
    height: 1080 / 2 //720
    visible: true
    title: qsTr("Canvas2D Tester")
    color: "#202020"

    onWidthChanged: forceRepaint = true;
    onHeightChanged: forceRepaint = true;

    Rectangle {
        id: toolbar
        anchors.bottom: parent.bottom
        z: 2
        width: parent.width
        height: 80
        color: "#f0f0f0"
        Row {
            anchors.verticalCenter: parent.verticalCenter
            x: 20
            Item {
                width: 100
                height: parent.height
                Switch {
                    id: canvasTypeSwitch
                    anchors.centerIn: parent
                    checked: true
                    onCheckedChanged: {
                        canvasView.updateBothCanvas();
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.top
                        anchors.verticalCenterOffset: -6
                        font.pixelSize: 12
                        color: "#202020"
                        text: requestCanvas2D ? "Canvas / <b>Canvas2D</b>" : "<b>Canvas</b> / Canvas2D"
                    }
                }
            }
            Item {
                width: 40
                height: parent.height
                Rectangle {
                    width: 2
                    height: 40
                    anchors.centerIn: parent
                    color: "#d0d0d0"
                }
            }
            Switch {
                id: animateSwitch
                anchors.verticalCenter: parent.verticalCenter
                checked: true
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.top
                    anchors.verticalCenterOffset: -6
                    font.pixelSize: 12
                    font.bold: animateSwitch.checked
                    color: "#202020"
                    text: "Animate"
                }
            }
            Item {
                width: 40
                height: parent.height
                Rectangle {
                    width: 2
                    height: 40
                    anchors.centerIn: parent
                    color: "#d0d0d0"
                }
            }
            Slider {
                id: complexitySlider
                anchors.verticalCenter: parent.verticalCenter
                width: 140
                from: 1
                to: 10
                stepSize: 1
                onValueChanged: canvasView.updateCurrentCanvas();
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.top
                    anchors.verticalCenterOffset: -6
                    font.pixelSize: 12
                    color: "#202020"
                    text: "Complexity: " + complexitySlider.value
                }
            }
            Item {
                width: 40
                height: parent.height
                Rectangle {
                    width: 2
                    height: 40
                    anchors.centerIn: parent
                    color: "#d0d0d0"
                }
            }
        }
    }

    Item {
        id: testsView

        readonly property real listItemWidth: 180
        readonly property real listItemHeight: 40

        anchors.fill: parent
        visible: mainWindow.testCase == -1

        ListModel {
            id: testsModel
            ListElement {
                name: "Text Align"
                testId: 0
            }
            ListElement {
                name: "Composite Modes"
                testId: 1
            }
            ListElement {
                name: "Gradients"
                testId: 2
            }
        }

        ListModel {
            id: benchmarksModel
            ListElement {
                name: "Rectangles"
                testId: 100
            }
            ListElement {
                name: "Lines"
                testId: 101
            }
            ListElement {
                name: "Circles"
                testId: 102
            }
            ListElement {
                name: "Clipping"
                testId: 103
            }
            ListElement {
                name: "Line Styles"
                testId: 104
            }
            ListElement {
                name: "Texts"
                testId: 106
            }
            ListElement {
                name: "State Handling"
                testId: 107
            }
            ListElement {
                name: "Images"
                testId: 108
            }
            ListElement {
                name: "Transformations"
                testId: 109
            }
        }

        ListModel {
            id: featuresModel
            ListElement {
                name: "Box Gradient"
                testId: 200
            }
            ListElement {
                name: "Box Shadow"
                testId: 201
            }
            ListElement {
                name: "Color Effects"
                testId: 202
            }
            ListElement {
                name: "Hole Subpaths"
                testId: 203
            }
            ListElement {
                name: "Adjusting Antialias"
                testId: 204
            }
            ListElement {
                name: "Grid Patterns"
                testId: 205
            }
            ListElement {
                name: "Path2D"
                testId: 206
            }
        }

        Component {
            id: listComponent
            Button {
                id: button
                required property string name
                required property int testId

                width: testsView.listItemWidth
                height: testsView.listItemHeight
                background: Rectangle {
                    id: buttonBackground
                    border.width: 0.5
                    border.color: "#d0808080"
                    color: "#d0404040"
                    opacity: button.hovered ? 1.0 : 0.5
                }
                contentItem: Text {
                    anchors.centerIn: parent
                    color: "#f0f0f0"
                    font.pointSize: mainWindow.fontSizeSmall
                    text: button.name
                }

                onClicked: {
                    mainWindow.testCase = button.testId
                    canvasView.updateBothCanvas();
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            spacing: 20

            component HeaderItem : Item {
                id: component
                property string text
                width: testsView.listItemWidth
                height: testsView.listItemHeight
                Text {
                    anchors.centerIn: parent
                    text: component.text
                    color: "#f0f0f0"
                    font.pointSize: mainWindow.fontSizeSmall * 1.2
                    font.bold: true
                }
            }

            Item {
                width: mainWindow.width * 0.3
                height: mainWindow.height
                Text {
                    id: topLabel
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    text: qsTr("Qt Canvas2D Tester")
                    color: "#f0f0f0"
                    font.pointSize: mainWindow.fontSizeLarge
                }
                Text {
                    id: infoLabel
                    anchors.top: topLabel.bottom
                    anchors.topMargin: 20
                    width: parent.width
                    text: qsTr("This example demonstrates the new Canvas2D features, " +
                               "performance and compatibility compared to Quick Canvas. \n\n" +
                               "The painting is done with a matching QML JavaScript " +
                               "code for both Canvas and Canvas2D elements.")
                    wrapMode: Text.Wrap
                    color: "#d0d0d0"
                    font.pointSize: mainWindow.fontSizeSmall
                }
            }

            ListView {
                id: benchmarksListView
                width: testsView.listItemWidth
                height: count * testsView.listItemHeight
                model: benchmarksModel
                delegate: listComponent
                header: HeaderItem {
                    text: "Benchmarks"
                }
            }
            ListView {
                id: examplesListView
                width: testsView.listItemWidth
                height: count * testsView.listItemHeight
                model: testsModel
                delegate: listComponent
                header: HeaderItem {
                    text: "Examples"
                }
            }
            ListView {
                id: featuresListView
                width: testsView.listItemWidth
                height: count * testsView.listItemHeight
                model: featuresModel
                delegate: listComponent
                header: HeaderItem {
                    text: "New Features"
                }
            }
        }
    }

    Button {
        id: backButton
        anchors.left: parent.left
        anchors.top: parent.top
        z: 10
        implicitWidth: mainWindow.iconSize
        implicitHeight: mainWindow.iconSize
        opacity: mainWindow.testCase >= 0
        visible: opacity
        icon.source: "arrow_icon.png"
        icon.width: backButton.width * 0.3
        icon.height: backButton.height * 0.3
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            mainWindow.testCase = -1
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }
    }

    CanvasView {
        id: canvasView
        anchors.top: parent.top
        anchors.bottom: toolbar.top
        width: parent.width
        animate: animateSwitch.checked
    }
}
