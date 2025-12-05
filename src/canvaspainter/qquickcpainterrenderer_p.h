// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCQUICKITEMPAINTER_P_H
#define QCQUICKITEMPAINTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qquickcpainterrenderer.h"
#include "engine/qcpainterengineutils_p.h"
#include "qcdebug_p.h"

QT_BEGIN_NAMESPACE

class QCPainterFactory;
class QRhiCommandBuffer;

class QQuickCPainterRendererPrivate : public QObject
{
    Q_DECLARE_PUBLIC(QQuickCPainterRenderer)
public:
    QQuickCPainterRendererPrivate(QQuickCPainterRenderer *q);
    ~QQuickCPainterRendererPrivate();

    struct ItemData {
        int width = 0;
        int height = 0;
        float devicePixelRatio = 1.0f;
    };

    QQuickCPainterRenderer *q_ptr;

    QQuickWindow *m_window = nullptr;

    bool m_sharedPainter = true;
    QMetaObject::Connection m_sharedPainterNewFrameConn;
    QCPainterFactory *m_factory = nullptr;
    QRhiCommandBuffer *m_currentCb = nullptr;
    QColor m_fillColor = Qt::black; // the default in item is what matters
    ItemData m_itemData;
    bool m_antialiasing = true;
    bool m_highQualityRendering = false;
    bool m_renderedOnce = false;
    QCDrawDebug m_drawDebug;
    QCDebug m_debug;
    QRhi *m_rhi = nullptr;
    QRhiRenderTarget *m_rt = nullptr;
    bool m_initialized = false;
    bool m_synchronized = false;
    bool m_firstRender = true;
    static QAtomicInt m_rendered;
};

QT_END_NAMESPACE

#endif // QCQUICKITEMPAINTER_P_H
