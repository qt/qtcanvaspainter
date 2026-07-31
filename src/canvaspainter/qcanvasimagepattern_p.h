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

#ifndef QCANVASIMAGEPATTERN_P_H
#define QCANVASIMAGEPATTERN_P_H

#include "engine/qcpainterengine_p.h"
#include "qcanvasbrush_p.h"
#include "qcanvasimage.h"
#include "qcanvasimagepattern.h"
#include <QtCore/qshareddata.h>
#include <QImage>

QT_BEGIN_NAMESPACE

class QCanvasImagePatternPrivate : public QCanvasBrushPrivate
{
public:
    static uint nextSerialNumber()
    {
        Q_CONSTINIT static QBasicAtomicInteger<uint> serial = Q_BASIC_ATOMIC_INITIALIZER(0);
        return serial.fetchAndAddRelaxed(1);
    }

    QCanvasImagePatternPrivate(const QCanvasImagePatternPrivate &) = default;

    QCanvasImagePatternPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::ImagePattern) {
        serialNumber = nextSerialNumber();
    }
    QCanvasBrushPrivate *clone() override;
    bool equals(const QCanvasBrushPrivate &other) const noexcept override;

    QCPaint createPaint(QCanvasPainter *painter) const override;

    static QCanvasImagePatternPrivate *get(QCanvasImagePattern *brush)
    { return brush->d.data(); }
    static const QCanvasImagePatternPrivate *get(const QCanvasImagePattern *brush)
    { return brush->d.data(); }
    static QCanvasImagePattern create(QCanvasImagePatternPrivate *p) { return QCanvasImagePattern(p); }

    // The values are stored in qreal precision so that the QCanvasImagePattern
    // getters return exactly what was passed to the setters. They are
    // truncated to float when the engine-side QCPaint is created.
    QCanvasImage image;
    QCPaint paint;
    QColor tintColor = QColorConstants::White;
    qreal x = 0.0;
    qreal y = 0.0;
    qreal width = 100.0;
    qreal height = 100.0;
    qreal angle = 0.0;
    bool changed = true;
    uint serialNumber = 0; // Unique id
};

QT_END_NAMESPACE

#endif // QCANVASIMAGEPATTERN_P_H
