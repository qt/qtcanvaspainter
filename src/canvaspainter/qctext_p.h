// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCTEXT_P_H
#define QCTEXT_P_H

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

#include "qctext.h"

QT_BEGIN_NAMESPACE

class QCTextPrivate : public QSharedData
{
public:
    QCTextPrivate() : id(nextId()) {}
    QCTextPrivate(const QRectF &rect)
        : id(nextId()), x(rect.x()), y(rect.y()), width(rect.width()), height(rect.height())
    {}
    QCTextPrivate(float x, float y, float width, float height)
        : id(nextId()), x(x), y(y), width(width), height(height)
    {}

    static QCTextPrivate *get(QCText *ctext) { return ctext->d.get(); }
    static const QCTextPrivate *get(const QCText *ctext) { return ctext->d.get(); }

    quint32 id;
    QString text;
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
    float fontSize = 0; // This is basically for state purposes
    bool optimized = false;
    bool isLayoutDirty = true;
    bool isDirty = true;
    bool isPrepared = false;

    static quint32 nextId();
};

QT_END_NAMESPACE

#endif // QCTEXT_P_H
