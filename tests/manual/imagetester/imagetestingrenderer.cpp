// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "imagetestingrenderer.h"
#include "imagetestingitem.h"
#include <QColor>
#include <QRandomGenerator>
#include <QTimer>
#include "qclineargradient.h"

ImageTestingRenderer::ImageTestingRenderer()
{
}

void ImageTestingRenderer::synchronize(QQuickCPainterItem *item)
{
    auto p = painter();
    ImageTestingItem *realItem = static_cast<ImageTestingItem*>(item);
    auto actions = realItem->m_actions;
    bool memDataUpdated = false;
    if (actions.testFlag(ImageTestingItem::Action::Generate)) {
        generateImage();
        memDataUpdated = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveAll)) {
        int clearedImages = m_images.size();
        if (clearedImages > 0) {
            for (int i = 0; i < clearedImages; i++) {
                auto image = m_images.takeFirst().image;
                p->removeImage(image);
            }
            memDataUpdated = true;
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveHidden)) {
        auto it = m_images.begin();
        while (it != m_images.end()) {
            if (!it->visible) {
                p->removeImage(it->image);
                it = m_images.erase(it);
            } else {
                it++;
            }
        }
        memDataUpdated = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::ShowHidden)) {
        for (int i = 0; i < m_images.size(); i++)
            m_images[i].visible = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveNewest)) {
        if (!m_images.isEmpty()) {
            auto image = m_images.takeLast().image;
            p->removeImage(image);
            memDataUpdated = true;
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::HideNewest)) {
        for (int i = m_images.size()-1; i >= 0; i--) {
            if (m_images.at(i).visible) {
                m_images[i].visible = false;
                break;
            }
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::CleanupResources)) {
        p->cleanupResources();
        memDataUpdated = true;
    }
    m_showGradient = realItem->m_showGradient;
    m_animateGradient = realItem->m_animateGradient;

    // Update data values for the item
    realItem->m_dataAmount = p->cacheMemoryUsage();
    realItem->dataAmountChanged();
    realItem->m_imageAmount = p->cacheTextureAmount();
    realItem->imageAmountChanged();
    if (memDataUpdated) {
        // This is needed for cache amounts to update from the item side
        QTimer::singleShot(0, realItem, [realItem]{realItem->update();});
    }
    realItem->m_actions = {};
}

void ImageTestingRenderer::paint(QCPainter *painter)
{
    const float w = width();
    const float h = height();
    // Render all visible images
    int imageCount = m_images.size();
    const float imageW = w * 0.5;
    const float imageH = h * 0.5;
    int index = 0;
    painter->setTextAlign(QCPainter::TextAlign::Center);
    painter->setTextBaseline(QCPainter::TextBaseline::Top);
    QFont font;
    float fontSize = imageH * 0.08;
    font.setPixelSize(fontSize);
    painter->setFont(font);
    for (auto &image : std::as_const(m_images)) {
        if (image.visible) {
            const float movPos = (float(index + 0.5) / imageCount - 0.5);
            const float movX = imageW * movPos;
            const float movY = imageH * movPos;
            QRectF pos(w * 0.5 - imageW * 0.5 + movX,
                       h * 0.5 - imageH * 0.5 + movY,
                       imageW, imageH);
            painter->drawImage(image.image, pos);
            // Paint image index
            painter->fillText(QStringLiteral("INDEX: %1").arg(QString::number(index)), pos);
        }
        index++;
    }

    static int anim = 0;
    if (m_showGradient) {
        float gradW = w * 0.8;
        float gradH = h * 0.05;
        float gradX = (w - gradW) * 0.5;
        float gradY = 0;
        QRectF rect(gradX, gradY, gradW, gradH);
        QCLinearGradient g(rect.x(), rect.y(), rect.x()+rect.width(), rect.y());
        g.setColorAt(0.0f, QColorConstants::Black);
        if (m_animateGradient)
            anim++;
        float gradPos = 0.5f + 0.5f * qSin(0.005 * anim);
        g.setColorAt(gradPos, QColorConstants::Red);
        g.setColorAt(1.0f, QColorConstants::Black);
        painter->setFillStyle(g);
        painter->fillRect(rect);
    }
}

void ImageTestingRenderer::generateImage()
{
    QImage image(1024, 1024, QImage::Format_RGBA8888);
    image.fill(QColorConstants::Black);
    auto *rand =  QRandomGenerator::global();
    int g = rand->generate() % 255;
    int b = rand->generate() % 255;
    int r = 0;
    for (int y = 0; y < image.height(); y++) {
        if (y % 10 == 0)
            r = rand->generate() % 255;
        for (int x = 0; x < image.width(); x++)
            image.setPixel(x, y, qRgba(r, g, b, 100));
    }
    ImageData imageData;
    imageData.image = painter()->addImage(image);
    m_images << imageData;
}
