// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCQUICKITEM_P_H
#define QCQUICKITEM_P_H

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

#include "qquickcpainteritem.h"
#include <QTimer>
#include <private/qquickrhiitem_p.h>

QT_BEGIN_NAMESPACE

struct QCDebugCounters;

class QQuickCPainterItemPrivate : public QQuickRhiItemPrivate
{
    Q_DECLARE_PUBLIC(QQuickCPainterItem)
public:
    void updateDebugData(const QCDebugCounters &debugCounters);
    void updateDebug();

    QQuickCPainterRenderer *m_renderer = nullptr;
    QColor m_fillColor = Qt::black;
    QVariantMap m_debugData;
    QTimer m_debugUpdateTimer;
    bool m_debugDataChanged = false;
};

QT_END_NAMESPACE

#endif // QCQUICKITEM_P_H
