// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCOFFSCREENCANVAS_P_H
#define QCOFFSCREENCANVAS_P_H

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

#include <QtCore/qshareddata.h>
#include <QtCore/qsize.h>
#include "qcoffscreencanvas.h"
#include "engine/qcpainterrhirenderer_p.h"

QT_BEGIN_NAMESPACE

class QCOffscreenCanvasPrivate : public QSharedData
{
public:
    static QCOffscreenCanvasPrivate *get(QCOffscreenCanvas *canvas) { return canvas->d.get(); }
    static const QCOffscreenCanvasPrivate *get(const QCOffscreenCanvas *canvas) { return canvas->d.get(); }

    QCRhiCanvas rhiCanvas;
    QColor fillColor = {0, 0, 0, 0};
};

QT_END_NAMESPACE

#endif // QCOFFSCREENCANVAS_P_H
