// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DTRANSFORMVALUETYPE_P_H
#define QCANVAS2DTRANSFORMVALUETYPE_P_H

#include "qtcanvas2dglobal_p.h"
#include <QtGui/qtransform.h>
#include <QtQml/private/qqmlvaluetype_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class Q_CANVAS2D_EXPORT QCanvas2DTransformValueType : private QTransform
{
    Q_GADGET
    QML_ADDED_IN_VERSION(6, 12)
    QML_FOREIGN(QTransform)
    QML_VALUE_TYPE(transform2d)
    QML_EXTENDED(QCanvas2DTransformValueType)
    QML_STRUCTURED_VALUE

public:
    Q_INVOKABLE QCanvas2DTransformValueType() = default;

    Q_INVOKABLE qreal m11() const;
    Q_INVOKABLE qreal m12() const;
    Q_INVOKABLE qreal m13() const;
    Q_INVOKABLE qreal m21() const;
    Q_INVOKABLE qreal m22() const;
    Q_INVOKABLE qreal m23() const;
    Q_INVOKABLE qreal m31() const;
    Q_INVOKABLE qreal m32() const;
    Q_INVOKABLE qreal m33() const;

    Q_INVOKABLE void setMatrix(qreal m11, qreal m12, qreal m13,
                               qreal m21, qreal m22, qreal m23,
                               qreal m31, qreal m32, qreal m33);
    Q_INVOKABLE void setMatrix(qreal m11, qreal m12,
                               qreal m21, qreal m22,
                               qreal m31, qreal m32);
    Q_INVOKABLE void translate(float dx, float dy);
    Q_INVOKABLE void rotate(float angle);
    Q_INVOKABLE void rotateRadians(float angle);
    Q_INVOKABLE void scale(float s);
    Q_INVOKABLE void scale(float sx, float sy);
    Q_INVOKABLE void shear(float sh, float sv);

    Q_INVOKABLE QTransform times(const QTransform &m) const;
    Q_INVOKABLE QTransform times(qreal factor) const;

    Q_INVOKABLE qreal determinant() const;
    Q_INVOKABLE QTransform inverted() const;
    Q_INVOKABLE QTransform transposed() const;
    Q_INVOKABLE void reset();
};

QT_END_NAMESPACE

#endif // QCANVAS2DTRANSFORMVALUETYPE_P_H
