// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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

#ifndef QCGRIDPATTERN_P_H
#define QCGRIDPATTERN_P_H

#include "engine/qcpainterengine_p.h"
#include "qcbrush_p.h"
#include <QtCore/qshareddata.h>
#include <QtGui/qcolor.h>
#include <qcgridpattern.h>

QT_BEGIN_NAMESPACE

class QCGridPatternPrivate : public QCBrushPrivate
{
public:
    QCGridPatternPrivate(const QCGridPatternPrivate &) = default;
    QCGridPatternPrivate() : QCBrushPrivate(QCBrush::BrushType::GridPattern) {}
    QCBrushPrivate *clone() override;
    QCPaint createPaint(QCPainter *painter) const override;

    static QCGridPatternPrivate *get(QCGridPattern *brush)
    { return static_cast<QCGridPatternPrivate*>(brush->baseData.get()); }
    static const QCGridPatternPrivate *get(const QCGridPattern *brush)
    { return static_cast<QCGridPatternPrivate*>(brush->baseData.get()); }

    void createGridPattern() const;

    QCPaint paint;
    QColor lineColor = QColorConstants::White;
    QColor backgroundColor = QColorConstants::Black;
    float x = 0.0f;
    float y = 0.0f;
    float width = 10.0f;
    float height = 10.0f;
    float feather = 1.0f;
    float angle = 0.0f;
    float lineWidth = 1.0f;
    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCGRIDPATTERN_P_H
