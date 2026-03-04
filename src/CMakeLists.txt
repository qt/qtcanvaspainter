# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::Gui AND TARGET Qt::qsb)
    add_subdirectory(canvaspainter)
    if(TARGET Qt::Quick)
        add_subdirectory(canvas2d)
    endif()
endif()
