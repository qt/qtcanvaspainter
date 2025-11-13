# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtCanvasPainter
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt CanvasPainter Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui ShaderTools Widgets Quick)

if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()

if(NOT TARGET Qt::ShaderTools)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::ShaderTools\" is not met.")
    return()
endif()

# TODO what if Quick or Widgets are not present?

include(tools/qcshadergen/Qt6CanvasPainterMacros.cmake)

qt_build_repo()
