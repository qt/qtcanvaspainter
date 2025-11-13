// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCCANVAS_H
#define QCCANVAS_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCCanvasPrivate;
class QRhiTexture;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCCanvasPrivate)

class Q_CANVASPAINTER_EXPORT QCCanvas
{
    Q_GADGET

public:
    enum class Flag {
        // Requests preserving (no clearing) of the color texture.
        // There's a catch: this will not always work with MSAA, depending on underlying details (on GLES, with certain extensions present) .
        // So probably going to be limited to non-multisample canvases.
        PreserveContents = 0x01
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    QCCanvas();
    QCCanvas(const QCCanvas &canvas) noexcept;
    ~QCCanvas();
    QCCanvas &operator=(const QCCanvas &canvas) noexcept;
    QCCanvas(QCCanvas &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCCanvas)
    void swap(QCCanvas &other) noexcept { d.swap(other.d); }

    bool operator==(const QCCanvas &canvas) const;
    inline bool operator!=(const QCCanvas &canvas) const { return !(operator==(canvas)); }

    void detach();

    bool isNull() const;

    // flags and settings such as multisample count must be controlled by the
    // canvas factory function, we cannot have simple setters here, since under
    // the hood everything is immutable...
    Flags flags() const;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    QRhiTexture *texture() const;

private:
    QExplicitlySharedDataPointer<QCCanvasPrivate> d;
    friend class QCCanvasPrivate;
    friend class QCPainterRhiRenderer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCCanvas::Flags)
Q_DECLARE_SHARED(QCCanvas)

QT_END_NAMESPACE

#endif // QCCANVAS_H
