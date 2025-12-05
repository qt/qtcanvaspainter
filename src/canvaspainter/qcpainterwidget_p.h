// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCPAINTERWIDGET_P_H
#define QCPAINTERWIDGET_P_H

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

#include <QtWidgets/private/qrhiwidget_p.h>
#include "qcpainterwidget.h"
#include "engine/qcpainterengineutils_p.h"
#include "qcdebug_p.h"
#include <QAtomicInt>

QT_BEGIN_NAMESPACE

class QCPainterFactory;

class QCPainterWidgetPrivate : public QRhiWidgetPrivate
{
    Q_DECLARE_PUBLIC(QCPainterWidget)

public:
    bool m_sharedPainter = true;
    QCPainterFactory *m_factory = nullptr;
    QColor m_fillColor = Qt::black;
    QCDebug m_debug;
    bool m_firstRender = true;
    QRhiCommandBuffer *m_currentCb = nullptr;
    static QAtomicInt m_rendered;
};

QT_END_NAMESPACE

#endif // QCPAINTERWIDGET_P_H
