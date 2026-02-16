// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBOXSHADOW_H
#define QCANVASBOXSHADOW_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCanvasBoxShadowPrivate;
class QCanvasBoxShadow;
class QCanvasImage;

class Q_CANVASPAINTER_EXPORT QCanvasBoxShadow : public QCanvasBrush
{
public:
    QCanvasBoxShadow();
    QCanvasBoxShadow(const QRectF &rect, float radius = 0.0f, float blur = 0.0f, const QColor &color = QColorConstants::Black);
    QCanvasBoxShadow(float x, float y, float width, float height, float radius = 0.0f, float blur = 0.0f, const QColor &color = QColorConstants::Black);
    ~QCanvasBoxShadow();

    operator QVariant() const;

    QRectF rect() const;
    inline void setRect(const QRectF &rect);
    void setRect(float x, float y, float width, float height);
    QRectF boundingRect() const;
    float radius() const;
    void setRadius(float radius);
    float blur() const;
    void setBlur(float blur);
    float spread() const;
    void setSpread(float spread);
    QColor color() const;
    void setColor(const QColor &color);

    float topLeftRadius() const;
    void setTopLeftRadius(float radius);
    float topRightRadius() const;
    void setTopRightRadius(float radius);
    float bottomLeftRadius() const;
    void setBottomLeftRadius(float radius);
    float bottomRightRadius() const;
    void setBottomRightRadius(float radius);

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasBoxShadow &lhs, const QCanvasBoxShadow &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasBoxShadow)

    friend class QCanvasPainter;
    friend class QCanvasPainterPrivate;
    friend class QCanvasBoxShadowPrivate;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasBoxShadow &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasBoxShadow &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBoxShadow &);
#endif

};

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasBoxShadow &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasBoxShadow &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBoxShadow &);
#endif

inline void QCanvasBoxShadow::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

QT_END_NAMESPACE

#endif // QCANVASBOXSHADOW_H
