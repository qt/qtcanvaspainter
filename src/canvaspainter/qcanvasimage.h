// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASIMAGE_H
#define QCANVASIMAGE_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCanvasImagePrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasImagePrivate)

class Q_CANVASPAINTER_EXPORT QCanvasImage {
    Q_GADGET
public:
    QCanvasImage();
    QCanvasImage(const QCanvasImage &image);
    ~QCanvasImage();

    QCanvasImage &operator=(const QCanvasImage &image) noexcept;
    QCanvasImage(QCanvasImage &&other) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasImage)
    void swap(QCanvasImage &other) noexcept { d.swap(other.d); }

    operator QVariant() const;

    void detach();

    int id() const;
    int width() const;
    int height() const;
    int size() const;
    bool isNull() const;

    QColor tintColor() const;
    void setTintColor(const QColor &color);

    // TODO: Add API for image rotation angle? Or rely only on state transformation?

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasImage &lhs, const QCanvasImage &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasImage)

    QExplicitlySharedDataPointer<QCanvasImagePrivate> d;
    friend class QCanvasImagePrivate;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasImage &);
#endif
};

Q_DECLARE_SHARED(QCanvasImage)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasImage &);
#endif

QT_END_NAMESPACE

#endif // QCANVASIMAGE_H
