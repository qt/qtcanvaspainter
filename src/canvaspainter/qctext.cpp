// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qctext.h"
#include "qctext_p.h"

QT_BEGIN_NAMESPACE

// The ID for this object which remains the same when it is copied
// or moved around

quint32 QCTextPrivate::nextId()
{
    static QAtomicInteger<quint32> id(0);
    return ++id;
}

QCText::QCText() : d(new QCTextPrivate) {}

QCText::QCText(float x, float y, float width, float height)
    : d(new QCTextPrivate(x, y, width, height))
{}

QCText::QCText(const QRectF &rect)
    : d(new QCTextPrivate(rect))
{}

QCText::QCText(const QCText &text) noexcept
    : d(text.d)
{
}

QCText::~QCText() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCTextPrivate)

QCText &QCText::operator=(const QCText &other) noexcept
{
    QCText(other).swap(*this);
    return *this;
}

void QCText::setX(float x)
{
    if (qFuzzyCompare(d->x, x))
        return;
    d->x = x;
    d->isDirty = true;
}

void QCText::setY(float y)
{
    if (qFuzzyCompare(d->y, y))
        return;
    d->y = y;
    d->isDirty = true;
}

void QCText::setWidth(float width)
{
    if (qFuzzyCompare(d->width, width))
        return;
    d->width = width;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

void QCText::setHeight(float height)
{
    if (qFuzzyCompare(d->height, height))
        return;
    d->height = height;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

void QCText::setRect(const QRectF &rect)
{
    const float x = rect.x();
    const float y = rect.y();
    const float width = rect.width();
    const float height = rect.height();
    if (qFuzzyCompare(d->x, x) && qFuzzyCompare(d->y, y) &&
        qFuzzyCompare(d->width, width) && qFuzzyCompare(d->height, height)) {
        return;
    }

    if (!qFuzzyCompare(d->width, width) || !qFuzzyCompare(d->height, height)) {
        d->isLayoutDirty = true;
        d->isPrepared = false;
    }
    else {
        d->isDirty = true;
    }

    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
}

void QCText::setFontSize(float size)
{
    if (qFuzzyCompare(d->fontSize, size))
        return;
    d->fontSize = size;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

void QCText::setText(const QString &text)
{
    if (d->text == text)
        return;
    d->text = text;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

void QCText::setOptimized(bool optimized)
{
    if (d->optimized == optimized)
        return;

    d->optimized = optimized;
    d->isDirty = true;
    d->isPrepared = false;
}

float QCText::x() const
{
    return d->x;
}

float QCText::y() const
{
    return d->y;
}

float QCText::width() const
{
    return d->width;
}

float QCText::height() const
{
    return d->height;
}

float QCText::fontSize() const
{
    return d->fontSize;
}

QString QCText::text() const
{
    return d->text;
}

bool QCText::optimized() const
{
    return d->optimized;
}

quint32 QCText::getId() const
{
    return d->id;
}

void QCText::setLayoutDirty()
{
    d->isLayoutDirty = true;
}

void QCText::setDirty()
{
    d->isDirty = true;
}

QT_END_NAMESPACE
