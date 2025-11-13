// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PAINTINGITEMRENDERER_H
#define PAINTINGITEMRENDERER_H

#include "qquickcpainterrenderer.h"
#include "qcpainter.h"
#include <QQueue>
#include <QVector>
#include <QSequentialAnimationGroup>

class Path : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
public:
    Path();
    ~Path();

    float opacity() {
        return m_opacity;
    }
    void setOpacity(float opacity) {
        m_opacity = opacity;
        Q_EMIT opacityChanged();
    }


Q_SIGNALS:
    void opacityChanged();

private:
    friend class PaintingItemRenderer;
    float m_opacity = 1.0f;
    QVector<QPoint> m_points;
    QSequentialAnimationGroup m_hideAnimation;
};

class PaintingItemRenderer: public QObject, public QQuickCPainterRenderer
{
    Q_OBJECT
public:
    PaintingItemRenderer();

    // Reimplement
    void synchronize(QQuickCPainterItem *item);
    void paint(QCPainter *painter);

Q_SIGNALS:
    void update();

private:
    void drawPathLine(const QVector<QPoint> &points, const QPoint translate);
    QQueue<QSharedPointer<Path> > m_paths;

};

#endif // PAINTINGITEMRENDERER_H
