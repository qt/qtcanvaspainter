// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCOFFSCREENCANVAS_H
#define QCOFFSCREENCANVAS_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCOffscreenCanvasPrivate;
class QRhiTexture;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCOffscreenCanvasPrivate)

class Q_CANVASPAINTER_EXPORT QCOffscreenCanvas
{
    Q_GADGET

public:
    enum class Flag {
        PreserveContents = 0x01,
        MipMaps = 0x02
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    QCOffscreenCanvas();
    QCOffscreenCanvas(const QCOffscreenCanvas &canvas);
    ~QCOffscreenCanvas();
    QCOffscreenCanvas &operator=(const QCOffscreenCanvas &canvas) noexcept;
    QCOffscreenCanvas(QCOffscreenCanvas &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCOffscreenCanvas)
    void swap(QCOffscreenCanvas &other) noexcept { d.swap(other.d); }

    void detach();

    bool isNull() const;

    Flags flags() const;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    QRhiTexture *texture() const;

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCOffscreenCanvas &lhs, const QCOffscreenCanvas &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCOffscreenCanvas)

    QExplicitlySharedDataPointer<QCOffscreenCanvasPrivate> d;
    friend class QCOffscreenCanvasPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCOffscreenCanvas::Flags)
Q_DECLARE_SHARED(QCOffscreenCanvas)

QT_END_NAMESPACE

#endif // QCOFFSCREENCANVAS_H
