// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DUTILS_P_H
#define QCANVAS2DUTILS_P_H

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
#include <QtGui/qfont.h>
#include <QtGui/qcolor.h>
#include <QtQml/private/qv4value_p.h>

QT_BEGIN_NAMESPACE

namespace QCanvas2DUtils
{
    QColor qColorFromString(const QV4::Value &name);
    QFont qFontFromString(const QString& fontString, const QFont &currentFont);
}

QT_END_NAMESPACE

#endif // QCANVAS2DUTILS_P_H
