# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_baseline_canvaspainting LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    scripts/*
    fonts/* )
list(APPEND test_data ${test_data_glob})
list(APPEND test_data "../data")

set(BASELINE_DIR "../../../../qtbase/tests/baseline/shared")

qt_internal_add_test(tst_baseline_canvaspainting
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        tst_baseline_canvaspainting.cpp
        canvaspainting_cpptests.cpp canvaspainting_cpptests.h
        ${BASELINE_DIR}/baselineprotocol.cpp
        ${BASELINE_DIR}/baselineprotocol.h
        ${BASELINE_DIR}/lookup3.cpp
        ${BASELINE_DIR}/qbaselinetest.cpp
        ${BASELINE_DIR}/qbaselinetest.h
    DEFINES
        QMAKESPEC=""
    INCLUDE_DIRECTORIES
        ${BASELINE_DIR}
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::Network
        Qt::CanvasPainter
        Qt::CanvasPainterPrivate
    TESTDATA ${test_data}
)

add_dependencies(tst_baseline_canvaspainting canvaspainter_qmlscenegrabber)

qt_internal_add_resource(tst_baseline_canvaspainting "resources"
    PREFIX
        "/"
    FILES
        "images/pattern1.png"
        "images/pattern2.png"
        "images/pattern2@2x.png"
        "images/pattern3.png"
        "images/face-smile-bw.png"
        "images/qt_development_white.png"
        "images/qt_development_white@2x.png"
        "images/dome_argb32.png"
        "images/dome_rgb32.png"
        "images/tiger.json"
        "fonts/QtEmojiTestFont-Regular.ttf"
)

qt_add_custom_brush_shaders(tst_baseline_canvaspainting "custombrushes"
    PREFIX
        "/"
    BASE
        "shaders"
    FILES
        "shaders/wobblebrush.vert"
        "shaders/wobblebrush.frag"
        "shaders/glowbrush.frag"
)
