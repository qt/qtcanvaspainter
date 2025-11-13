// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCTEXT_H
#define QCTEXT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtGui/qtextlayout.h>
#include <QtCore/qatomic.h>

QT_BEGIN_NAMESPACE

// A state object for texts that will be rendered
class Q_CANVASPAINTER_EXPORT QCText
{
public:
    QCText();
    QCText(float x, float y, float width, float height);
    QCText(const QRectF &rect);

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
    friend class QCPainterEngine;
    friend class QCPainterRhiRenderer;

    // TODO: Make this a proper value class and
    // move all these into a private class.
    quint32 m_Id;
    QString m_text;
    float m_x = 0;
    float m_y = 0;
    float m_width = 0;
    float m_height = 0;
    float m_fontSize; // This is basically for state purposes
    bool m_optimized = false;
    bool m_isLayoutDirty = true;
    bool m_isDirty = true;
    bool m_isPrepared = false;

    static quint32 nextId();
};

QT_END_NAMESPACE

#endif
