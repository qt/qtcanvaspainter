// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERFACTORY_P_H
#define QCANVASPAINTERFACTORY_P_H

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

#include "qcanvaspainterfactory.h"
#include "qcanvasrhipaintdriver.h"
#include "engine/qcpainterrhirenderer_p.h"

QT_BEGIN_NAMESPACE

class QCanvasPainterFactoryPrivate
{
public:
    static const QCanvasPainterFactoryPrivate *get(const QCanvasPainterFactory *obj) { return obj->d.get(); }
    static QCanvasPainterFactoryPrivate *get(QCanvasPainterFactory *obj) { return obj->d.get(); }

    std::unique_ptr<QCanvasRhiPaintDriver> paintDriver;
    std::unique_ptr<QCanvasPainter> painter;
    QCPainterRhiRenderer renderer;
    // Set when an item or widget sharing this factory has rendered since the
    // last resetForNewFrame(). Per factory, i.e. per QRhi, so that multiple
    // windows do not consume each other's reset. Only used on the thread of
    // the QRhi.
    bool renderedSinceNewFrame = false;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERFACTORY_P_H
