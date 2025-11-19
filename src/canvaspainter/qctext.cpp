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

/*!
    \class QCText
    \brief QCText is a cacheable text container for QCPainter.
    \inmodule QtCanvasPainter

    \warning This API is still under active development and likely to change in future versions.

    QCText contains static text which can be cached by QCPainter. Since it has a persistent
    identity, it can be redrawn without having to layout the text, provided it has not been modified.

    \note This class is \l [QtCore]{QExplicitlySharedDataPointer}{explicitly shared}. This
    means that modifying a copy will also modify the original.
*/

/*!
    Constructs a default text container.
*/

QCText::QCText() : d(new QCTextPrivate) {}


/*!
    Constructs a text container positioned at \a x and \a y, with dimensions \a width and \a height.
*/

QCText::QCText(float x, float y, float width, float height)
    : d(new QCTextPrivate(x, y, width, height))
{}

/*!
    Constructs a text container with a position and size given by \a rect.
*/

QCText::QCText(const QRectF &rect)
    : d(new QCTextPrivate(rect))
{}

/*!
    Constructs an text container that is a copy of the given \a text.
*/
QCText::QCText(const QCText &text) noexcept
    : d(text.d)
{
}

/*!
    Destroys the text container.
*/
QCText::~QCText() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCTextPrivate)

/*!
    Assigns the text container \a other to this text container and returns a reference to
    this text container.
*/
QCText &QCText::operator=(const QCText &other) noexcept
{
    QCText(other).swap(*this);
    return *this;
}

/*!
    Sets the horizontal position of the text container to \a x.
*/
void QCText::setX(float x)
{
    if (qFuzzyCompare(d->x, x))
        return;
    d->x = x;
    d->isDirty = true;
}

/*!
    Sets the vertical position of the text container to \a y.
*/
void QCText::setY(float y)
{
    if (qFuzzyCompare(d->y, y))
        return;
    d->y = y;
    d->isDirty = true;
}

/*!
    Sets the horizontal extent of the text container to \a width.
*/
void QCText::setWidth(float width)
{
    if (qFuzzyCompare(d->width, width))
        return;
    d->width = width;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

/*!
    Sets the vertical extent of the text container to \a height.
*/
void QCText::setHeight(float height)
{
    if (qFuzzyCompare(d->height, height))
        return;
    d->height = height;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

/*!
    Sets the position and size of the text container to \a rect.
*/
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

// TODO: should we have setFont instead/in addition?

/*!
    Sets the font size for the text container to \a size.
*/
void QCText::setFontSize(float size)
{
    if (qFuzzyCompare(d->fontSize, size))
        return;
    d->fontSize = size;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}

/*!
    Sets the text string of the text container to \a text.
*/
void QCText::setText(const QString &text)
{
    if (d->text == text)
        return;
    d->text = text;
    d->isLayoutDirty = true;
    d->isPrepared = false;
}


/*!
   \internal
    Sets the optimization flag of the text container to \a optimized.
*/
void QCText::setOptimized(bool optimized)
{
    if (d->optimized == optimized)
        return;

    d->optimized = optimized;
    d->isDirty = true;
    d->isPrepared = false;
}

/*!
    Returns the horizontal position of the text container.
*/
float QCText::x() const
{
    return d->x;
}

/*!
    Returns the vertical position of the text container.
*/
float QCText::y() const
{
    return d->y;
}

/*!
    Returns the horizontal extent of the text container.
*/
float QCText::width() const
{
    return d->width;
}

/*!
    Returns the vertical extent of the text container.
*/
float QCText::height() const
{
    return d->height;
}

/*!
    Returns the font size for the text container.
*/
float QCText::fontSize() const
{
    return d->fontSize;
}

/*!
    Returns the text string of the text container.
*/
QString QCText::text() const
{
    return d->text;
}

/*!
    \internal
    Returns the optimization flag of the text container.
*/
bool QCText::optimized() const
{
    return d->optimized;
}

/*!
    Returns the persistent ID of the text container.
*/
quint32 QCText::getId() const
{
    return d->id;
}

/*!
    \internal
*/
void QCText::setLayoutDirty()
{
    d->isLayoutDirty = true;
}

/*!
    \internal
*/
void QCText::setDirty()
{
    d->isDirty = true;
}

QT_END_NAMESPACE
