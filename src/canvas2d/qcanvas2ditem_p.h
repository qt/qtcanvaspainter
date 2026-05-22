// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DITEM_P_H
#define QCANVAS2DITEM_P_H

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

#include "qtcanvas2dglobal_p.h"
#include "qcanvaspainteritem.h"
#include <QtCanvasPainter/qcanvaspainter.h>
#include <QtCanvasPainter/qcanvasimagepattern.h>
#include <QtQml/private/qqmlrefcount_p.h>
#include <QtGui/qimage.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>

QT_BEGIN_NAMESPACE

class QCanvas2DContext;

class QCanvas2DItemPrivate;
class QQuickPixmap;
class QCanvas2DCommandBuffer;

class Q_CANVAS2D_EXPORT QCanvas2DPixmap final : public QQmlRefCounted<QCanvas2DPixmap>
{
public:
    QCanvas2DPixmap(const QImage& image);
    QCanvas2DPixmap(QQuickPixmap *pixmap);
    ~QCanvas2DPixmap();

    QImage image();

    qreal width() const;
    qreal height() const;
    bool isValid() const;
    QQuickPixmap *pixmap() const { return m_pixmap;}

private:
    QQuickPixmap *m_pixmap;
    QImage m_image;
};

class Q_CANVAS2D_EXPORT QCanvas2DItem : public QCanvasPainterItem
{
    Q_OBJECT
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged FINAL)
    Q_PROPERTY(QString contextType READ contextType WRITE setContextType NOTIFY contextTypeChanged FINAL)
    Q_PROPERTY(QJSValue context READ context NOTIFY contextChanged FINAL)
    QML_NAMED_ELEMENT(Canvas2D)

public:
    QCanvas2DItem(QQuickItem *parent = nullptr);
    ~QCanvas2DItem();

    // Managing QCanvasImages for image patterns.
    struct ImageData {
        QCanvasImagePattern pattern;
        QImage image;
        QCanvasPainter::ImageFlags flags;
        QString url;
    };

    bool isAvailable() const;

    QString contextType() const;
    void setContextType(const QString &contextType);

    QJSValue context() const;

    Q_INVOKABLE void getContext(QQmlV4FunctionPtr args);

    Q_INVOKABLE void requestAnimationFrame(QQmlV4FunctionPtr args);
    Q_INVOKABLE void cancelRequestAnimationFrame(QQmlV4FunctionPtr args);

    Q_INVOKABLE void requestPaint();
    Q_INVOKABLE void markDirty();

    QQmlRefPointer<QCanvas2DPixmap> loadedPixmap(const QUrl& url, QSizeF sourceSize = QSizeF());

    QCanvas2DCommandBuffer *ccb() const;
    void setCcb(QCanvas2DCommandBuffer *ccb);
    QCanvasPainterItemRenderer *createItemRenderer() const override;
    void addImagePattern(const QCanvasImagePattern &pattern, const QString &url, const QImage &image, QCanvasPainter::ImageFlags flags);
    QHash<uint, ImageData> imageData() const;
    void clearImageDataCache();

Q_SIGNALS:
    void paint();
    void painted();
    void availableChanged();
    void contextTypeChanged();
    void contextChanged();
    void imageLoaded();

public Q_SLOTS:
    void loadImage(const QUrl& url, QSizeF sourceSize = QSizeF());
    void unloadImage(const QUrl& url);
    bool isImageLoaded(const QUrl& url) const;
    bool isImageLoading(const QUrl& url) const;
    bool isImageError(const QUrl& url) const;

private Q_SLOTS:
    void sceneGraphInitialized();
    void checkAnimationCallbacks();
    void invalidateSceneGraph();
    void schedulePolish();

protected:
    void componentComplete() override;
    void itemChange(QQuickItem::ItemChange, const QQuickItem::ItemChangeData &) override;
    void updatePolish() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void releaseResources() override;
    bool event(QEvent *event) override;

private:
    Q_DECLARE_PRIVATE(QCanvas2DItem)
    Q_INVOKABLE void delayedCreate();
    bool createContext(const QString &contextType);
    void initializeContext(QCanvas2DContext *context, const QVariantMap &args = QVariantMap());
    bool isPaintConnected();
};

QT_END_NAMESPACE

#endif //QCANVAS2DITEM_P_H
