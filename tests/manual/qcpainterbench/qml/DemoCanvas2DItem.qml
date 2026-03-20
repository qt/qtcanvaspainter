// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtCanvas2D

Item {
    id: rootItem

    property int enabledTests: 0
    property int testCount: 1
    property real animationTime: 0

    onTestCountChanged: canvas.requestPaint();
    onEnabledTestsChanged: canvas.requestPaint();
    onAnimationTimeChanged: canvas.requestPaint();

    Canvas2D {
        id: canvas
        anchors.fill: parent
        alphaBlending: true
        fillColor: "transparent"
        onPaint: {
            var ctx = canvas.getContext("2d");
            paintView(ctx);
        }
        Component.onCompleted: {
            loadImage("images/circle.png")
            canvas.requestPaint();
        }
    }

    property color m_colorWhite: Qt.rgba(1, 1, 1, 1)
    property color m_colorGray: Qt.rgba(180/255, 180/255, 180/255, 1)
    property color m_colorBlack: Qt.rgba(0, 0, 0, 1)
    property color m_color1: Qt.rgba(180/255, 190/255, 40/255, 20/255)
    property color m_color2: Qt.rgba(1, 1, 1, 150/255)
    property color m_color3: Qt.rgba(1, 1, 1, 80/255)

    function paintView(ctx) {
        const w = rootItem.width;
        const h = rootItem.height;
        if (w <= 0 || h <= 0)
            return;
        const s = Math.min(w, h);
        let t = rootItem.animationTime;
        const testCount = rootItem.testCount;
        const enabledTests = rootItem.enabledTests;

        // These painting commands are identical with other renderers
        for (let i=0; i<testCount; i++) {
            //Paint ruler
            if (enabledTests & 1) {
                drawRuler(ctx, 0, h * 0.02, w, h * 0.05, t, i);
            }
            // Paint circles
            if (enabledTests & 2) {
                const bigCircle = 50.0 + s * 0.5;
                const smallCircle = 20.0 + s * 0.2;
                drawGraphCircles(ctx, w / 2 - bigCircle / 2, h * 0.1, bigCircle, bigCircle, 8, t * 2);
                drawGraphCircles(ctx, w * 0.05, h * 0.55, smallCircle, smallCircle, 6, t * 3);
                drawGraphCircles(ctx, w - smallCircle - w * 0.05, h * 0.55, smallCircle, smallCircle, 3, t);
            }
            // Paint lines
            if (enabledTests & 4) {
                drawGraphLine(ctx, 0, h, w, -h, 4, t);
                drawGraphLine(ctx, 0, h, w, -h * 0.8, 6, t + 10);
                drawGraphLine(ctx, 0, h, w, -h * 0.6, 12, t / 2);
            }
            // Paint bars
            if (enabledTests & 8) {
                drawGraphBars(ctx, 0, h, w, -h * 0.8, 6, t * 3);
                drawGraphBars(ctx, 0, h, w, -h * 0.4, 10, t + 2);
                drawGraphBars(ctx, 0, h, w, -h * 0.3, 20, t * 2 + 2);
                drawGraphBars(ctx, 0, h, w, -h * 0.2, 40, t * 3 + 2);
            }
            // Paint icons
            if (enabledTests & 16) {
                const icons = 20;
                drawIcons(ctx, 0, h * 0.2, w, h * 0.2, icons, t, i);
            }
            // Paint flower
            if (enabledTests & 32) {
                const flowerSize = 80.0 + s * 0.6;
                drawFlower(ctx, w / 2 - flowerSize / 2, h - flowerSize, flowerSize, flowerSize, t);
            }

            // Increase animation time when testCount > 1
            t += 0.3;

        }
    }

    function drawRuler(ctx, x, y, w, h, t, index)
    {
        let posX = x + w * 0.05;
        const space = w * 0.03 + Math.sin(t) * w * 0.02;
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        const fontSize = 10.0 + w * 0.01;
        // TODO: Using custom (Roboto) font.
        const fontString = fontSize.toString() + "px sans-serif";
        ctx.font = fontString;

        ctx.strokeStyle = "#E0E0E0";
        ctx.fillStyle = "#E0E0B0";
        ctx.beginPath();
        let i = 0;
        while (posX < w) {
            ctx.moveTo(posX, y);
            let height = h * 0.2;
            let posY = y + h;
            if (i % 10 == 0) {
                height = h * 0.5;
                ctx.fillText(i.toString(), posX, posY);
            } else if (i % 5 == 0) {
                height = h * 0.3;
                if (space > w * 0.02) {
                    ctx.fillText(i.toString(), posX, posY);
                }
            }
            ctx.lineTo(posX, y + height);
            posX += space;
            i++;
        }
        ctx.lineWidth = 1.0;
        ctx.stroke();
    }

    function drawGraphCircles(ctx, x, y, w, h, items, t)
    {
        const barWidth = 0.3 * w / items;
        const lineMargin = 0.2 * barWidth;
        const showAnimationProgress = 0.1 + 0.4 * Math.sin(t * 0.8) + 0.5;
        const lineWidth = barWidth * showAnimationProgress;

        const cx = x + w / 2;
        const cy = y + h / 2;
        const radius1 = w / 2 - lineWidth;

        ctx.lineWidth = lineWidth;
        ctx.lineJoin = "round";
        ctx.lineCap = "round";

        // Draw cicle backgrounds
        let r = radius1;
        let c_background = Qt.rgba(215 / 255, 215 / 255, 215 / 255, 50 / 255);
        ctx.strokeStyle = c_background;
        for (let i = 0; i < items; i++) {
            ctx.beginPath();
            ctx.circle(cx, cy, r);
            ctx.stroke();
            r -= (lineWidth + lineMargin);
        }

        // Draw circle bars
        r = radius1;
        const a1 = -Math.PI / 2;
        for (let i = 0; i < items; i++) {
            const a0 = -Math.PI / 2 + 2 * Math.PI * ((items - i) / items) * showAnimationProgress;
            ctx.beginPath();
            ctx.arc(cx, cy, r, a0, a1, true);
            const s = i / items;
            let c = Qt.rgba((200 - 150 * s) / 255, (200 - 50 * s) / 255,
                            (100 + 50 * s) / 255, showAnimationProgress);
            ctx.strokeStyle = c;
            ctx.stroke();
            r -= (lineWidth + lineMargin);
        }
    }

    function drawGraphLine(ctx, x, y, w, h, items, t)
    {
        let samples = new Array(items);
        let sx = new Array(items);
        let sy = new Array(items);
        const dx = w / (items - 1);
        const dotSize = 4.0 + w * 0.005;
        let i = 0;

        // Generate positions
        for (i = 0; i < items; i++) {
            samples[i] = 0.5 + Math.sin((i + 1) * t * 0.2) * 0.1;
        }
        for (i = 0; i < items; i++) {
            sx[i] = x + i * dx;
            sy[i] = y + h * samples[i] * 0.8;
        }

        // Draw graph background area
        const bg = ctx.createLinearGradient(x, y, x, y + h);
        bg.addColorStop(0, m_color1);
        bg.addColorStop(1, m_color2);
        ctx.beginPath();
        ctx.moveTo(sx[0], sy[0]);
        for (i = 1; i < items; i++)
            ctx.bezierCurveTo(sx[i - 1] + dx * 0.5, sy[i - 1], sx[i] - dx * 0.5, sy[i], sx[i], sy[i]);
        ctx.lineTo(x + w, y);
        ctx.lineTo(x, y);
        ctx.fillStyle = bg;
        ctx.fill();

        // Draw graph line
        ctx.beginPath();
        ctx.moveTo(sx[0], sy[0]);
        for (i = 1; i < items; i++)
            ctx.bezierCurveTo(sx[i - 1] + dx * 0.5, sy[i - 1], sx[i] - dx * 0.5, sy[i], sx[i], sy[i]);
        ctx.strokeStyle = m_colorGray;
        ctx.lineWidth = 1.0 + dotSize * 0.2;
        ctx.stroke();

        // Draw dots
        ctx.beginPath();
        for (i = 0; i < items; i++)
            ctx.circle(sx[i], sy[i], dotSize * 0.8);
        ctx.lineWidth = dotSize * 0.2;
        ctx.strokeStyle = m_colorBlack;
        ctx.fillStyle = m_colorWhite;
        ctx.fill();
        ctx.stroke();
    }

    function drawGraphBars(ctx, x, y, w, h, items, t)
    {
        let samples = new Array(items);
        let sx = new Array(items);
        let sy = new Array(items);
        const dx = w / items;
        const barWidth = dx * 0.8;
        const margin = dx - barWidth;
        let i = 0;

        // Generate positions
        for (i = 0; i < items; i++) {
            samples[i] = 0.5 + Math.sin(i * 0.1 + t) * 0.5;
        }
        for (i = 0; i < items; i++) {
            sx[i] = x + i * dx + margin / 2;
            sy[i] = h * samples[i];
        }

        // Draw graph bars
        ctx.beginPath();
        for (i = 0; i < items; i++) {
            ctx.rect(Math.floor(sx[i]) + 0.5, Math.floor(y) + 1.5, Math.floor(barWidth), Math.floor(sy[i]));
        }
        const lineWidth = 1.0;
        ctx.lineWidth = lineWidth;
        ctx.lineJoin = "miter";
        ctx.fillStyle = m_color3;
        ctx.strokeStyle = m_colorBlack;
        ctx.fill();
        ctx.stroke();
    }

    function drawIcons(ctx, x, y, w, h, items, t)
    {
        const s = Math.min(rootItem.width, rootItem.height);
        const size = 16.0 + s * 0.05;
        const fontSize = size * 0.5;
        // TODO: Using custom (Roboto) font.
        const fontString = fontSize.toString() + "px sans-serif";
        ctx.font = fontString;
        ctx.fillStyle = "#FFFFFF";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        for (let i = 0; i < items; i++) {
            const xp = x + (w - size) / items * i;
            const yp = y + h * 0.5 + h * Math.sin((i + 1) * t * 0.1) * 0.5;
            ctx.drawImage("images/circle.png", xp, yp, size, size)
            ctx.fillText((i + 1).toString(), xp + size * 0.5, yp + size * 0.5);
        }
    }

    function _flowerPos(i)
    {
        const items = 12;
        return (2 * Math.PI) * (1 - i / items) - Math.PI / 2;
    }

    function drawFlower(ctx, x, y, w, h, t)
    {
        const cx = x + w / 2;
        const cy = y + h / 2;
        const leafSize = w / 2;

        ctx.strokeStyle = "#40000000";
        ctx.lineWidth = 4;
        const gradient1 = ctx.createRadialGradient(cx, cy, 0, cx, cy, leafSize);
        const startColor = Qt.rgba((0.5 + Math.sin(t * 2) * 0.5), 0, (0.5 + Math.sin(t + Math.PI) * 0.5), 1);
        gradient1.addColorStop(0, startColor);
        gradient1.addColorStop(1, "#ffffff");
        ctx.fillStyle = gradient1;
        ctx.translate(cx, cy);
        ctx.rotate(Math.sin(t) * 20 * (Math.PI / 180));
        ctx.translate(-cx, -cy);
        ctx.beginPath();
        ctx.moveTo(cx, cy);
        const items = 12;
        for (let i = 0; i < items; i += 2) {
            ctx.quadraticCurveTo(
                        cx + Math.cos(_flowerPos(i)) * leafSize,
                        cy + Math.sin(_flowerPos(i)) * leafSize,
                        cx + Math.cos(_flowerPos(i + 1)) * leafSize,
                        cy + Math.sin(_flowerPos(i + 1)) * leafSize);
            ctx.quadraticCurveTo(
                        cx + Math.cos(_flowerPos(i + 2)) * leafSize,
                        cy + Math.sin(_flowerPos(i + 2)) * leafSize,
                        cx,
                        cy);
        }
        ctx.fill();
        ctx.stroke();

        ctx.fillStyle = "#ffffff";
        ctx.beginPath();
        ctx.circle(cx, cy, 0.1 * w);
        ctx.fill();
        ctx.resetTransform();
    }
}
