# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(gallery)
qt_internal_add_example(compacthealth)

foreach(target IN LISTS reused_dir_targets)
    if(TARGET ${target})
        qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS moc rcc)
        if(TARGET Qt6::Widgets)
            qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS uic)
        endif()
    endif()
endforeach()
