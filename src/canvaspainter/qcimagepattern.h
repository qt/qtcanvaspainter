// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCIMAGEPATTERN_H
#define QCIMAGEPATTERN_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcbrush.h>
#include <QtCanvasPainter/qcimage.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QCImagePatternPrivate;
class QCImagePattern;

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCImagePattern &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCImagePattern &);
#endif

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCImagePatternPrivate)

class Q_CANVASPAINTER_EXPORT QCImagePattern : public QCBrush
{
public:
    QCImagePattern();
    QCImagePattern(const QCImage &image);
    QCImagePattern(const QCImage &image, const QRectF &rect, float angle = 0.0f, const QColor &tintColor = QColorConstants::White);
    QCImagePattern(const QCImage &image, float x, float y, float width, float height, float angle = 0.0f, const QColor &tintColor = QColorConstants::White);
    QCImagePattern(const QCImagePattern &pattern) noexcept;
    ~QCImagePattern();

    QCImagePattern &operator=(const QCImagePattern &pattern) noexcept;
    QCImagePattern(QCImagePattern &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCImagePattern)
    void swap(QCImagePattern &other) noexcept { d.swap(other.d); }

    bool operator==(const QCImagePattern &pattern) const;
    inline bool operator!=(const QCImagePattern &pattern) const { return !(operator==(pattern)); }
    operator QVariant() const;

    void detach();
    BrushType type() const override;

    QPointF startPosition() const;
    void setStartPosition(float x, float y);
    void setStartPosition(const QPointF &point);
    QSizeF imageSize() const;
    void setImageSize(float width, float height);
    void setImageSize(const QSizeF &size);
    QCImage image() const;
    void setImage(const QCImage &image);
    float rotation() const;
    void setRotation(float rotation);
    QColor tintColor() const;
    void setTintColor(const QColor &color);

private:
    QCPaint createPaint(QCPainter *painter) const final;

private:
    QExplicitlySharedDataPointer<QCImagePatternPrivate> d;
};

Q_DECLARE_SHARED(QCImagePattern)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCImagePattern &);
#endif

QT_END_NAMESPACE

#endif // QCIMAGEPATTERN_H
