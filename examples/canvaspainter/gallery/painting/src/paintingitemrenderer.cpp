// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "paintingitemrenderer.h"
#include "paintingitem.h"
#include <QPropertyAnimation>
#include "qcradialgradient.h"

Path::Path()
{
    // Animate opacity 1->0 after a short delay
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setDuration(2000);
    animation->setStartValue(1);
    animation->setEndValue(0);
    m_hideAnimation.addPause(5000);
    m_hideAnimation.addAnimation(animation);
}

Path::~Path()
{
}

PaintingItemRenderer::PaintingItemRenderer()
{
}

void PaintingItemRenderer::synchronize(QQuickCPainterItem *item)
{
    PaintingItem *realItem = static_cast<PaintingItem*>(item);
    if (realItem) {

        // New line?
        if (realItem->m_pointsSynced == 0) {
            // Start new path
            QSharedPointer<Path> newPath = QSharedPointer<Path>::create();
            m_paths.append(newPath);
        }

        auto path = m_paths.last().data();

        // Line drawing has ended?
        if (realItem->m_points.count() > 0 && realItem->m_points.last().x() == INT_MAX) {
            QObject::connect(&path->m_hideAnimation, &QAbstractAnimation::finished, [=](){
                // When hide animation finishes, remove path
                m_paths.removeFirst();
            });
            path->m_hideAnimation.start();
            // Last point is used just to inform line has ended, so remove
            realItem->m_points.removeLast();
        }

        // Sync all new points into latest path
        for (int i=realItem->m_pointsSynced; i<realItem->m_points.count(); ++i) {
            path->m_points.append(realItem->m_points.at(i));
        }
        realItem->m_pointsSynced = path->m_points.count();
    }
}

void PaintingItemRenderer::paint(QCPainter *painter)
{
    // Background
    QCRadialGradient grad;
    grad.setStartColor("#808080");
    grad.setEndColor("#606060");
    grad.setCenterPosition(width()/2, height()/2);
    grad.setOuterRadius(qMax(width(), height())*0.6f);
    painter->setFillStyle(grad);
    painter->setStrokeStyle("#F0F0F0");
    painter->setLineWidth(1);
    painter->beginPath();
    painter->rect(0, 0, width(), height());
    painter->fill();
    painter->stroke();

    // Draw painting line
    if (m_paths.size() > 0) {
        // Set this on so overlapping fading lines don't
        // show rendering artifacts.
        painter->setRenderHint(QCPainter::RenderHint::HighQualityStroking);
        painter->setLineCap(QCPainter::LineCap::Round);
        painter->setLineJoin(QCPainter::LineJoin::Round);
        float lineWidth = QCPainter::mmToPx(2);
        painter->setLineWidth(lineWidth);

        for (int i=0; i<m_paths.count(); ++i) {
            auto item = m_paths.at(i).data();
            auto points = item->m_points;
            if (points.count() > 0) {
                QColor shadowColor("#202020");
                shadowColor.setAlpha(item->opacity() * 200);
                painter->setStrokeStyle(shadowColor);
                painter->setAntialias(10.0);
                drawPathLine(points, QPoint(lineWidth*0.2, lineWidth*0.2));

                QColor lineColor("#F0D060");
                lineColor.setAlpha(item->opacity() * 255);
                painter->setStrokeStyle(lineColor);
                painter->setAntialias(2.0);
                drawPathLine(points, QPoint(0,0));

                // If animation is still running, update
                if (item->m_hideAnimation.state() == QAbstractAnimation::Running) {
                    Q_EMIT update();
                }
            }
        }
    }
}

void PaintingItemRenderer::drawPathLine(const QVector<QPoint> &points, const QPoint translate) {
    painter()->beginPath();
    QPoint firstPoint = points.first() + translate;
    painter()->moveTo(firstPoint.x(), firstPoint.y()-1);
    painter()->lineTo(firstPoint);
    for (int i=1; i<points.size(); ++i) {
        painter()->lineTo(points.at(i) + translate);
    }
    painter()->stroke();
}
