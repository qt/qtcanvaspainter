# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(CanvasPainter
    SOURCES
        qcpainter.cpp qcpainter.h qcpainter_p.h
        qclineargradient.cpp qclineargradient.h
        qcimagepattern.cpp qcimagepattern.h qcimagepattern_p.h
        qcgridpattern.cpp qcgridpattern.h qcgridpattern_p.h
        qcradialgradient.cpp qcradialgradient.h
        qcboxgradient.cpp qcboxgradient.h
        qcboxshadow.cpp qcboxshadow.h qcboxshadow_p.h
        qcimage.cpp qcimage.h qcimage_p.h
        qcdebug.cpp qcdebug_p.h
        qcbrush.cpp qcbrush.h qcbrush_p.h
        qcgradient.cpp qcgradient.h qcgradient_p.h
        qcconicalgradient.cpp qcconicalgradient.h
        qccustombrush.cpp qccustombrush.h qccustombrush_p.h
        qcpainterpath.cpp qcpainterpath.h qcpainterpath_p.h
        qcoffscreencanvas.cpp qcoffscreencanvas.h qcoffscreencanvas_p.h
        qcpainterfactory.cpp qcpainterfactory.h qcpainterfactory_p.h
        qcrhipaintdriver.cpp qcrhipaintdriver.h qcrhipaintdriver_p.h

        engine/qcpainterengine.cpp engine/qcpainterengine_p.h engine/qcpainterengineutils_p.h
        engine/qcpainterrhirenderer.cpp engine/qcpainterrhirenderer_p.h
        engine/qcdistancefieldglyphcache.cpp engine/qcdistancefieldglyphcache_p.h
        engine/qcrhidistancefieldglyphcache.cpp engine/qcrhidistancefieldglyphcache_p.h
        engine/qcareaallocator.cpp engine/qcareaallocator_p.h
        engine/qctextlayout.cpp engine/qctextlayout_p.h

        qtcanvaspainterglobal.h qtcanvaspainterglobal_p.h

    DEFINES
        QT_BUILD_CANVASPAINTER_LIB
        # QCPAINTER_TRACK_TEXTURE_USAGE
        # QCPAINTER_EQUAL_POINTS_CHECKING_ENABLED
        # QCPAINTER_FAST_BEVEL_CALCULATIONS
        QCPAINTER_CACHE_GLYPH_RUNS
        QCPAINTER_GLYPH_RUNS_CACHE_SIZE=1024
        # QCPAINTER_DISABLE_TEXT_SUPPORT
        NVG_NO_STB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::GuiPrivate
)

qt_internal_extend_target(CanvasPainter CONDITION TARGET Qt::Quick
    SOURCES
        qquickcpainteritem.cpp qquickcpainteritem.h qquickcpainteritem_p.h
        qquickcpainterrenderer.cpp qquickcpainterrenderer.h qquickcpainterrenderer_p.h
    LIBRARIES
        Qt::QuickPrivate
)

qt_internal_extend_target(CanvasPainter CONDITION TARGET Qt::Widgets
    SOURCES
        qcpainterwidget.cpp qcpainterwidget.h qcpainterwidget_p.h
    LIBRARIES
        Qt::WidgetsPrivate
)

qt_internal_add_docs(CanvasPainter
    doc/qtcanvaspainter.qdocconf
)

qc_internal_add_shaders(CanvasPainter "qcshaders"
    PREFIX
        "/qcshaders"
    BASE
        engine
    OUTPUTS
        qcpainter.vert.qsb
        qcpainter.frag.qsb
    FILES
        engine/qcpainter.vert
        engine/qcpainter.frag
)

qc_internal_add_shaders(CanvasPainter "qcshaders_t"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_t.frag.qsb
    DEFINES
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aa"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa.frag.qsb
    DEFINES
        EDGE_AA
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aat"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aat.frag.qsb
    DEFINES
        EDGE_AA
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_sc"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_sc.frag.qsb
    DEFINES
        SCISSORING
)

qc_internal_add_shaders(CanvasPainter "qcshaders_sct"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_sct.frag.qsb
    DEFINES
        SCISSORING
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aass"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_ss.frag.qsb
    DEFINES
        EDGE_AA
        STENCIL_STROKES
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasst"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_sst.frag.qsb
    DEFINES
        EDGE_AA
        STENCIL_STROKES
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasc"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_sc.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasct"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_sct.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasssc"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_ss_sc.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
        STENCIL_STROKES
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasssct"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_ss_sct.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
        STENCIL_STROKES
        TEXT_A
)
