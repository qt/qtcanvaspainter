// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCTEXT_H
#define QCTEXT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtGui/qtextlayout.h>
#include <QtCore/qatomic.h>

QT_BEGIN_NAMESPACE

class QCTextPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCTextPrivate)

// A state object for texts that will be rendered
class Q_CANVASPAINTER_EXPORT QCText
{
public:
    QCText();
    QCText(float x, float y, float width, float height);
    QCText(const QRectF &rect);
    QCText(const QCText &text) noexcept;
    ~QCText();

    QCText &operator=(const QCText &brush) noexcept;
    QCText(QCText &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCText)
    void swap(QCText &other) noexcept { d.swap(other.d); }

    void setX(float x);
    void setY(float y);
    void setWidth(float width);
    void setHeight(float height);
    void setRect(const QRectF &rect);
    void setFontSize(float size);
    void setText(const QString &text);
    // TODO: Should this be public API?
    // When should user set this true/false?
    void setOptimized(bool optimized);

    float x() const;
    float y() const;
    float width() const;
    float height() const;
    float fontSize() const;
    QString text() const;
    bool optimized() const;
    quint32 getId() const;

    void setLayoutDirty();
    void setDirty();

private:
    QExplicitlySharedDataPointer<QCTextPrivate> d;
    friend class QCTextPrivate;
};

Q_DECLARE_SHARED(QCText)

QT_END_NAMESPACE

#endif
