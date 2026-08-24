// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASIMAGEPATTERN_H
#define QCANVASIMAGEPATTERN_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtCanvasPainter/qcanvasimage.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QCanvasImagePatternPrivate;
class QCanvasImagePattern;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasImagePatternPrivate)

class QCanvasImagePattern
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasImagePattern();
    Q_CANVASPAINTER_EXPORT QCanvasImagePattern(const QCanvasImage &image);
    QCanvasImagePattern(const QCanvasImage &image, QPointF startPosition, QSizeF imageSize)
        : QCanvasImagePattern(image, float(startPosition.x()), float(startPosition.y()),
                              float(imageSize.width()), float(imageSize.height()))
    {}
    Q_CANVASPAINTER_EXPORT QCanvasImagePattern(const QCanvasImage &image,
                                               float startX, float startY,
                                               float imageWidth, float imageHeight);
    Q_CANVASPAINTER_EXPORT QCanvasImagePattern(const QCanvasImagePattern &);
    Q_CANVASPAINTER_EXPORT QCanvasImagePattern &operator=(const QCanvasImagePattern &);
    QCanvasImagePattern(QCanvasImagePattern &&) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasImagePattern)
    Q_CANVASPAINTER_EXPORT ~QCanvasImagePattern();

    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QCanvasBrush() const;
    Q_CANVASPAINTER_EXPORT Q_IMPLICIT operator QVariant() const;

    Q_CANVASPAINTER_EXPORT QPointF startPosition() const;
    Q_CANVASPAINTER_EXPORT void setStartPosition(float x, float y);
    inline void setStartPosition(QPointF point);
    Q_CANVASPAINTER_EXPORT QSizeF imageSize() const;
    Q_CANVASPAINTER_EXPORT void setImageSize(float width, float height);
    inline void setImageSize(QSizeF size);
    Q_CANVASPAINTER_EXPORT QCanvasImage image() const;
    Q_CANVASPAINTER_EXPORT void setImage(const QCanvasImage &image);
    Q_CANVASPAINTER_EXPORT float rotation() const;
    Q_CANVASPAINTER_EXPORT void setRotation(float rotation);
    Q_CANVASPAINTER_EXPORT QColor tintColor() const;
    Q_CANVASPAINTER_EXPORT void setTintColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT uint serialNumber() const;
    void swap(QCanvasImagePattern &other) noexcept { d.swap(other.d); }

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasImagePattern &lhs, const QCanvasImagePattern &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasImagePattern)

    void detach();
    explicit QCanvasImagePattern(QCanvasImagePatternPrivate *p);
    friend class QCanvasImagePatternPrivate;
    QExplicitlySharedDataPointer<QCanvasImagePatternPrivate> d;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasImagePattern &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasImagePattern &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasImagePattern &);
#endif
};

void QCanvasImagePattern::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()), float(point.y()));
}

void QCanvasImagePattern::setImageSize(QSizeF size)
{
    setImageSize(float(size.width()), float(size.height()));
}

template<> Q_CANVASPAINTER_EXPORT QCanvasImagePattern QCanvasBrush::as<QCanvasImagePattern>() const;

QT_END_NAMESPACE

#endif // QCANVASIMAGEPATTERN_H
