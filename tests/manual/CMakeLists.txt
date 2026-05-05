# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(hellorhi)
add_subdirectory(hellorhi2)
add_subdirectory(widgettest)
add_subdirectory(itemtest)
if(TARGET Qt::OpenGLWidgets)
    add_subdirectory(2dpainting_port)
endif()
add_subdirectory(rendernode)
add_subdirectory(imagetester)
add_subdirectory(qcpainterbench)
