// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DSVGPARSER_P_H
#define QCANVAS2DSVGPARSER_P_H

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

#include <QtCore/qstring.h>
#include <QtGui/qpainterpath.h>

QT_BEGIN_NAMESPACE

namespace QCanvas2DSvgParser
{
    bool parsePathDataFast(const QString &dataStr, QPainterPath &path);
    void pathArc(QPainterPath &path, qreal rx, qreal ry, qreal x_axis_rotation,
                 int large_arc_flag, int sweep_flag, qreal x, qreal y, qreal curx,
                 qreal cury);
}

QT_END_NAMESPACE

#endif // QCANVAS2DSVGPARSER_P_H
