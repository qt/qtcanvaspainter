# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qcmobilewidgetperf LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui GuiPrivate Widgets CanvasPainter)

qt_standard_project_setup(REQUIRES 6.11)

set(QT_ANDROID_DEPLOYMENT_TYPE Debug)

qt_add_executable(qcmobilewidgetperf
    MACOSX_BUNDLE
    main.cpp
    benchwidget.cpp benchwidget.h
)

target_link_libraries(qcmobilewidgetperf PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Widgets
    Qt6::CanvasPainter
)

install(TARGETS qcmobilewidgetperf
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET qcmobilewidgetperf
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
