// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBOXSHADOW_P_H
#define QCANVASBOXSHADOW_P_H

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

#include "qcanvasboxshadow.h"
#include "qcanvasbrush_p.h"
#include "qcpaint_p.h"
#include "qcanvasimage.h"
#include <QtCore/qshareddata.h>
#include <QImage>

QT_BEGIN_NAMESPACE

class QCanvasBoxShadowPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasBoxShadowPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::BoxShadow) {}
    QCanvasBoxShadowPrivate(const QCanvasBoxShadowPrivate &) = default;
    QCanvasBrushPrivate *clone() override;
    bool equals(const QCanvasBrushPrivate &other) const noexcept override;

    static QCanvasBoxShadowPrivate *get(QCanvasBoxShadow *brush)
    { return brush->d.data(); }
    static const QCanvasBoxShadowPrivate *get(const QCanvasBoxShadow *brush)
    { return brush->d.data(); }
    static QCanvasBoxShadow create(QCanvasBoxShadowPrivate *p) { return QCanvasBoxShadow(p); }
    static QCanvasBoxShadow getFromCanvasBrush(const QCanvasBrush &brush)
    {
        Q_ASSERT(brush.type() == QCanvasBrush::BrushType::BoxShadow);
        return create(static_cast<QCanvasBoxShadowPrivate *>(QCanvasBrushPrivate::get(brush)));
    }

    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createBoxShadow(qreal x, qreal y, qreal width, qreal height,
                         const QVector4D &radius,
                         qreal blur, const QColor &color) const;

    qreal clampedRadius(qreal radius, qreal width, qreal height) const;
    QColor clampedColor() const;

    // The values are stored in qreal precision so that the QCanvasBoxShadow
    // getters return exactly what was passed to the setters. They are
    // truncated to float when the engine-side QCPaint is created.
    QCPaint paint;
    qreal x = 0.0;
    qreal y = 0.0;
    qreal width = 100.0;
    qreal height = 100.0;
    qreal radius = 0.0;
    qreal spread = 0.0;
    qreal topLeftRadius = -1.0;
    qreal topRightRadius = -1.0;
    qreal bottomLeftRadius = -1.0;
    qreal bottomRightRadius = -1.0;
    qreal blur = 0.0;
    QColor color = QColorConstants::Black;

    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCBOXSHADOW_P_H
