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

// ***** Linear Gradient *****

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
    Q_INVOKABLE QPointF startPosition() const;
    Q_INVOKABLE void setStartPosition(float x, float y);
    Q_INVOKABLE inline void setStartPosition(QPointF start);
    Q_INVOKABLE QPointF endPosition() const;
    Q_INVOKABLE void setEndPosition(float x, float y);
    Q_INVOKABLE inline void setEndPosition(QPointF end);
};

inline void QCanvas2DLinearGradientValueType::setStartPosition(QPointF start)
{
    setStartPosition(float(start.x()), float(start.y()));
}

inline void QCanvas2DLinearGradientValueType::setEndPosition(QPointF end)
{
    setEndPosition(float(end.x()), float(end.y()));
}

// ***** Radial Gradient *****

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
    Q_INVOKABLE QPointF centerPosition() const;
    Q_INVOKABLE void setCenterPosition(float x, float y);
    Q_INVOKABLE inline void setCenterPosition(QPointF center);
    Q_INVOKABLE QPointF innerCenterPosition() const;
    Q_INVOKABLE void setInnerCenterPosition(float x, float y);
    Q_INVOKABLE inline void setInnerCenterPosition(QPointF center);
    Q_INVOKABLE QPointF outerCenterPosition() const;
    Q_INVOKABLE void setOuterCenterPosition(float x, float y);
    Q_INVOKABLE inline void setOuterCenterPosition(QPointF center);
    Q_INVOKABLE float outerRadius() const;
    Q_INVOKABLE void setOuterRadius(float radius);
    Q_INVOKABLE float innerRadius() const;
    Q_INVOKABLE void setInnerRadius(float radius);
};

inline void QCanvas2DRadialGradientValueType::setCenterPosition(QPointF center)
{
    setCenterPosition(float(center.x()), float(center.y()));
}

inline void QCanvas2DRadialGradientValueType::setInnerCenterPosition(QPointF center)
{
    setInnerCenterPosition(float(center.x()), float(center.y()));
}

inline void QCanvas2DRadialGradientValueType::setOuterCenterPosition(QPointF center)
{
    setOuterCenterPosition(float(center.x()), float(center.y()));
}

// ***** Conical Gradient *****

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
    Q_INVOKABLE QPointF centerPosition() const;
    Q_INVOKABLE void setCenterPosition(float x, float y);
    Q_INVOKABLE inline void setCenterPosition(QPointF center);
    Q_INVOKABLE float angle() const;
    Q_INVOKABLE void setAngle(float angle);
};

inline void QCanvas2DConicalGradientValueType::setCenterPosition(QPointF center)
{
    setCenterPosition(float(center.x()), float(center.y()));
}

// ***** Box Gradient *****

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
    Q_INVOKABLE QRectF rect() const;
    Q_INVOKABLE void setRect(float x, float y, float width, float height);
    Q_INVOKABLE inline void setRect(const QRectF &rect);
    Q_INVOKABLE float feather() const;
    Q_INVOKABLE void setFeather(float feather);
    Q_INVOKABLE float radius() const;
    Q_INVOKABLE void setRadius(float radius);
};

inline void QCanvas2DBoxGradientValueType::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

// ***** Grid Pattern *****

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
    Q_INVOKABLE QPointF startPosition() const;
    Q_INVOKABLE void setStartPosition(float x, float y);
    Q_INVOKABLE inline void setStartPosition(QPointF point);
    Q_INVOKABLE QSizeF cellSize() const;
    Q_INVOKABLE void setCellSize(float width, float height);
    Q_INVOKABLE inline void setCellSize(QSizeF size);
    Q_INVOKABLE float lineWidth() const;
    Q_INVOKABLE void setLineWidth(float width);
    Q_INVOKABLE float feather() const;
    Q_INVOKABLE void setFeather(float feather);
    Q_INVOKABLE float rotation() const;
    Q_INVOKABLE void setRotation(float rotation);
    Q_INVOKABLE QColor lineColor() const;
    Q_INVOKABLE void setLineColor(const QColor &color);
    Q_INVOKABLE QColor backgroundColor() const;
    Q_INVOKABLE void setBackgroundColor(const QColor &color);
};

inline void QCanvas2DGridPatternValueType::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()), float(point.y()));
}

inline void QCanvas2DGridPatternValueType::setCellSize(QSizeF size)
{
    setCellSize(float(size.width()), float(size.height()));
}

// ***** Box Shadow *****

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
    Q_INVOKABLE QRectF rect() const;
    Q_INVOKABLE inline void setRect(const QRectF &rect);
    Q_INVOKABLE void setRect(float x, float y, float width, float height);
    Q_INVOKABLE QRectF boundingRect() const;
    Q_INVOKABLE float radius() const;
    Q_INVOKABLE void setRadius(float radius);
    Q_INVOKABLE float blur() const;
    Q_INVOKABLE void setBlur(float blur);
    Q_INVOKABLE float spread() const;
    Q_INVOKABLE void setSpread(float spread);
    Q_INVOKABLE QColor color() const;
    Q_INVOKABLE void setColor(const QColor &color);
    Q_INVOKABLE float topLeftRadius() const;
    Q_INVOKABLE void setTopLeftRadius(float radius);
    Q_INVOKABLE float topRightRadius() const;
    Q_INVOKABLE void setTopRightRadius(float radius);
    Q_INVOKABLE float bottomLeftRadius() const;
    Q_INVOKABLE void setBottomLeftRadius(float radius);
    Q_INVOKABLE float bottomRightRadius() const;
    Q_INVOKABLE void setBottomRightRadius(float radius);
};

inline void QCanvas2DBoxShadowValueType::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

// ***** Image Pattern *****

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
    Q_INVOKABLE QPointF startPosition() const;
    Q_INVOKABLE void setStartPosition(float x, float y);
    Q_INVOKABLE inline void setStartPosition(QPointF point);
    Q_INVOKABLE QSizeF imageSize() const;
    Q_INVOKABLE void setImageSize(float width, float height);
    Q_INVOKABLE inline void setImageSize(QSizeF size);
    Q_INVOKABLE float rotation() const;
    Q_INVOKABLE void setRotation(float rotation);
    Q_INVOKABLE QColor tintColor() const;
    Q_INVOKABLE void setTintColor(const QColor &color);
};

inline void QCanvas2DImagePatternValueType::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()), float(point.y()));
}

inline void QCanvas2DImagePatternValueType::setImageSize(QSizeF size)
{
    setImageSize(float(size.width()), float(size.height()));
}

QT_END_NAMESPACE

#endif // QCANVAS2DBRUSHVALUETYPE_P_H
