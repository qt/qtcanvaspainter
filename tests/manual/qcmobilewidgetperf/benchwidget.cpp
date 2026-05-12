// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "benchwidget.h"

#include <QCanvasPainter>
#include <QFont>

#include <rhi/qrhi.h>

using namespace Qt::StringLiterals;

static constexpr int COLS = 30;
static constexpr int ROWS = 20;

BenchWidget::BenchWidget(Api api)
{
    setApi(api);
    setFillColor(Qt::black);
    m_fpsTimer.start();
}

void BenchWidget::paint(QCanvasPainter *p)
{
    const float w = float(width());
    const float h = float(height());
    const float cellW = w / COLS;
    const float cellH = h / ROWS;
    const float pad = 2.0f;

    p->setLineWidth(1.5f);

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            const float x = col * cellW;
            const float y = row * cellH;

            // Filled rectangle
            const int fillHue = (m_frame + row * 13 + col * 7) % 360;
            p->setFillStyle(QColor::fromHsv(fillHue, 200, 200));
            p->fillRect(x + pad, y + pad, cellW - 2.0f * pad, cellH - 2.0f * pad);

            // Stroked rectangle, slightly inset
            const int strokeHue = (m_frame * 2 + row * 11 + col * 17 + 180) % 360;
            p->setStrokeStyle(QColor::fromHsv(strokeHue, 255, 255));
            p->strokeRect(x + pad * 3, y + pad * 3, cellW - 6.0f * pad, cellH - 6.0f * pad);
        }
    }

    if (m_apiName.isEmpty()) {
        if (QRhi *r = rhi())
            m_apiName = QString::fromLatin1(r->backendName());
    }

    QFont overlayFont;
    overlayFont.setPixelSize(18);
    overlayFont.setBold(true);
    p->setFont(overlayFont);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    p->setTextAlign(QCanvasPainter::TextAlign::Right);
    p->setFillStyle(QColor(Qt::white));
    const float overlayX = w - 8.0f;
    const float overlayY = h - 6.0f;
    p->fillText(u"FPS: %1"_s.arg(m_fps), overlayX, overlayY);
    p->fillText(u"3D API: %1"_s.arg(m_apiName), overlayX, overlayY - 22.0f);

    ++m_frame;
    ++m_fpsFrameCount;

    const qint64 elapsed = m_fpsTimer.elapsed();
    if (elapsed >= 1000) {
        m_fps = qRound(m_fpsFrameCount * 1000.0 / elapsed);
        window()->setWindowTitle(u"cpwidgettest — %1 fps"_s.arg(m_fps));
        qDebug("cpwidgettest - %d fps", m_fps);
        m_fpsFrameCount = 0;
        m_fpsTimer.restart();
    }

    update();
}
