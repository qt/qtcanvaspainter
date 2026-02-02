// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCDEBUG_H
#define QCDEBUG_H

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

#include "engine/qcpainterengineutils_p.h"
#include "qcpainter.h"
#include <QElapsedTimer>
#include <QString>

QT_BEGIN_NAMESPACE

class QCDebugCounterVisualizer
{
public:
    QCDebugCounterVisualizer();
    void start();
    void paint(QCPainter *painter, float width, float height, const QCDebugCounters &debugCounters);

private:
    QElapsedTimer m_debugTimer;
    QElapsedTimer m_debugUpdateTimer;
    qint64 m_debugNsElapsed;
    qint64 m_debugCounter;
    QString m_debugMsElapsed;
};

QT_END_NAMESPACE

#endif // QCDEBUG_H
