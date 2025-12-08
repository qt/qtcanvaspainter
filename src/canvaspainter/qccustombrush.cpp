// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qccustombrush.h"
#include "engine/qcpainterengineutils_p.h"
#include "qccustombrush_p.h"
#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE

#define C_D() auto *d = static_cast<QCCustomBrushPrivate*>(baseData.get())
#define C_CD() const auto *d = static_cast<QCCustomBrushPrivate*>(baseData.get())

static QShader getCustomShader(const QString &name)
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
    : QCBrush(new QCCustomBrushPrivate)
{
}

/*!
    Constructs a custom brush.
    The fragment shader is \a fragmentShader and vertex shader is
    \a vertexShader.
*/

QCCustomBrush::QCCustomBrush(const QString &fragmentShader,
                             const QString &vertexShader)
    : QCBrush(new QCCustomBrushPrivate)
{
    setFragmentShader(fragmentShader);
    setVertexShader(vertexShader);
}


/*!
    Destroys the custom brush.
*/

QCCustomBrush::~QCCustomBrush() = default;


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
    if (b.baseData == baseData)
        return true;

    auto *d = QCCustomBrushPrivate::get(this);
    auto *bd = QCCustomBrushPrivate::get(&b);
    if (d->fragmentShader != bd->fragmentShader
        || d->vertexShader != bd->vertexShader
        || d->timeRunning != bd->timeRunning
        || d->time != bd->time
        || d->data[0] != bd->data[0]
        || d->data[1] != bd->data[1]
        || d->data[2] != bd->data[2]
        || d->data[3] != bd->data[3])
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
    Sets the custom brush to use \a fragmentShader.
    This must be path to a valid qsb file.
*/

void QCCustomBrush::setFragmentShader(const QString &fragmentShader)
{
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->fragmentShader = getCustomShader(fragmentShader);
}

/*!
    Sets the custom brush to use \a vertexShader.
    This must be path to a valid qsb file.
*/

void QCCustomBrush::setVertexShader(const QString &vertexShader)
{
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->vertexShader = getCustomShader(vertexShader);
}

/*!
    Returns true if the time is running.
*/

bool QCCustomBrush::timeRunning() const
{
    auto *d = QCCustomBrushPrivate::get(this);
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
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->timeRunning = running;
}

/*!
    Sets the uniform data1 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData1(const QVector4D &data)
{
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->data[0] = data;
}

/*!
    Sets the uniform data2 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData2(const QVector4D &data)
{
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->data[1] = data;
}

/*!
    Sets the uniform data3 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData3(const QVector4D &data)
{
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->data[2] = data;
}

/*!
    Sets the uniform data4 value to \a data.
    This allows setting custom data into shaders.
*/

void QCCustomBrush::setData4(const QVector4D &data)
{
    auto *d = QCCustomBrushPrivate::get(this);
    detach();
    d->data[3] = data;
}

QCBrushPrivate *QCCustomBrushPrivate::clone()
{
    return new QCCustomBrushPrivate(*this);
}

QCPaint QCCustomBrushPrivate::createPaint(QCPainter *) const
{
    QCPaint empty;
    return empty;
}

QT_END_NAMESPACE
