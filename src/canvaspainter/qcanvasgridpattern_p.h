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

#ifndef QCANVASGRIDPATTERN_P_H
#define QCANVASGRIDPATTERN_P_H

#include "engine/qcpainterengine_p.h"
#include "qcanvasbrush_p.h"
#include <QtCore/qshareddata.h>
#include <QtGui/qcolor.h>
#include <qcanvasgridpattern.h>

QT_BEGIN_NAMESPACE

class QCanvasGridPatternPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasGridPatternPrivate(const QCanvasGridPatternPrivate &) = default;
    QCanvasGridPatternPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::GridPattern) {}
    QCanvasBrushPrivate *clone() override;
    bool equals(const QCanvasBrushPrivate &other) const noexcept override;
    QCPaint createPaint(QCanvasPainter *painter) const override;

    static QCanvasGridPatternPrivate *get(QCanvasGridPattern *brush)
    { return brush->d.data(); }
    static const QCanvasGridPatternPrivate *get(const QCanvasGridPattern *brush)
    { return brush->d.data(); }
    static QCanvasGridPattern create(QCanvasGridPatternPrivate *p) { return QCanvasGridPattern(p); }

    void createGridPattern() const;

    // The values are stored in qreal precision so that the QCanvasGridPattern
    // getters return exactly what was passed to the setters. They are
    // truncated to float when the engine-side QCPaint is created.
    QCPaint paint;
    QColor lineColor = QColorConstants::White;
    QColor backgroundColor = QColorConstants::Black;
    qreal x = 0.0;
    qreal y = 0.0;
    qreal width = 10.0;
    qreal height = 10.0;
    qreal feather = 1.0;
    qreal angle = 0.0;
    qreal lineWidth = 1.0;
    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCANVASGRIDPATTERN_P_H
