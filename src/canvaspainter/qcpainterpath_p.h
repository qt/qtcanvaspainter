// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCPAINTERPATH_P_H
#define QCPAINTERPATH_P_H

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

#include "qcpainterpath.h"
#include "engine/qcpainterengineutils_p.h"

QT_BEGIN_NAMESPACE

class QCPainterPathPrivate
{
    Q_DECLARE_PUBLIC(QCPainterPath)
public:
    QCPainterPathPrivate(QCPainterPath *q);
    QCPainterPath *q_ptr = nullptr;

    void appendCommand(QCCommand command);
    void appendCommands(const QCCommand commands[], int cCount);
    void appendCommandsData(const float commandsData[], int dCount);
    void ensureCommands(int addition);
    void ensureCommandsData(int addition);

    QCCommands commands;
    QCCommandsData commandsData;
    int commandsCount = 0;
    int commandsDataCount = 0;
    // Distance tolerance for consecutive points
    float distTol = 0.01f;
    // When this increases, path has been (re)created
    // so all commands need to be re-evaluated.
    int pathIterations = 0;
};

QT_END_NAMESPACE

#endif // QCPAINTERPATH_P_H
