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

#ifndef QCIMAGEPATTERN_P_H
#define QCIMAGEPATTERN_P_H

#include "engine/qcpainterengine_p.h"
#include "qcbrush_p.h"
#include "qcimage.h"
#include "qcimagepattern.h"
#include <QtCore/qshareddata.h>
#include <QImage>

QT_BEGIN_NAMESPACE

class QCImagePatternPrivate : public QCBrushPrivate
{
public:
    QCImagePatternPrivate(const QCImagePatternPrivate &) = default;

    QCImagePatternPrivate() : QCBrushPrivate(QCBrush::BrushType::ImagePattern) {}
    QCBrushPrivate *clone() override;

    QCPaint createPaint(QCPainter *painter) const override;

    static QCImagePatternPrivate *get(QCImagePattern *brush)
    { return static_cast<QCImagePatternPrivate*>(brush->baseData.get()); }
    static const QCImagePatternPrivate *get(const QCImagePattern *brush)
    { return static_cast<QCImagePatternPrivate*>(brush->baseData.get()); }

    QCImage image;
    QCPaint paint;
    QColor tintColor = QColorConstants::White;
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 100.0f;
    float angle = 0.0f;
    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCIMAGEPATTERN_P_H
