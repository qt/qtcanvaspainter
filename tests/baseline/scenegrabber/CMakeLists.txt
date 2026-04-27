# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(canvaspainter_qmlscenegrabber LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_executable(canvaspainter_qmlscenegrabber
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::Quick
        Qt::CanvasPainter
)

# Turn it into a QML module which has no plugin (i.e. just a backing target)
qt6_add_qml_module(canvaspainter_qmlscenegrabber
    URI "QtCanvasPainter.Lancelot"
    VERSION "${PROJECT_VERSION}"
    NO_GENERATE_PLUGIN_SOURCE
    RESOURCE_PREFIX "/"
)
