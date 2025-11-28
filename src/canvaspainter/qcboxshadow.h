// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCBOXSHADOW_H
#define QCBOXSHADOW_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcbrush.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCBoxShadowPrivate;
class QCBoxShadow;
class QCImage;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCBoxShadow &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCBoxShadow &);
#endif

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCBoxShadowPrivate)

class Q_CANVASPAINTER_EXPORT QCBoxShadow : public QCBrush
{
public:
    QCBoxShadow();
    QCBoxShadow(const QRectF &rect, float radius = 0.0f, float blur = 0.0f, const QColor &color = QColorConstants::Black);
    QCBoxShadow(float x, float y, float width, float height, float radius = 0.0f, float blur = 0.0f, const QColor &color = QColorConstants::Black);
    QCBoxShadow(const QCBoxShadow &shadow) noexcept;
    ~QCBoxShadow();

    QCBoxShadow &operator=(const QCBoxShadow &shadow) noexcept;
    QCBoxShadow(QCBoxShadow &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCBoxShadow)
    void swap(QCBoxShadow &other) noexcept { d.swap(other.d); }

    bool operator==(const QCBoxShadow &shadow) const;
    inline bool operator!=(const QCBoxShadow &shadow) const { return !(operator==(shadow)); }
    operator QVariant() const;

    void detach();
    BrushType type() const override;

    QRectF rect() const;
    void setRect(const QRectF &rect);
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
    friend class QCPainter;
    friend class QCPainterPrivate;
    QCPaint createPaint(QCPainter *painter) const final;
    void createBoxShadow(float x, float y, float width, float height,
                         const QVector4D &radius,
                         float blur, const QColor &color) const;

private:
    QExplicitlySharedDataPointer<QCBoxShadowPrivate> d;
};

Q_DECLARE_SHARED(QCBoxShadow)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCBoxShadow &);
#endif

QT_END_NAMESPACE

#endif // QCBOXSHADOW_H
