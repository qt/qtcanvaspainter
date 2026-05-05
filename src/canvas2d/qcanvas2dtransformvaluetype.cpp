// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dtransformvaluetype_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlvaluetype transform2d
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides transform2d type,a 3x3 2D transformation matrix,
    matching to QTransform.

    A \c transform2d type has nine values, each accessible via the properties
    \c m11 through \c m33 in QML (in row/column order).

    A property of type \c transform2d defaults to the identity matrix, whose
    diagonal entries \c m11, \c m22 and \c m33 are all \c 1, with all
    other components \c 0.

    This value type is provided by the \l{QtCanvas2D} import.

    \sa QTransform
*/

/*!
    \qmlmethod real transform2d::m11()

    Returns the horizontal scaling factor.
*/

qreal QCanvas2DTransformValueType::m11() const
{
    return QTransform::m11();
}

/*!
    \qmlmethod real transform2d::m12()

    Returns the vertical shearing factor.
*/

qreal QCanvas2DTransformValueType::m12() const
{
    return QTransform::m12();
}

/*!
    \qmlmethod real transform2d::m13()

    Returns the horizontal projection factor.
*/

qreal QCanvas2DTransformValueType::m13() const
{
    return QTransform::m13();
}

/*!
    \qmlmethod real transform2d::m21()

    Returns the horizontal shearing factor.
*/

qreal QCanvas2DTransformValueType::m21() const
{
    return QTransform::m21();
}

/*!
    \qmlmethod real transform2d::m22()

    Returns the vertical scaling factor.
*/

qreal QCanvas2DTransformValueType::m22() const
{
    return QTransform::m22();
}

/*!
    \qmlmethod real transform2d::m23()

    Returns the vertical projection factor.
*/

qreal QCanvas2DTransformValueType::m23() const
{
    return QTransform::m23();
}

/*!
    \qmlmethod real transform2d::m31()

    Returns the horizontal translation factor.
*/

qreal QCanvas2DTransformValueType::m31() const
{
    return QTransform::m31();
}

/*!
    \qmlmethod real transform2d::m32()

    Returns the vertical translation factor.
*/

qreal QCanvas2DTransformValueType::m32() const
{
    return QTransform::m32();
}

/*!
    \qmlmethod real transform2d::m33()

    Returns the division factor.
*/

qreal QCanvas2DTransformValueType::m33() const
{
    return QTransform::m33();
}

/*!
    \qmlmethod void transform2d::setMatrix(real m11, real m12, real m13,
                                      real m21, real m22, real m23,
                                      real m31, real m32, real m33)

    Sets the matrix elements to the specified values, \a m11, \a m12, \a m13 \a m21,
    \a m22, \a m23 \a m31, \a m32 and \a m33. Note that this function replaces the
    previous values.
*/

void QCanvas2DTransformValueType::setMatrix(qreal m11, qreal m12, qreal m13,
                                            qreal m21, qreal m22, qreal m23,
                                            qreal m31, qreal m32, qreal m33)
{
    QTransform::setMatrix(m11, m12, m13,
                          m21, m22, m23,
                          m31, m32, m33);
}

/*!
    \qmlmethod void transform2d::setMatrix(real m11, real m12,
                                           real m21, real m22,
                                           real m31, real m32)

    Sets the matrix elements to the specified values, \a m11, \a m12, \a m21,
    \a m22, \a m31 and \a m32. Note that this function replaces the
    previous values.
*/

void QCanvas2DTransformValueType::setMatrix(qreal m11, qreal m12,
                                            qreal m21, qreal m22,
                                            qreal m31, qreal m32)
{
    QTransform::setMatrix(m11, m12, 0,
                          m21, m22, 0,
                          m31, m32, 1);
}

/*!
    \qmlmethod void transform2d::translate(real dx, qreal dy)

    Moves the coordinate system \a dx along the x axis and \a dy along the y axis.
*/

void QCanvas2DTransformValueType::translate(float dx, float dy)
{
    QTransform::translate(dx, dy);
}

/*!
    \qmlmethod void transform2d::rotate(real angle)

    Rotates the coordinate system counterclockwise by the given \a angle in degrees.
*/

void QCanvas2DTransformValueType::rotate(float angle)
{
    QTransform::rotate(angle);
}

/*!
    \qmlmethod void transform2d::rotateRadians(real angle)

    Rotates the coordinate system counterclockwise by the given \a angle in radians.
*/

void QCanvas2DTransformValueType::rotateRadians(float angle)
{
    QTransform::rotateRadians(angle);
}

/*!
    \qmlmethod void transform2d::scale(real scale)

    Scales the coordinate system by \a scale both horizontally and vertically.
*/

void QCanvas2DTransformValueType::scale(float s)
{
    QTransform::scale(s, s);
}

/*!
    \qmlmethod void transform2d::scale(real sx, real sy)

    Scales the coordinate system by \a sx horizontally and \a sy vertically.
*/

void QCanvas2DTransformValueType::scale(float sx, float sy)
{
    QTransform::scale(sx, sy);
}

/*!
    \qmlmethod void transform2d::shear(real sh, real sv)

    Shears the coordinate system by \a sh horizontally and \a sv vertically.
*/

void QCanvas2DTransformValueType::shear(float sh, float sv)
{
    QTransform::shear(sh, sv);
}

/*!
    \qmlmethod transform2d transform2d::times(transform2d other)

    Returns the transform2d result of multiplying this transform2d with
    the \a other transform2d.
*/

QTransform QCanvas2DTransformValueType::times(const QTransform &m) const
{
    return *this * m;
}

/*!
    \qmlmethod transform2d transform2d::times(real factor)

    Returns the transform2d result of multiplying this transform2d with
    the scalar \a factor.
*/

QTransform QCanvas2DTransformValueType::times(qreal factor) const
{
    return *this * factor;
}

/*!
    \qmlmethod real transform2d::determinant()

    Returns the determinant of this transform2d.
*/

qreal QCanvas2DTransformValueType::determinant() const
{
    return QTransform::determinant();
}

/*!
    \qmlmethod transform2d transform2d::inverted()

    Returns the inverse of this transform2d if it exists, else the identity matrix.
*/

QTransform QCanvas2DTransformValueType::inverted() const
{
    return QTransform::inverted();
}

/*!
    \qmlmethod transform2d transform2d::transposed()

    Returns the transpose of this transform2d.
*/

QTransform QCanvas2DTransformValueType::transposed() const
{
    return QTransform::transposed();
}

QT_END_NAMESPACE
