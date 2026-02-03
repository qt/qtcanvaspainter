// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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

#ifndef QCPAINTER_P_H
#define QCPAINTER_P_H

#include "qcpainter.h"
#include "qcimage.h"
#include <QtGui/qfont.h>
#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include <QtCore/qset.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

class QCPainterEngine;
class QCPainterRhiRenderer;

Q_DECLARE_LOGGING_CATEGORY(QC_INFO)

// Keeps count of the texture id's and the total size of textures.
// This doesn't cache the actual texture data.
// Only used for QCImages created from QImage (gradient or user-supplied).
// Does not track QCImages created from native textures (offscreen canvas or user-supplied).
class QCImageTracker
{
public:
    inline bool contains(qint64 key) const { return m_data.contains(key); }
    inline QCImage image(qint64 key) const { return m_data.value(key); }
    inline int textureId(qint64 key) const { return m_data.value(key).id(); }
    inline void insert(qint64 key, const QCImage &image)
    {
        m_dataAmount += image.size();
        m_data.insert(key, image);
    }
    inline qsizetype dataAmount() const { return m_dataAmount; }
    inline qsizetype size() const { return m_data.size(); }
    void removeTemporaryResources();
    void removeTextureId(int imageId);
    void handleRemoveTextures();
    void markTextureIdUsed(int imageId);
    void clear();

private:
    friend class QCPainterPrivate;
    QCPainterPrivate *m_painterPrivate = nullptr;
    QHash<qint64, QCImage> m_data;
    QList<QCImage> m_cleanupTextures;
    QList<int> m_usedTextureIDs;
    qsizetype m_dataAmount = 0;
    bool m_doingResourcesRemoval = false;
};

class QRhi;

class QCPainterPrivate
{
public:
    QCPainterPrivate();
    ~QCPainterPrivate();
    QCPainterEngine *engine() const;

    static QCPainterPrivate *get(QCPainter *painter) { return painter->d_func(); }
    static const QCPainterPrivate *get(const QCPainter *painter) { return painter->d_func(); }

    qint64 generateImageKey(const QImage &image, QCPainter::ImageFlags flags) const;
    QCImage getQCImage(const QImage &image, QCPainter::ImageFlags flags, qint64 imageKey = 0);
    QCImage getQCImage(QRhiTexture *texture, QCPainter::ImageFlags flags);
    QCImage getQCImage(const QCOffscreenCanvas &canvas, QCPainter::ImageFlags flags);
    void drawImageId(int imageId, float x, float y, float width, float height, const QColor &tintColor);
    void handleCleanupTextures();
    void clearTextureCache();
    void markTextureIdUsed(int imageId);

    void setFont(const QFont &font);
    void fillText(const QString &text, float x, float y, float maxWidth = -1);
    void fillText(const QString &text, const QRectF &rect);
    QRectF textBoundingBox(const QString &text, float x, float y, float maxWidth = -1);
    QRectF textBoundingBox(const QString &text, const QRectF &rect);

    QCImageTracker m_imageTracker;
    QHash<quint64, QCImage> m_nativeTextureCache; // QRhiTexture::globalResourceId -> QCImage
    QSet<int> m_pendingNativeTextureDelete;
    float m_devicePixelRatio = 1.0f;
    QCPainterRhiRenderer *m_renderer = nullptr;
    QCPainterEngine *m_e = nullptr;
    int m_maxTextures = 0;
    bool m_trackingDisabled = false;
};

QT_END_NAMESPACE

#endif // QCPAINTER_P_H
