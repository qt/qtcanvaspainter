// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCQUICKITEM_H
#define QCQUICKITEM_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtQuick/qquickrhiitem.h>
#include <QtGui/qcolor.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCanvasPainter/qcoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QQuickCPainterRenderer;
class QQuickCPainterItemPrivate;

class Q_CANVASPAINTER_EXPORT QQuickCPainterItem : public QQuickRhiItem
{
    Q_OBJECT
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged FINAL)
    Q_PROPERTY(QVariantMap debug READ debug NOTIFY debugChanged FINAL)

public:

    QQuickCPainterItem(QQuickItem *parent = nullptr);
    ~QQuickCPainterItem() override;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    QVariantMap debug();

protected:
    virtual QQuickCPainterRenderer* createItemRenderer() const = 0;

    QQuickRhiItemRenderer *createRenderer() final;

Q_SIGNALS:
    void fillColorChanged();
    void debugChanged();

private:
    friend class QQuickCPainterRenderer;
    Q_DECLARE_PRIVATE(QQuickCPainterItem)
};

QT_END_NAMESPACE

#endif // QCQUICKITEM_H
