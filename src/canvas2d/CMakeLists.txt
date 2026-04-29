# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(Canvas2DPrivate
    URI "QtCanvas2D"
    TARGET_PRODUCT "Qt Canvas2D (Qt $$QT_VERSION)"
    TARGET_DESCRIPTION "Qt Canvas2D component for Qt Quick."
    VERSION "${PROJECT_VERSION}"
    PLUGIN_TARGET canvas2dplugin
    CLASS_NAME QtCanvas2DPlugin
    INTERNAL_MODULE

    DEPENDENCIES
        QtQuick
        QtCanvasPainter
    SOURCES
        qtcanvas2dglobal_p.h

        qcanvas2ditem.cpp qcanvas2ditem_p.h
        qcanvas2ditemrenderer.cpp qcanvas2ditemrenderer_p.h
        qcanvas2dcontext.cpp qcanvas2dcontext_p.h
        qcanvas2dcommandbuffer.cpp qcanvas2dcommandbuffer_p.h
        qcanvas2dutils.cpp qcanvas2dutils_p.h
    DEFINES
        QT_NO_FOREACH
        QT_NO_INTEGER_EVENT_COORDINATES
        QT_NO_URL_CAST_FROM_STRING
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickPrivate
        Qt::CanvasPainterPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::CanvasPainter
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickPrivate
        Qt::CanvasPainterPrivate
)
