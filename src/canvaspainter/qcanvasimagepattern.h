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
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QCanvasImagePatternPrivate;
class QCanvasImagePattern;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasImagePattern &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasImagePattern &);
#endif

class Q_CANVASPAINTER_EXPORT QCanvasImagePattern : public QCanvasBrush
{
public:
    QCanvasImagePattern();
    QCanvasImagePattern(const QCanvasImage &image);
    QCanvasImagePattern(const QCanvasImage &image, const QRectF &rect, float angle = 0.0f, const QColor &tintColor = QColorConstants::White);
    QCanvasImagePattern(const QCanvasImage &image, float x, float y, float width, float height, float angle = 0.0f, const QColor &tintColor = QColorConstants::White);
    ~QCanvasImagePattern();

    operator QVariant() const;

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    inline void setStartPosition(QPointF point);
    QSizeF imageSize() const;
    void setImageSize(float width, float height);
    inline void setImageSize(QSizeF size);
    QCanvasImage image() const;
    void setImage(const QCanvasImage &image);
    float rotation() const;
    void setRotation(float rotation);
    QColor tintColor() const;
    void setTintColor(const QColor &color);

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasImagePattern &lhs, const QCanvasImagePattern &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasImagePattern)

    friend class QCanvasImagePatternPrivate;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasImagePattern &);
#endif
};

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasImagePattern &);
#endif

inline void QCanvasImagePattern::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()), float(point.y()));
}

inline void QCanvasImagePattern::setImageSize(QSizeF size)
{
    setImageSize(float(size.width()), float(size.height()));
}

QT_END_NAMESPACE

#endif // QCANVASIMAGEPATTERN_H
