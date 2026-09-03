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
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow(qreal x, qreal y, qreal width, qreal height);
    explicit QCanvasBoxShadow(const QRectF &rect)
        : QCanvasBoxShadow(rect.x(), rect.y(), rect.width(), rect.height())
    {}
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow(const QCanvasBoxShadow &);
    Q_CANVASPAINTER_EXPORT QCanvasBoxShadow &operator=(const QCanvasBoxShadow &);
    QCanvasBoxShadow(QCanvasBoxShadow &&) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasBoxShadow)
    Q_CANVASPAINTER_EXPORT ~QCanvasBoxShadow();

    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QCanvasBrush() const;
    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QVariant() const;

    Q_CANVASPAINTER_EXPORT QRectF rect() const;
    inline void setRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void setRect(qreal x, qreal y, qreal width, qreal height);
    Q_CANVASPAINTER_EXPORT QRectF boundingRect() const;
    Q_CANVASPAINTER_EXPORT qreal radius() const;
    Q_CANVASPAINTER_EXPORT void setRadius(qreal radius);
    Q_CANVASPAINTER_EXPORT qreal blur() const;
    Q_CANVASPAINTER_EXPORT void setBlur(qreal blur);
    Q_CANVASPAINTER_EXPORT qreal spread() const;
    Q_CANVASPAINTER_EXPORT void setSpread(qreal spread);
    Q_CANVASPAINTER_EXPORT QColor color() const;
    Q_CANVASPAINTER_EXPORT void setColor(QColor color);

    Q_CANVASPAINTER_EXPORT qreal topLeftRadius() const;
    Q_CANVASPAINTER_EXPORT void setTopLeftRadius(qreal radius);
    Q_CANVASPAINTER_EXPORT qreal topRightRadius() const;
    Q_CANVASPAINTER_EXPORT void setTopRightRadius(qreal radius);
    Q_CANVASPAINTER_EXPORT qreal bottomLeftRadius() const;
    Q_CANVASPAINTER_EXPORT void setBottomLeftRadius(qreal radius);
    Q_CANVASPAINTER_EXPORT qreal bottomRightRadius() const;
    Q_CANVASPAINTER_EXPORT void setBottomRightRadius(qreal radius);
    void swap(QCanvasBoxShadow &other) noexcept { d.swap(other.d); }

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasBoxShadow &lhs, const QCanvasBoxShadow &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasBoxShadow)

    void detach();
    friend class QCanvasPainter;
    friend class QCanvasPainterPrivate;
    Q_CANVASPAINTER_EXPORT explicit QCanvasBoxShadow(QCanvasBoxShadowPrivate *p);
    friend class QCanvasBoxShadowPrivate;
    QExplicitlySharedDataPointer<QCanvasBoxShadowPrivate> d;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasBoxShadow &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasBoxShadow &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasBoxShadow &);
#endif
    friend size_t qHash(const QCanvasBoxShadow &, size_t seed) = delete;
};

Q_DECLARE_SHARED(QCanvasBoxShadow)

void QCanvasBoxShadow::setRect(const QRectF &rect)
{
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QT_END_NAMESPACE

#endif // QCANVASBOXSHADOW_H
