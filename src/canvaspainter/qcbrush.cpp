// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcbrush.h"
#include "qcbrush_p.h"
#include "engine/qcpainterengineutils_p.h"
#include "qdatastream.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCBrush
    \since 6.11
    \brief QCBrush is the base class for all QCPainter fill / stroke brushes.
    \inmodule QtCanvasPainter

    QCBrush is the base class for all styles used for \l QCPainter::fill()
    and \l QCPainter::stroke().
*/

/*!
   Constructs an invalid brush.
*/

QCBrush::QCBrush()
{
}

/*!
    Constructs a brush that is a copy of the given \a brush.
*/
QCBrush::QCBrush(const QCBrush &brush) noexcept
    : baseData(brush.baseData)
{
}

QCBrush::~QCBrush() = default;

/*!
    Assigns the given \a brush to this brush and returns a reference to
    this brush.
*/

QCBrush &QCBrush::operator=(const QCBrush &brush) noexcept
{
    QCBrush(brush).swap(*this);
    return *this;
}

/*!
    \fn QCBrush::QCBrush(QCImage &&other) noexcept

    Move-constructs a new QCBrush from \a other.
*/

/*!
    \fn QCBrush &QCBrush::operator=(QCBrush &&other)

    Move-assigns \a other to this QCBrush instance.
*/

/*!
    \fn void QCBrush::swap(QCBrush &other)
    \memberswap{brush}
*/


/*!
    \enum QCBrush::BrushType

    Specifies the type of brush.

    \value Invalid - Empty brush.

    \value LinearGradient - Interpolates colors between start and end points
    (QCLinearGradient)

    \value RadialGradient - Interpolates colors between a focal point and end
    points on a circle surrounding it (QCRadialGradient).

    \value ConicalGradient - Interpolates colors around a center point
    (QCConicalGradient).

    \value BoxGradient - Interpolates colors on a round rectangle
    (QCBoxGradient).

    \value BoxShadow - Creates a soft round rectangle shadow (QCBoxShadow).

    \value ImagePattern - Creates a pattern using the specified image and
    repetition (QCImagePattern).

    \value GridPattern - Creates a pattern using the specified grid (QCGridPattern).

    \value Custom - Creates a custom shader brush (QCCustomBrush).

    \sa type()
*/

/*!
    Returns the type of the brush.
*/

QCBrush::BrushType QCBrush::type() const
{
    return baseData ? baseData->type : QCBrush::BrushType::Invalid;
}

// ***** Private *****

/*!
   \internal
*/
void QCBrush::detach()
{
    if (baseData)
        baseData.detach();
}

QCBrush::QCBrush(QCBrushPrivate *priv)
    : baseData(priv)
{
}

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCBrushPrivate);

/*!
   \internal
*/
QCPaint QCBrush::createPaint(QCPainter *painter) const
{
    Q_UNUSED(painter)
    if (baseData)
        return baseData->createPaint(painter);
    QCPaint empty;
    return empty;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCBrush &b)
{
    QDebugStateSaver saver(dbg);
    const auto t = b.type();
    dbg.nospace() << "QCBrush(" << t << ")";
    return dbg;
}
#endif
QT_END_NAMESPACE
