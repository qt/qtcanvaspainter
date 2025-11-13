# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qcgallery LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick ShaderTools CanvasPainter)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(qcgalleryexample
    WIN32
    MACOSX_BUNDLE

    main.cpp

    painting/src/paintingitem.cpp painting/src/paintingitem.h
    painting/src/paintingitemrenderer.cpp painting/src/paintingitemrenderer.h

    mouse_events/src/eventitem.cpp mouse_events/src/eventitem.h
    mouse_events/src/eventitemrenderer.cpp mouse_events/src/eventitemrenderer.h

    qcpainter_features/src/galleryitem.cpp qcpainter_features/src/galleryitem.h
    qcpainter_features/src/galleryitemrenderer.cpp qcpainter_features/src/galleryitemrenderer.h
)

qt_add_resources(qcgalleryexample
    PREFIX
        "/"
    FILES
        main.qml
        back.png
        painting/main.qml
        mouse_events/main.qml
        mouse_events/ToggleButton.qml
        qcpainter_features/main.qml
        qcpainter_features/TopBar.qml
        qcpainter_features/images/pattern1.png
        qcpainter_features/images/pattern2.png
        qcpainter_features/fonts/Pacifico.ttf
        qcpainter_features/images/pattern3.png
        qcpainter_features/images/qt_development_white.png
        qcpainter_features/images/face-smile-bw.png
)

target_link_libraries(qcgalleryexample PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::CanvasPainter
)

qc_add_shaders(qcgalleryexample "qcgallery_custombrush_shaders"
    PREFIX
        "/qcgalleryexample"
    FILES
        qcpainter_features/brush1.frag
        qcpainter_features/brush2.frag
        qcpainter_features/brush3.frag
        qcpainter_features/brush4.frag
        qcpainter_features/brush3.vert
)

install(TARGETS qcgalleryexample
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET qcgalleryexample
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
