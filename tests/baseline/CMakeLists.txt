# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Special case: test includes the QBaselineTest module sources from qtbase directly
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../qtbase/tests/baseline/shared")
    add_subdirectory(scenegrabber)
    if(TARGET Qt::Network)
        add_subdirectory(canvaspainting)
    endif()
endif()
