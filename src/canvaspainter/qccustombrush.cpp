// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qccustombrush.h"
#include "qccustombrush_p.h"
#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE

static QShader getShader(const QString &name)
{
    QFile f(name);
    if (f.open(QIODevice::ReadOnly)) {
        return QShader::fromSerialized(f.readAll());
    } else {
        qWarning() << "Unable to create custom brush with shader:" << name;
        return QShader();
    }
}

/*!
    \class QCCustomBrush
    \brief QCCustomBrush is a brush with custom shaders.
    \inmodule QtCanvasPainter

    QCCustomBrush is a stroke/fill brush with custom vertex and fragment shaders.
*/

/*!
    Constructs a default custom brush.
*/

QCCustomBrush::QCCustomBrush()
    : d(new QCCustomBrushPrivate)
{
}

/*!
    Constructs a custom brush.
    The fragment shader is \a fragmentShader and vertex shader is
    \a vertexShader.
*/

QCCustomBrush::QCCustomBrush(const QString &fragmentShader,
                             const QString &vertexShader)
    : d(new QCCustomBrushPrivate)
{
    setFragmentShader(fragmentShader);
    setVertexShader(vertexShader);
}

/*!
    Constructs a custom brush that is a copy of the given \a brush.
*/

QCCustomBrush::QCCustomBrush(const QCCustomBrush &brush) noexcept
    : d(brush.d)
{
}

/*!
    Destroys the custom brush.
*/

QCCustomBrush::~QCCustomBrush() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCCustomBrushPrivate)

/*!
    Assigns the given \a brush to this custom brush and returns a reference to
    this custom brush.
*/

QCCustomBrush &QCCustomBrush::operator=(const QCCustomBrush &brush) noexcept
{
    QCCustomBrush(brush).swap(*this);
    return *this;
}

/*!
    \fn QCCustomBrush::QCCustomBrush(QCCustomBrush &&other) noexcept

    Move-constructs a new QCCustomBrush from \a other.
*/

/*!
    \fn QCCustomBrush &QCCustomBrush::operator=(QCCustomBrush &&other)

    Move-assigns \a other to this QCCustomBrush instance.
*/

/*!
    \fn void QCCustomBrush::swap(QCCustomBrush &other)
    \memberswap{other}
*/

/*!
   Returns the custom brush as a QVariant.
*/

QCCustomBrush::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCCustomBrush::operator!=(const QCCustomBrush &brush) const

    Returns \c true if the custom brush is different from the given \a brush;
    otherwise false.

    \sa operator==()
*/

/*!
    \fn bool QCCustomBrush::operator==(const QCCustomBrush &brush) const

    Returns \c true if the custom brush is equal to the given \a brush;
    otherwise false.

    \sa operator!=()
*/

bool QCCustomBrush::operator==(const QCCustomBrush &b) const
{
    if (b.d == d)
        return true;

    if (d->fragmentShader != b.d->fragmentShader
        || d->vertexShader != b.d->vertexShader
        || d->timeRunning != b.d->timeRunning
        || d->time != b.d->time
        || d->globalAlpha != b.d->globalAlpha
        || d->data[0] != b.d->data[0]
        || d->data[1] != b.d->data[1]
        || d->data[2] != b.d->data[2]
        || d->data[3] != b.d->data[3])
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCCustomBrush &)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCCustomBrush()";
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*!
    Returns the type of brush, \c QCBrush::BrushType::Custom.
*/

QCBrush::BrushType QCCustomBrush::type() const
{
    return QCBrush::BrushType::Custom;
}

/*!
    Sets the custom brush to use \a fragmentShader.
    This must be path to a valid qsb file.
*/

void QCCustomBrush::setFragmentShader(const QString &fragmentShader)
{
    detach();
    d->fragmentShader = getShader(fragmentShader);
}

/*!
    Sets the custom brush to use \a vertexShader.
    This must be path to a valid qsb file.
*/

void QCCustomBrush::setVertexShader(const QString &vertexShader)
{
    detach();
    d->vertexShader = getShader(vertexShader);
}

/*!
    Returns true if the time is running.
*/

bool QCCustomBrush::timeRunning() const
{
    return d->timeRunning;
}

/*!
    Sets the time running state to \a running.
    When this is true, the shader uniform \c iTime
    value is animated to get the current animation running time.
    The default value is \c false.
*/

void QCCustomBrush::setTimeRunning(bool running)
{
    detach();
    d->timeRunning = running;
}

/*!
    Sets the uniform data1 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData1(const QVector4D &data)
{
    detach();
    d->data[0] = data;
}

/*!
    Sets the uniform data2 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData2(const QVector4D &data)
{
    detach();
    d->data[1] = data;
}

/*!
    Sets the uniform data3 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData3(const QVector4D &data)
{
    detach();
    d->data[2] = data;
}

/*!
    Sets the uniform data4 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData4(const QVector4D &data)
{
    detach();
    d->data[3] = data;
}

/*!
    \internal
*/

void QCCustomBrush::detach()
{
    if (d)
        d.detach();
    else
        d = new QCCustomBrushPrivate;
}

QT_END_NAMESPACE
