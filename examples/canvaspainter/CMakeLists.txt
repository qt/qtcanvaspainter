# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

# TODO: enable example, so that the CI builds it, when everything works
# qt_internal_add_example(gallery)

foreach(target IN LISTS reused_dir_targets)
    if(TARGET ${target})
        qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS moc rcc)
        if(TARGET Qt6::Widgets)
            qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS uic)
        endif()
    endif()
endforeach()
