// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DBRUSHVALUETYPE_P_H
#define QCANVAS2DBRUSHVALUETYPE_P_H

#include "qtcanvas2dglobal_p.h"
#include <QtCanvasPainter/qcanvaslineargradient.h>
#include <QtCanvasPainter/qcanvasradialgradient.h>
#include <QtCanvasPainter/qcanvasconicalgradient.h>
#include <QtCanvasPainter/qcanvasboxgradient.h>
#include <QtCanvasPainter/qcanvasgridpattern.h>
#include <QtCanvasPainter/qcanvasboxshadow.h>
#include <QtCanvasPainter/qcanvasimagepattern.h>
#include <QtGui/qcolor.h>
#include <QtQml/private/qqmlvaluetype_p.h>

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

class Q_CANVAS2D_EXPORT QCanvas2DLinearGradientValueType : private QCanvasLinearGradient
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasLinearGradient)
    QML_VALUE_TYPE(lineargradient2d)
    QML_EXTENDED(QCanvas2DLinearGradientValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DLinearGradientValueType() = default;
    Q_INVOKABLE void addColorStop(float offset, const QColor &color);
};

class Q_CANVAS2D_EXPORT QCanvas2DRadialGradientValueType : private QCanvasRadialGradient
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasRadialGradient)
    QML_VALUE_TYPE(radialgradient2d)
    QML_EXTENDED(QCanvas2DRadialGradientValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DRadialGradientValueType() = default;
    Q_INVOKABLE void addColorStop(float offset, const QColor &color);
};

class Q_CANVAS2D_EXPORT QCanvas2DConicalGradientValueType : private QCanvasConicalGradient
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasConicalGradient)
    QML_VALUE_TYPE(conicalgradient2d)
    QML_EXTENDED(QCanvas2DConicalGradientValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DConicalGradientValueType() = default;
    Q_INVOKABLE void addColorStop(float offset, const QColor &color);
};

class Q_CANVAS2D_EXPORT QCanvas2DBoxGradientValueType : private QCanvasBoxGradient
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasBoxGradient)
    QML_VALUE_TYPE(boxgradient2d)
    QML_EXTENDED(QCanvas2DBoxGradientValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DBoxGradientValueType() = default;
    Q_INVOKABLE void addColorStop(float offset, const QColor &color);
};

class Q_CANVAS2D_EXPORT QCanvas2DGridPatternValueType : private QCanvasGridPattern
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasGridPattern)
    QML_VALUE_TYPE(gridpattern2d)
    QML_EXTENDED(QCanvas2DGridPatternValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DGridPatternValueType() = default;
};

class Q_CANVAS2D_EXPORT QCanvas2DBoxShadowValueType : private QCanvasBoxShadow
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasBoxShadow)
    QML_VALUE_TYPE(boxshadow2d)
    QML_EXTENDED(QCanvas2DBoxShadowValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DBoxShadowValueType() = default;
};

class Q_CANVAS2D_EXPORT QCanvas2DImagePatternValueType : private QCanvasImagePattern
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QCanvasImagePattern)
    QML_VALUE_TYPE(imagepattern2d)
    QML_EXTENDED(QCanvas2DImagePatternValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DImagePatternValueType() = default;
};

QT_END_NAMESPACE

#endif // QCANVAS2DBRUSHVALUETYPE_P_H
