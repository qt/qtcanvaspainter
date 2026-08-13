# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include_guard(GLOBAL)

# For use by Qt modules that need qt_internal_add_resource.
# The implementation is shared with the public command and comes from the
# installed Qt6CanvasPainterMacros.cmake file.
function(qt_internal_add_custom_brush_shaders)
    _qt_internal_add_custom_brush_shaders_impl(${ARGV} _QT_INTERNAL)
    cmake_parse_arguments(PARSE_ARGV 2 arg "" "OUTPUT_TARGETS" "")
    if (arg_OUTPUT_TARGETS)
        set(${arg_OUTPUT_TARGETS} ${${arg_OUTPUT_TARGETS}} PARENT_SCOPE)
    endif()
endfunction()
