# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(canvaspainter_snippets LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS CanvasPainter Gui GuiPrivate Widgets)

add_library(canvaspainter_snippets OBJECT
    item-ex-1.cpp
    paintdriver-ex-1.cpp
    renderer-ex-1.cpp
    widget-canvas-ex-1.cpp
    widget-ex-1.cpp
)

target_link_libraries(canvaspainter_snippets PRIVATE
    Qt::CanvasPainter
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
)
