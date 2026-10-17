// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASCUSTOMBRUSH_P_H
#define QCANVASCUSTOMBRUSH_P_H

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

#include "qcanvasbrush_p.h"
#include "qcanvascustombrush.h"
#include "qcanvasbrushprogram_p.h"
#include <QtCore/qatomic.h>
#include <QtCore/qshareddata.h>
#include <qvectornd.h>
#include <rhi/qshader.h>
#include <memory>

QT_BEGIN_NAMESPACE

// Animation clock of a custom brush. Shared by all copies of the brush, also
// over detaching, so modifying a copy of the brush does not restart the
// animation. The clock is advanced by the renderers using frame timestamps,
// at most once per timestamp, so drawing the brush several times per frame
// does not speed up the animation.
class Q_CANVASPAINTER_EXPORT QCanvasCustomBrushClock
{
public:
    // Steps larger than this are clamped, so that the animation does not
    // jump after the brush has not been rendered for a long while.
    static constexpr qint64 MaxStepNs = 5000 * 1000 * 1000LL;

    explicit QCanvasCustomBrushClock(qreal startSeconds = 0)
        : m_timeNs(qint64(startSeconds * 1e9)) {}

    static qint64 nowNs();

    // Advances the clock to frameNs and returns the position in seconds.
    qreal advance(qint64 frameNs);
    qreal position() const { return m_timeNs.loadAcquire() * 1e-9; }

private:
    QAtomicInteger<qint64> m_timeNs = 0;
    QAtomicInteger<qint64> m_lastNs = -1;
};

class QCanvasCustomBrushPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasCustomBrushPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::Custom) {}
    QCanvasCustomBrushPrivate(const QCanvasCustomBrushPrivate&) = default;
    QCanvasBrushPrivate *clone() override;
    bool equals(const QCanvasBrushPrivate &other) const noexcept override;
    QCPaint createPaint(QCanvasPainter *painter) const override;

    static QCanvasCustomBrushPrivate *get(QCanvasCustomBrush *brush)
    { return brush->d.data(); }
    static const QCanvasCustomBrushPrivate *get(const QCanvasCustomBrush *brush)
    { return brush->d.data(); }
    static QCanvasCustomBrushPrivate *get(QCanvasCustomBrush &brush)
    { return brush.d.data(); }
    static const QCanvasCustomBrushPrivate *get(const QCanvasCustomBrush &brush)
    { return brush.d.data(); }
    static QCanvasCustomBrush create(QCanvasCustomBrushPrivate *p) { return QCanvasCustomBrush(p); }
    static QCanvasCustomBrush getFromCanvasBrush(const QCanvasBrush &brush)
    {
        Q_ASSERT(brush.type() == QCanvasBrush::BrushType::Custom);
        return create(static_cast<QCanvasCustomBrushPrivate *>(QCanvasBrushPrivate::get(brush)));
    }

    struct CommonUniforms {
        // Total size 112 + 112 = 224 bytes.
        // Built-in variables size is 112 bytes
        float scissorMat[12]; // matrices are actually 3 vec4s
        float scissorExt[2];
        float scissorScale[2];
        float alphaMult;
        float strokeThr;
        float fontAlphaMin;
        float fontAlphaMax;
        float colorEffects[4];
        int texType;
        int type;
        float globalAlpha;
        int unusedInt;
        // Custom input size is 112 bytes.
        float iTime;
        int alphaIsRed;
        // Take into use when needed
        int unused1;
        float unused3;
        QVector4D data[4];
        // Take these into use when needed
        float unused2[8];
    };

    void setShaders(const QShader &fragmentShader, const QShader &vertexShader);
    // Returns the animation time in seconds for a frame rendered at frameNs.
    float currentTime(qint64 frameNs) const
    {
        if (!timeRunning || !clock)
            return time;
        return float(clock->advance(frameNs));
    }

    std::shared_ptr<const QCanvasBrushProgram> program;
    std::shared_ptr<QCanvasCustomBrushClock> clock;
    bool timeRunning = false;
    float time = 0; // The time when not running
    QVector4D data[4];
};

QT_END_NAMESPACE

#endif // QCANVASCUSTOMBRUSH_P_H
