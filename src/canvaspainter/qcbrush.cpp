// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcbrush.h"
#include "engine/qcpainterengineutils_p.h"
#include "qdatastream.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCBrush
    \brief QCBrush is the base class for all QCPainter fill / stroke brushes.
    \inmodule QtCanvasPainter

    QCBrush is the base class for all styles used for \l QCPainter::fill()
    and \l QCPainter::stroke().
*/

/*!
    \enum QCBrush::BrushType

    Specifies the type of brush.

    \value Brush - Empty brush.

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
    Subclasses override this method.
*/

QCBrush::BrushType QCBrush::type() const
{
    return QCBrush::BrushType::Brush;
}

// ***** Private *****

/*!
   \internal
*/
QCBrush::~QCBrush()
{
}

/*!
   \internal
*/
QCPaint QCBrush::createPaint(QCPainter *painter) const
{
    Q_UNUSED(painter)
    QCPaint empty;
    return empty;
}

QT_END_NAMESPACE
