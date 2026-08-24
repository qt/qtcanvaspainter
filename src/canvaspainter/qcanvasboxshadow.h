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

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasBoxShadowPrivate)

class QCanvasBoxShadow
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow();
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow(float x, float y, float width, float height);
    explicit QCanvasBoxShadow(const QRectF &rect)
        : QCanvasBoxShadow(float(rect.x()), float(rect.y()),
                           float(rect.width()), float(rect.height()))
    {}
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow(const QCanvasBoxShadow &);
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow &operator=(const QCanvasBoxShadow &);
    QCanvasBoxShadow(QCanvasBoxShadow &&) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasBoxShadow)
    Q_CANVASPAINTER_EXPORT ~QCanvasBoxShadow();

    Q_CANVASPAINTER_EXPORT operator QCanvasBrush() const;
    Q_CANVASPAINTER_EXPORT operator QVariant() const;

    Q_CANVASPAINTER_EXPORT QRectF rect() const;
    inline void setRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void setRect(float x, float y, float width, float height);
    Q_CANVASPAINTER_EXPORT QRectF boundingRect() const;
    Q_CANVASPAINTER_EXPORT float radius() const;
    Q_CANVASPAINTER_EXPORT void setRadius(float radius);
    Q_CANVASPAINTER_EXPORT float blur() const;
    Q_CANVASPAINTER_EXPORT void setBlur(float blur);
    Q_CANVASPAINTER_EXPORT float spread() const;
    Q_CANVASPAINTER_EXPORT void setSpread(float spread);
    Q_CANVASPAINTER_EXPORT QColor color() const;
    Q_CANVASPAINTER_EXPORT void setColor(const QColor &color);

    Q_CANVASPAINTER_EXPORT float topLeftRadius() const;
    Q_CANVASPAINTER_EXPORT void setTopLeftRadius(float radius);
    Q_CANVASPAINTER_EXPORT float topRightRadius() const;
    Q_CANVASPAINTER_EXPORT void setTopRightRadius(float radius);
    Q_CANVASPAINTER_EXPORT float bottomLeftRadius() const;
    Q_CANVASPAINTER_EXPORT void setBottomLeftRadius(float radius);
    Q_CANVASPAINTER_EXPORT float bottomRightRadius() const;
    Q_CANVASPAINTER_EXPORT void setBottomRightRadius(float radius);
    void swap(QCanvasBoxShadow &other) noexcept { d.swap(other.d); }

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasBoxShadow &lhs, const QCanvasBoxShadow &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasBoxShadow)

    void detach();
    friend class QCanvasPainter;
    friend class QCanvasPainterPrivate;
    explicit QCanvasBoxShadow(QCanvasBoxShadowPrivate *p);
    friend class QCanvasBoxShadowPrivate;
    QExplicitlySharedDataPointer<QCanvasBoxShadowPrivate> d;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasBoxShadow &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasBoxShadow &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBoxShadow &);
#endif

};

void QCanvasBoxShadow::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

template<> Q_CANVASPAINTER_EXPORT QCanvasBoxShadow QCanvasBrush::as<QCanvasBoxShadow>() const;

QT_END_NAMESPACE

#endif // QCANVASBOXSHADOW_H
