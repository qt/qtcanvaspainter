// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCBRUSH_P_H
#define QCBRUSH_P_H

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

#include "qcbrush.h"

QT_BEGIN_NAMESPACE

class QCBrushPrivate : public QSharedData
{
public:
    QCBrushPrivate(QCBrush::BrushType t) : type(t) {}
    virtual ~QCBrushPrivate() = default;
    QCBrush::BrushType type;
    virtual QCBrushPrivate *clone() = 0;
    virtual QCPaint createPaint(QCPainter *painter) const = 0;
};

template <>
inline QCBrushPrivate *QExplicitlySharedDataPointer<QCBrushPrivate>::clone()
{
    return d ? d->clone() : nullptr;
}

QT_END_NAMESPACE

#endif // QCBRUSH_P_H
