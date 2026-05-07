// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dbrushvaluetype_p.h"
#include "qcanvas2dutils_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlvaluetype lineargradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides lineargradient2d type matching to QCanvasLinearGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasLinearGradient
*/


/*!
    \qmlmethod void lineargradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createLinearGradient(0, 0, 100, 100);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DLinearGradientValueType::addColorStop(float offset, const QColor &color)
{
    QCanvasLinearGradient::addColorStop(offset, color);
}

/*!
    \qmlvaluetype radialgradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides radialgradient2d type matching to QCanvasRadialGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasRadialGradient
*/


/*!
    \qmlmethod void radialgradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createRadialGradient(100, 1000, 10, 100, 100, 80);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DRadialGradientValueType::addColorStop(float offset, const QColor &color)
{
    QCanvasRadialGradient::addColorStop(offset, color);
}

/*!
    \qmlvaluetype conicalgradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides conicalgradient2d type matching to QCanvasConicalGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasConicalGradient
*/


/*!
    \qmlmethod void conicalgradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createConicalGradient(100, 100, Math.PI / 2);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DConicalGradientValueType::addColorStop(float offset, const QColor &color)
{
    QCanvasConicalGradient::addColorStop(offset, color);
}

/*!
    \qmlvaluetype boxgradient2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides boxgradient2d type matching to QCanvasBoxGradient.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasBoxGradient
*/


/*!
    \qmlmethod void boxgradient2d::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createBoxGradient(50, 50, 100, 100, 20, 10);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(0.5, "#ff8040");
    gradient.addColorStop(1.0, "red");
    \endcode
*/

void QCanvas2DBoxGradientValueType::addColorStop(float offset, const QColor &color)
{
    QCanvasBoxGradient::addColorStop(offset, color);
}

/*!
    \qmlvaluetype gridpattern2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides gridpattern2d type matching to QCanvasGridPattern.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasGridPattern
*/

/*!
    \qmlvaluetype boxshadow2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides boxshadow2d type matching to QCanvasBoxShadow.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasBoxShadow
*/

/*!
    \qmlvaluetype imagepattern2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides imagepattern2d type matching to QCanvasImagePattern.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QCanvasImagePattern
*/

QT_END_NAMESPACE
