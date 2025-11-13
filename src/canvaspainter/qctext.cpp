// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qctext.h"

QT_BEGIN_NAMESPACE

// The ID for this object which remains the same when it is copied
// or moved around
quint32 QCText::nextId()
{
    static QAtomicInteger<quint32> id(0);
    return ++id;
}

QCText::QCText() : m_Id(nextId()) {}

QCText::QCText(float x, float y, float width, float height)
    : m_Id(nextId())
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
{}

QCText::QCText(const QRectF &rect)
    : m_Id(nextId())
    , m_x(rect.x())
    , m_y(rect.y())
    , m_width(rect.width())
    , m_height(rect.height())
{}

void QCText::setX(float x)
{
    if (qFuzzyCompare(m_x, x))
        return;
    m_x = x;
    m_isDirty = true;
}

void QCText::setY(float y)
{
    if (qFuzzyCompare(m_y, y))
        return;
    m_y = y;
    m_isDirty = true;
}

void QCText::setWidth(float width)
{
    if (qFuzzyCompare(m_width, width))
        return;
    m_width = width;
    m_isLayoutDirty = true;
    m_isPrepared = false;
}

void QCText::setHeight(float height)
{
    if (qFuzzyCompare(m_height, height))
        return;
    m_height = height;
    m_isLayoutDirty = true;
    m_isPrepared = false;
}

void QCText::setRect(const QRectF &rect)
{
    const float x = rect.x();
    const float y = rect.y();
    const float width = rect.width();
    const float height = rect.height();
    if (qFuzzyCompare(m_x, x) && qFuzzyCompare(m_y, y) &&
        qFuzzyCompare(m_width, width) && qFuzzyCompare(m_height, height)) {
        return;
    }

    if (!qFuzzyCompare(m_width, width) || !qFuzzyCompare(m_height, height)) {
        m_isLayoutDirty = true;
        m_isPrepared = false;
    }
    else {
        m_isDirty = true;
    }

    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

void QCText::setFontSize(float size)
{
    if (qFuzzyCompare(m_fontSize, size))
        return;
    m_fontSize = size;
    m_isLayoutDirty = true;
    m_isPrepared = false;
}

void QCText::setText(const QString &text)
{
    if (m_text == text)
        return;
    m_text = text;
    m_isLayoutDirty = true;
    m_isPrepared = false;
}

void QCText::setOptimized(bool optimized)
{
    if (m_optimized == optimized)
        return;

    m_optimized = optimized;
    m_isDirty = true;
    m_isPrepared = false;
}

float QCText::x() const
{
    return m_x;
}

float QCText::y() const
{
    return m_y;
}

float QCText::width() const
{
    return m_width;
}

float QCText::height() const
{
    return m_height;
}

float QCText::fontSize() const
{
    return m_fontSize;
}

QString QCText::text() const
{
    return m_text;
}

bool QCText::optimized() const
{
    return m_optimized;
}

quint32 QCText::getId() const
{
    return m_Id;
}

void QCText::setLayoutDirty()
{
    m_isLayoutDirty = true;
}

void QCText::setDirty()
{
    m_isDirty = true;
}

QT_END_NAMESPACE
