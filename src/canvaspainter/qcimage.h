// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCIMAGE_H
#define QCIMAGE_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCImagePrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCImagePrivate)

class Q_CANVASPAINTER_EXPORT QCImage {
    Q_GADGET
public:
    QCImage();
    QCImage(const QCImage &image) noexcept;
    ~QCImage();

    QCImage &operator=(const QCImage &image) noexcept;
    QCImage(QCImage &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCImage)
    void swap(QCImage &other) noexcept { d.swap(other.d); }

    bool operator==(const QCImage &image) const;
    bool operator!=(const QCImage &image) const { return !(operator==(image)); }
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

protected:
    QExplicitlySharedDataPointer<QCImagePrivate> d;
    friend class QCImagePrivate;
};

Q_DECLARE_SHARED(QCImage)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCImage &);
#endif

QT_END_NAMESPACE

#endif // QCIMAGE_H
