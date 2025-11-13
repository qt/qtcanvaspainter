// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCPAINTERFACTORY_P_H
#define QCPAINTERFACTORY_P_H

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

#include "qcpainterfactory.h"
#include "qcrhipaintdriver.h"
#include "engine/qcpainterrhirenderer_p.h"

QT_BEGIN_NAMESPACE

class QCPainterFactoryPrivate
{
public:
    static const QCPainterFactoryPrivate *get(const QCPainterFactory *obj) { return obj->d; }
    static QCPainterFactoryPrivate *get(QCPainterFactory *obj) { return obj->d; }

    std::unique_ptr<QCRhiPaintDriver> paintDriver;
    std::unique_ptr<QCPainter> painter;
    QCPainterRhiRenderer renderer;
};

QT_END_NAMESPACE

#endif // QCPAINTERFACTORY_P_H
