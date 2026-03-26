cmake_minimum_required(VERSION 3.16)

project(canvas2dtester VERSION 0.1 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Quick CanvasPainter)

qt_standard_project_setup(REQUIRES 6.12)

qt_add_executable(canvas2dtesterexample
    WIN32
    MACOSX_BUNDLE
    main.cpp
)

qt_add_qml_module(canvas2dtesterexample
    URI Canvas2DTester
    QML_FILES
        Main.qml
        CanvasView.qml
    RESOURCE_PREFIX
        /
    RESOURCES
        qtlogo.png
        face-smile.png
        arrow_icon.png
    NO_RESOURCE_TARGET_PATH
)

target_link_libraries(canvas2dtesterexample PRIVATE
    Qt6::Quick
    Qt6::CanvasPainter
)

install(TARGETS canvas2dtesterexample
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET canvas2dtesterexample
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
