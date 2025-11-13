// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCAREAALLOCATOR_H
#define QCAREAALLOCATOR_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#include <QtGui/private/qtguiglobal_p.h>
#include <QtCore/qsize.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

struct QCAreaAllocatorNode;
class Qpoint;

class QCAreaAllocator
{
public:
    QCAreaAllocator(const QSize &size);
    ~QCAreaAllocator();

    QRect allocate(const QSize &size);
    bool deallocate(const QRect &rect);
    bool isEmpty() const { return m_root == nullptr; }
    QSize size() const { return m_size; }

private:
    bool allocateInNode(
        const QSize &size, QPoint &result, const QRect &currentRect, QCAreaAllocatorNode *node);
    bool deallocateInNode(const QPoint &pos, QCAreaAllocatorNode *node);
    void mergeNodeWithNeighbors(QCAreaAllocatorNode *node);

    QCAreaAllocatorNode *m_root = nullptr;
    QSize m_size;
};

QT_END_NAMESPACE

#endif // QCAREAALLOCATOR_H
