// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DPATHVALUETYPE_P_H
#define QCANVAS2DPATHVALUETYPE_P_H

#include "qtcanvas2dglobal_p.h"
#include <QtCanvasPainter/qcanvaspath.h>
#include <QtQml/private/qqmlvaluetype_p.h>
#include <QtCore/qstring.h>
#include <QtGui/qtransform.h>
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

QT_BEGIN_NAMESPACE

class Q_CANVAS2D_EXPORT QCanvas2DPathValueType : private QCanvasPath
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasPath)
    QML_VALUE_TYPE(path2d)
    QML_EXTENDED(QCanvas2DPathValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DPathValueType() = default;

    Q_INVOKABLE void closePath();
    Q_INVOKABLE void moveTo(float x, float y);
    Q_INVOKABLE void lineTo(float x, float y);
    Q_INVOKABLE void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y);
    Q_INVOKABLE void quadraticCurveTo(float cpx, float cpy, float x, float y);
    Q_INVOKABLE void arcTo(float x1, float y1, float x2, float y2, float radius);
    Q_INVOKABLE void arc(float x, float y, float radius,
                         float startAngle, float endAngle, bool counterClockWise = false);
    Q_INVOKABLE void rect(float x, float y, float width, float height);
    Q_INVOKABLE void roundRect(float x, float y, float width, float height, float radius);
    Q_INVOKABLE void roundRect(float x, float y, float width, float height,
                               float radiusTopLeft, float radiusTopRight,
                               float radiusBottomRight, float radiusBottomLeft);
    Q_INVOKABLE void ellipse(float x, float y, float radiusX, float radiusY);
    Q_INVOKABLE void ellipseRect(float x, float y, float width, float height);
    Q_INVOKABLE void circle(float x, float y, float radius);

    Q_INVOKABLE void beginSolidSubPath();
    Q_INVOKABLE void beginHoleSubPath();

    Q_INVOKABLE void addPath(const QCanvasPath &path, const QTransform &transform = QTransform());
    Q_INVOKABLE void addPath(const QString &svgPath, const QTransform &transform = QTransform());

    Q_INVOKABLE bool isEmpty() const;
    Q_INVOKABLE void clear();
};

QT_END_NAMESPACE

#endif // QCANVAS2DPATHVALUETYPE_P_H
