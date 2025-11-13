// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcdebug_p.h"
#include "qcpainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

QCDebug::QCDebug()
    : m_debugNsElapsed(0)
    , m_debugCounter(0)
    , m_debugMsElapsed(QLatin1String("0.000"))
{
}

void QCDebug::start()
{
    m_debugTimer.start();
}

void QCDebug::paintDrawDebug(QCPainter *painter, float width, float height)
{
    if (!painter)
        return;

    auto *painterPriv = QCPainterPrivate::get(painter);
    if (!painterPriv->m_e)
        return;

    m_drawDebug = painterPriv->m_e->drawDebug();

    qint64 elapsed = m_debugTimer.nsecsElapsed();
    m_debugNsElapsed += elapsed;
    m_debugCounter++;
    if (!m_debugUpdateTimer.isValid())
        m_debugUpdateTimer.start();

    // How often time is updated, in seconds
    // Longer period increases accuracy
    const int UPDATE_FREQUENCY_MS = 1000;
    if (m_debugUpdateTimer.elapsed() >= UPDATE_FREQUENCY_MS) {
        qint64 ms = 1000000;
        double msElapsed = double(m_debugNsElapsed) / (ms * m_debugCounter);
        m_debugMsElapsed = QString::number(msElapsed, 'f', 3);
        m_debugNsElapsed = 0;
        m_debugCounter = 0;
        m_debugUpdateTimer.start();
    }
    //float fontSize = qMin(QCPainter::ptToPx(14), width*0.04f);
    float fontSize = 14.0f;
    float margin = fontSize * 0.2f;
    float debugHeight = fontSize * 5.0f + margin * 3.0f;
    float debugWidth = qMin(width, 28 * fontSize);
    float debugY = height - debugHeight;
    // Note: Returning these state settings back
    // shouldn't be needed as this debug is painted
    // as the very last thing.
    painter->resetTransform();
    painter->setAntialias(1.0f);
    painter->setGlobalCompositeOperation(QCPainter::CompositeOperation::SourceOver);
    painter->resetClipping();

    // Background
    painter->setFillStyle(QColor::fromRgba(0xB0000000));
    painter->fillRect(0, debugY, debugWidth, debugHeight);

    // Texts
    static const QString debugText1 = QStringLiteral(u"DRAWCALLS:");
    static const QString debugText2 = QStringLiteral(u"FILL");
    static const QString debugText3 = QStringLiteral(u"STROKE");
    static const QString debugText4 = QStringLiteral(u"TEXT");
    static const QString debugText5 = QStringLiteral(u"TOTAL");
    static const QString debugText6 = QStringLiteral(u"TRIANGLES:");
    QFont font;
    font.setPixelSize(fontSize);
    painter->setFont(font);
    painter->setTextAlign(QCPainter::TextAlign::Left);
    painter->setTextBaseline(QCPainter::TextBaseline::Top);
    painter->setFillStyle(0xFFFFFFFF);
    // Draw calls and triangles table
    float textY = debugY;
    float cellWidth = debugWidth / 6.0f;
    painter->fillText(debugText2, margin + 2.0f * cellWidth, textY);
    painter->fillText(debugText3, margin + 3.0f * cellWidth, textY);
    painter->fillText(debugText4, margin + 4.0f * cellWidth, textY);
    painter->fillText(debugText5, margin + 5.0f * cellWidth, textY);
    textY += fontSize + margin;
    painter->fillText(debugText1, margin, textY);
    painter->fillText(QString::number(m_drawDebug.fillDrawCallCount),
                      margin + 2.0f * cellWidth, textY);
    painter->fillText(QString::number(m_drawDebug.strokeDrawCallCount),
                      margin + 3.0f * cellWidth, textY);
    painter->fillText(QString::number(m_drawDebug.textDrawCallCount),
                      margin + 4.0f * cellWidth, textY);
    painter->fillText(QString::number(m_drawDebug.drawCallCount),
                      margin + 5.0f * cellWidth, textY);
    textY += fontSize + margin;
    painter->fillText(debugText6, margin, textY);
    painter->fillText(QString::number(m_drawDebug.fillTriangleCount),
                      margin + 2.0f * cellWidth, textY);
    painter->fillText(QString::number(m_drawDebug.strokeTriangleCount),
                      margin + 3.0f * cellWidth, textY);
    painter->fillText(QString::number(m_drawDebug.textTriangleCount),
                      margin + 4.0f * cellWidth, textY);
    painter->fillText(QString::number(m_drawDebug.triangleCount),
                      margin + 5.0f * cellWidth, textY);
    // Textures and timing info
    textY = height - fontSize * 1.5f;
    int textures = painter->cacheTextureAmount();
    int textureMem = painter->cacheMemoryUsage();
    QString debugText7 = QStringLiteral("IMAGES: %1, MEM: %2 kB, TIME: %3 ms")
                             .arg(textures).arg(textureMem).arg(m_debugMsElapsed);
    painter->fillText(debugText7, margin, textY);
}

QT_END_NAMESPACE
