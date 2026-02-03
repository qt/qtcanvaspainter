// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCIMAGE_P_H
#define QCIMAGE_P_H

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

#include <QtCore/qshareddata.h>
#include <qcolor.h>

QT_BEGIN_NAMESPACE

class QCImagePrivate : public QSharedData
{
public:
    QCImagePrivate();

    static QCImagePrivate *get(QCImage *image) { return image->d.get(); }
    static const QCImagePrivate *get(const QCImage *image) { return image->d.get(); }

    enum class DataType {
        Unknown,
        GradientTextureFromImage,
        TextureFromImage,
        ImportedTexture
    };

    int id;
    int width;
    int height;
    DataType type;
    qsizetype size;
    QColor tintColor;
};

QT_END_NAMESPACE

#endif // QCIMAGE_P_H
