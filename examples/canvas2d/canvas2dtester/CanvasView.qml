// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtCanvas2D

Item {
    id: rootItem
    // Currently actually shown canvas, follows the requested one
    property bool showCanvas2D: false
    // True when animation is on
    property bool animate: true

    function updateCurrentCanvas() {
        if (mainWindow.requestCanvas2D)
            canvas2D.requestPaint();
        else
            legacyCanvas.requestPaint();
    }

    function updateBothCanvas() {
        canvas2D.requestPaint();
        legacyCanvas.requestPaint();
    }

    visible: mainWindow.testCase >= 0

    FrameAnimation {
        id: fa
        running: true
        paused: !rootItem.animate
        onTriggered: {
            updateCurrentCanvas();
        }
    }

    // FPS text
    Rectangle {
        id: fpsItem
        readonly property real textSize: 24
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.top: parent.top
        anchors.topMargin: 4
        z: 10
        color: "#80000000"
        width: fpsItem.textSize * 5
        height: fpsItem.textSize * 1.5
        Text {
            anchors.centerIn: parent
            color: "#ffffff"
            font.pixelSize: fpsItem.textSize
            text: Math.round(frameAnimation.fps) + " FPS"
            FrameAnimation {
                id: frameAnimation
                property int ticks: 0
                property real frameTimes: 0
                property real fps: 0
                running: true
                onTriggered: {
                    ticks++;
                    frameTimes += frameTime;
                    let frequency = 1.0;
                    if (frameTimes > frequency) {
                        let averageFrameTime = frameTimes / ticks;
                        fps = 1.0 / averageFrameTime;
                        ticks = 0;
                        frameTimes = 0;
                    }
                }
            }
        }
    }

    function paintView(ctx, isCanvas2D) {
        // Clear the canvas, not needed with Canvas2D.
        if (!isCanvas2D)
            ctx.clearRect(0, 0, canvas2D.width, canvas2D.height);
        let complexity = complexitySlider.value;

        if (testCase == -1)
            return;

        if (testCase >= 200 && !isCanvas2D)
            return;

        // Tests
        if (testCase == 0)
            textAlignTest(ctx);
        else if (testCase == 1)
            compositeModesTest(ctx);
        else if (testCase == 2)
            gradientsTest(ctx, isCanvas2D);

        // Benchmarks
        else if (testCase == 100)
            rectanglesBenchmark(ctx, complexity)
        else if (testCase == 101)
            linesBenchmark(ctx, complexity);
        else if (testCase == 102)
            circlesBenchmark(ctx, complexity);
        else if (testCase == 103)
            clipBenchmark(ctx, complexity, isCanvas2D);
        else if (testCase == 104)
            lineStylesBenchmark(ctx, complexity);
        else if (testCase == 106)
            textsBenchmark(ctx, complexity);
        else if (testCase == 107)
            stateHandlingBenchmark(ctx, complexity);
        else if (testCase == 108)
            imagesBenchmark(ctx, complexity);
        else if (testCase == 109)
            transformationsBenchmark(ctx, complexity, isCanvas2D);

        // Features
        else if (testCase == 200)
            boxGradientTest(ctx);
        else if (testCase == 201)
            boxShadowTest(ctx);
        else if (testCase == 202)
            colorEffectsTest(ctx);
        else if (testCase == 203)
            pathHolesTest(ctx);
        else if (testCase == 204)
            antialiasTest(ctx);
        else if (testCase == 205)
            gridPatternTest(ctx);
        else if (testCase == 206)
            path2DTest(ctx, complexity);

        mainWindow.forceRepaint = false;
    }

    function rectanglesBenchmark(ctx, complexity) {
        // Draw rectangles
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rectSize = 80;
        const rectCount = 500 * complexity;
        const yMovement = (h - rectSize) / rectCount;
        ctx.lineWidth = 1;
        ctx.lineJoin = "miter";
        ctx.lineCap = "butt";
        for (let i=0; i<rectCount; i++) {
            ctx.fillStyle = Qt.rgba((i * 0.1) % 1, (i * 0.01) % 1, (i * 0.001) % 1, 0.1 + 0.9 * (i * 0.02) % 1);
            ctx.strokeStyle = ctx.fillStyle;
            if (i%2 === 0)
                ctx.fillRect(w / 2 + (w / 2) * Math.sin(0.1 * fa.elapsedTime + i * 2.0), i * yMovement, rectSize, rectSize);
            else
                ctx.strokeRect(w / 2 + (w / 2) * Math.sin(0.1 * fa.elapsedTime + i * 2.0), i * yMovement, rectSize, rectSize);
        }
    }

    function linesBenchmark(ctx, complexity) {
        // Draw lines
        const w = canvas2D.width;
        const h = canvas2D.height;
        const lines = 5 * complexity;
        const lineCount = 1000;
        const xMovement = w / lineCount;
        ctx.lineWidth = 2;
        ctx.lineJoin = "miter";
        ctx.lineCap = "butt";
        for (let j=0; j<lines; j++) {
            ctx.beginPath();
            ctx.strokeStyle = Qt.rgba((j/lines), 0.5, 1, 1);
            for (let i=0; i<lineCount; i++) {
                let posY = h * 0.5 + h * 0.4 * Math.sin(0.05 * xMovement * i + fa.elapsedTime);
                let posX = i * xMovement + j * (100 / lines);
                if (i === 0)
                    ctx.moveTo(posX, posY);
                else
                    ctx.lineTo(posX, posY);
            }
            ctx.stroke();
        }
    }

    function circlesBenchmark(ctx, complexity) {
        // Draw circles
        const m = canvas2D.width * 0.1;
        const w = canvas2D.width - 2 * m;
        const h = canvas2D.height;
        const circleCount = 200 * complexity;
        const xMovement = w / circleCount;
        ctx.lineWidth = 2;
        ctx.lineJoin = "miter";
        ctx.lineCap = "butt";
        ctx.beginPath();
        ctx.strokeStyle = "#2CDE85";
        for (let i=0; i<circleCount; i++) {
            let posY = h * 0.5 + h * 0.3 * Math.sin(0.05 * xMovement * i + fa.elapsedTime*0.2);
            let posX = m + i * xMovement;
            let circleSize = 50 + 40 * Math.sin(i * 0.05 * xMovement);
            ctx.arc(posX, posY, circleSize, 0, 2 * Math.PI);
        }
        ctx.stroke();
    }

    function clipBenchmark(ctx, complexity, isCanvas2D) {
        // Set clipping rect
        const w = canvas2D.width;
        const h = canvas2D.height;
        const m = h * 0.3 + h * 0.15 * Math.sin(fa.elapsedTime * 1.5);
        ctx.save();
        ctx.beginPath();
        ctx.strokeStyle = "#b08020";
        ctx.lineWidth = 2;
        ctx.lineJoin = "miter";
        ctx.rect(m, m, w - 2 * m, h - 2 * m);
        ctx.stroke();
        if (isCanvas2D) {
            ctx.clipRect(m, m, w - 2 * m, h - 2 * m);
        } else {
            ctx.clip();
        }
        // Normal circles benchmark
        circlesBenchmark(ctx, complexity);
        ctx.restore();
    }

    function lineStylesBenchmark(ctx, complexity) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const m = canvas2D.height * 0.1;
        const lines = 3 * complexity;
        const lineCount = 100;
        const xMovement = (w - 2 * m) / lineCount;
        const lineJoins = ["bevel", "round", "miter"];
        const lineCaps = ["butt", "round", "square"];
        ctx.lineWidth = 5 + 10 / complexity;
        ctx.miterLimit = 5;
        const setH = (h - 2 * m)  / (lines);
        let setY = m + 0.5 * setH;
        for (let j=0; j<lines; j++) {
            ctx.lineJoin = lineJoins[j%3];
            ctx.lineCap = lineCaps[j%3];
            ctx.beginPath();
            ctx.strokeStyle = Qt.rgba(1, 0.2 * (j%3), 0.6 - 0.2 * (j%3), 1);
            for (let i=0; i<lineCount; i++) {
                let posY = setY + 0.5 * setH * Math.sin(1.2 * i + fa.elapsedTime) * Math.sin(0.1 * Math.PI * i);
                let posX = m + i * xMovement;
                if (i === 0)
                    ctx.moveTo(posX, posY);
                else
                    ctx.lineTo(posX, posY);
            }
            setY += setH;
            ctx.stroke();
        }
    }

    function drawRectsWithLinearGradient(ctx) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rects = 5;
        const types = 3;
        const margin = w * 0.02;
        const sizeW = w / (rects + 1) - margin;
        const sizeH = (h - 4 * margin) / types;
        const animationSine = 0.5 + 0.5 * Math.sin(fa.elapsedTime);
        let posX = sizeW * 0.5 + margin;
        let posY = margin;

        let g1 = ctx.createLinearGradient(0, posY, 0, posY + sizeH);
        g1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
        g1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
        ctx.fillStyle = g1;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        let g2 = ctx.createLinearGradient(posX + (sizeW * 0.4 * animationSine), 0,
                                          posX + sizeW - (sizeW * 0.4 * animationSine), 0);
        g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
        g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
        ctx.fillStyle = g2;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        let g3 = ctx.createLinearGradient(posX, posY,
                                          posX + sizeW, posY + sizeH);
        g3.addColorStop(0, Qt.rgba(animationSine, 1, 1, 1));
        g3.addColorStop(1, Qt.rgba(1, animationSine, 1, 1 - animationSine));
        ctx.fillStyle = g3;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        let g4 = ctx.createLinearGradient(posX, posY,
                                          posX, posY + sizeH);
        g4.addColorStop(0.0, "black");
        g4.addColorStop(0.2, "red");
        g4.addColorStop(0.4, "lime");
        g4.addColorStop(0.6, "blue");
        g4.addColorStop(0.8, "white");
        g4.addColorStop(1.0, "transparent");
        ctx.fillStyle = g4;
        ctx.fillRect(posX, posY, sizeW, sizeH);
    }

    function drawRectsWithRadialGradient(ctx) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rects = 5;
        const types = 3;
        const margin = w * 0.02;
        const sizeW = w / (rects + 1) - margin;
        const sizeH = (h - 4 * margin) / types;
        const animationSine = 0.5 + 0.5 * Math.sin(fa.elapsedTime);
        let posX = sizeW * 0.5 + margin;
        let posY = sizeH + 2 * margin;

        let g1 = ctx.createRadialGradient(posX, posY, 0, posX, posY, sizeW);
        g1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
        g1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
        ctx.fillStyle = g1;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        let cX = posX + sizeW / 2;
        let cY = posY + sizeH / 2;
        let g2 = ctx.createRadialGradient(cX, cY, 0.4 * sizeW * animationSine, cX, cY, 0.5 * sizeW);
        g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
        g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
        ctx.fillStyle = g2;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        cX = posX + sizeW / 2;
        let g3 = ctx.createRadialGradient(cX, cY, sizeW / 4, cX, cY, sizeW / 2);
        g3.addColorStop(0, Qt.rgba(animationSine, 1, 1));
        g3.addColorStop(1, Qt.rgba(1, animationSine, 1, 1 - animationSine));
        ctx.fillStyle = g3;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        cX = posX + sizeW / 2;
        let g4 = ctx.createRadialGradient(cX - 0.2 * sizeW + 0.4 * sizeW * animationSine, cY, sizeW / 16,
                                          cX, cY, sizeW / 2);
        g4.addColorStop(0.0, Qt.rgba(animationSine, 0, 0));
        g4.addColorStop(0.1, Qt.rgba(1, 1, 0));
        g4.addColorStop(0.5, Qt.rgba(0, 1, 0));
        g4.addColorStop(1.0, Qt.rgba(0, 0, 0, 0));

        ctx.fillStyle = g4;
        ctx.fillRect(posX, posY, sizeW, sizeH);
    }

    function drawRectsWithConicalGradients(ctx, isCanvas2D) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rects = 5;
        const types = 3;
        const margin = w * 0.02;
        const sizeW = w / (rects + 1) - margin;
        const sizeH = (h - 4 * margin) / types;
        const t = fa.elapsedTime;
        const animationSine = 0.5 + 0.5 * Math.sin(t);
        let posX = sizeW * 0.5 + margin;
        let posY = 2 * sizeH + 3 * margin;

        let cX = posX + sizeW / 2;
        let cY = posY + sizeH / 2;
        let g1 = ctx.createConicalGradient(cX, cY, 0);
        if (isCanvas2D) {
            g1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
            g1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
        } else {
            // Quick canvas uses counter-clockwise angle.
            g1.addColorStop(0, Qt.rgba(0, 0, 0, 0));
            g1.addColorStop(1, Qt.rgba(1, 1, 1, 1));
        }
        ctx.fillStyle = g1;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        cX = posX + sizeW / 2;
        let angle2 = isCanvas2D ? animationSine : -animationSine;
        let g2 = ctx.createConicalGradient(cX, cY, angle2);
        if (isCanvas2D) {
            g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
        } else {
            // Quick canvas uses counter-clockwise angle.
            g2.addColorStop(0, Qt.rgba(1, 0, 0, 1));
            g2.addColorStop(1, Qt.rgba(0, 1, 0, 1));
        }
        ctx.fillStyle = g2;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        cX = posX + sizeW / 2;
        let angle3 = isCanvas2D ? -0.5 * Math.PI : 0.5 * Math.PI;
        let g3 = ctx.createConicalGradient(cX, cY, angle3);
        if (isCanvas2D) {
            g3.addColorStop(0, Qt.rgba(animationSine, 1, 1));
            g3.addColorStop(1, Qt.rgba(1, animationSine, 1, 1 - animationSine));
        } else {
            // Quick canvas uses counter-clockwise angle.
            g3.addColorStop(0, Qt.rgba(1, animationSine, 1, 1 - animationSine));
            g3.addColorStop(1, Qt.rgba(animationSine, 1, 1));
        }
        ctx.fillStyle = g3;
        ctx.fillRect(posX, posY, sizeW, sizeH);
        posX += sizeW + margin;

        let angle4 = isCanvas2D ? t : -t;
        let g4 = ctx.createConicalGradient(posX + animationSine * sizeW, cY, angle4);
        if (isCanvas2D) {
            g4.addColorStop(0.00, Qt.rgba(1, 1, 0));
            g4.addColorStop(0.25, Qt.rgba(0, 1, 0));
            g4.addColorStop(0.50, Qt.rgba(0, 0, 1));
            g4.addColorStop(0.75, Qt.rgba(1, 0, 1));
            g4.addColorStop(1.00, Qt.rgba(1, 1, 0));
        } else {
            // Quick canvas uses counter-clockwise angle.
            g4.addColorStop(1.00, Qt.rgba(1, 1, 0));
            g4.addColorStop(0.75, Qt.rgba(0, 1, 0));
            g4.addColorStop(0.50, Qt.rgba(0, 0, 1));
            g4.addColorStop(0.25, Qt.rgba(1, 0, 1));
            g4.addColorStop(0.00, Qt.rgba(1, 1, 0));
        }
        ctx.fillStyle = g4;
        ctx.fillRect(posX, posY, sizeW, sizeH);
    }

    function gradientsTest(ctx, isCanvas2D) {
        drawRectsWithLinearGradient(ctx);
        drawRectsWithRadialGradient(ctx);
        drawRectsWithConicalGradients(ctx, isCanvas2D);
    }

    function textsBenchmark(ctx, complexity) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const m = h * 0.1;
        ctx.font = "16px sans-serif";
        ctx.fillStyle = "#e0e0e0";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        const texts = 100 * complexity;//200;
        const yMovement = (h - m*2) / texts;
        for (let i=0; i<texts; i++) {
            const xPos = w * 0.5 + w * 0.4 * Math.sin(0.1 * fa.elapsedTime + i*0.5);
            const yPos = m + i * yMovement;
            const s = "pos:(" + xPos.toFixed(0) + ", " + yPos.toFixed(0) + ")"
            ctx.fillText(s, xPos, yPos);
        }
    }

    function textAlignTest(ctx, complexity) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const m = w * 0.1;
        const cX = w * 0.7;

        ctx.beginPath();
        ctx.lineWidth = 2;
        ctx.strokeStyle = "red";
        ctx.moveTo(cX, h * 0.1);
        ctx.lineTo(cX, h * 0.9);
        ctx.stroke();

        ctx.lineWidth = 2;
        ctx.strokeStyle = "#d08040";
        ctx.font = "30px sans-serif";
        ctx.fillStyle = "#e0e0e0";
        ctx.textBaseline = "top";
        const alignments = [
          "start",
          "end",
          "left",
          "center",
          "right",
        ];
        alignments.forEach((alignment, index) => {
            const y = h * 0.1 + index * h * 0.15;
            ctx.textAlign = alignment;
            let s = `Align (${alignment})`;
            ctx.strokeText(s, cX, y);
            ctx.fillText(s, cX, y);
        });

        ctx.textAlign = "left";
        const baselines = [
          "top",
          "hanging",
          "middle",
          "alphabetic",
          "ideographic",
          "bottom",
        ];

        baselines.forEach((baseline, index) => {
            ctx.textBaseline = baseline;
            const y = h*0.1 + index * h*0.15;
            ctx.beginPath();
            ctx.moveTo(m, y + 0.5);
            ctx.lineTo(m + w*0.4, y + 0.5);
            ctx.strokeStyle = "red";
            ctx.stroke();
            ctx.lineWidth = 2;
            ctx.strokeStyle = "#d08040";
            const s = `Baseline (${baseline})`;
            ctx.strokeText(s, m, y);
            ctx.fillText(s, m, y);
        });
    }

    function imagesBenchmark(ctx, complexity) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const m = canvas2D.height * 0.1;
        ctx.fillStyle = "#000000";
        ctx.fillRect(0, m, w, h-m);
        // TODO: This doesn't work, so pattern image size can't be adjusted after load?
        // Also starting pos for pattern can't be changed?
        //let img = ctx.createImageData("qtlogo.png")
        //img.width = 32;
        //img.height = 23;
        //var pattern = ctx.createPattern(img, 'repeat');
        var pattern = ctx.createPattern("qtlogo.png", 'repeat');
        ctx.fillStyle = pattern;
        ctx.fillRect(0, m, w, h-m);
        const images = 1000 * complexity;//200;
        const imageSize = w * 0.05;
        const yMovement = (h - m*2) / images;
        for (let i=0; i<images; i++) {
            const xPos = -0.5*imageSize + w * 0.5 + w * 0.4 * Math.sin(0.1 * fa.elapsedTime + i);
            const yPos = m + i * yMovement;
            ctx.drawImage("face-smile.png", xPos, yPos, imageSize, imageSize)
        }
    }

    function drawFlower(ctx, isCanvas2D, cX, cY, leafs) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rotateAngle = (2 * Math.PI) / leafs;
        const leafHeight = h/2;
        const leafWidth = h/leafs * 2;
        const cRadius = h/20;
        ctx.save();
        ctx.translate(cX, cY);
        const flowerScale = 0.4 + 0.1 * Math.sin(fa.elapsedTime);
        ctx.rotate(fa.elapsedTime);
        ctx.scale(flowerScale, flowerScale);
        ctx.translate(-cX, -cY);

        // Leafs
        ctx.beginPath();
        for (let i = 0; i < leafs; i++) {
            ctx.translate(cX, cY);
            ctx.rotate(rotateAngle);
            if (isCanvas2D) {
                ctx.moveTo(0.5 * leafWidth, 0.5 * leafHeight);
                ctx.ellipse(0, 0.5 * leafHeight, 0.5 * leafWidth, 0.5 * leafHeight, 0, 0, 2 * Math.PI);
                // Alternative for Qt Quick Canvas compatibility:
                //ctx.ellipseRect(-0.5 * leafWidth, 0, leafWidth, leafHeight, 0);
            } else {
                // Note: Quick Canvas ellipse() doesn't follow 2d context standard.
                // It takes ellipse rectangle area instead of center point & radius.
                // https://doc.qt.io/qt-6/qml-qtquick-context2d.html#ellipse-method
                ctx.ellipse(-0.5 * leafWidth, 0, leafWidth, leafHeight, 0);
            }
            ctx.translate(-cX, -cY);
        }
        ctx.lineWidth = 8;
        ctx.fill();
        ctx.stroke();

        // Center circle
        ctx.beginPath();
        ctx.arc(cX, cY, cRadius, 0, 2 * Math.PI);
        ctx.fillStyle = "#f0c060";
        ctx.fill();

        ctx.restore();
    }

    function transformationsBenchmark(ctx, complexity, isCanvas2D) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const cX = w * 0.5;
        const cY = h * 0.5;
        const s = Math.min(w, h);
        ctx.strokeStyle = "#000000";
        ctx.fillStyle = "#e0e0e0";
        drawFlower(ctx, isCanvas2D, w*0.5, h*0.5, 26);
        const flowers = complexity;//5*complexity;
        for (let i = 0; i < flowers; i++) {
            ctx.resetTransform();
            ctx.translate(cX, cY);
            const rotateAngle = (i / flowers) * Math.PI*0.5;
            ctx.rotate(rotateAngle);
            ctx.translate(-cX, -cY);
            let c1 = Qt.rgba(1 - (i / flowers), (i / flowers), 1, 1);
            ctx.fillStyle = c1;
            const sr = s * 0.25;
            drawFlower(ctx, isCanvas2D, cX + sr, cY - sr, 6);
            drawFlower(ctx, isCanvas2D, cX + sr, cY + sr, 10);
            drawFlower(ctx, isCanvas2D, cX - sr, cY + sr, 14);
            drawFlower(ctx, isCanvas2D, cX - sr, cY - sr, 18);
        }
        ctx.resetTransform();
    }

    function stateHandlingBenchmark(ctx, complexity) {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rects = 3;
        const margin = w * 0.2;
        const marginY = h * 0.05;
        const size = w / rects - margin;
        const lineWidth = 1 + w * 0.004;
        const animationSine = 0.5 + 0.5 * Math.sin(fa.elapsedTime);
        const renders = complexity * 20;
        for (let i = 0; i < renders; i++) {
            let posX = margin/2 + (i / renders) * margin;
            let posY = marginY + (i / (renders - 1)) * (h - size - 2 * marginY);

            ctx.fillStyle = "#D0D0D0";
            ctx.strokeStyle = "#f0f0f0";
            ctx.lineWidth = lineWidth;
            ctx.beginPath();
            ctx.rect(posX, posY, size, size);
            ctx.fill();
            ctx.stroke();
            // Save the painter state
            ctx.save();

            // Modify painter state
            posX += size + margin * 0.5;
            ctx.fillStyle = "#900000";
            ctx.strokeStyle = "#E00000";
            ctx.lineWidth = lineWidth * 0.5;
            ctx.lineCap = "round";
            ctx.lineJoin = "round";
            ctx.translate(posX+size/2, posY+size/2);
            ctx.rotate(1.0 + fa.elapsedTime);
            const scale = 0.75 + 0.25 * Math.sin(10 * (i / renders) + fa.elapsedTime);
            ctx.scale(scale, scale);
            ctx.translate(-(posX+size/2), -(posY+size/2));
            ctx.beginPath();
            ctx.rect(posX, posY, size, size);
            ctx.fill();
            ctx.stroke();

            posX += size + margin * 0.5;
            // And restore painter state back
            ctx.restore();
            ctx.beginPath();
            ctx.rect(posX, posY, size, size);
            ctx.fill();
            ctx.stroke();
        }
    }

    function drawCompositeItem1(ctx, x, y, w, h, mode)
    {
        ctx.globalCompositeOperation = "source-over";
        // First item
        ctx.beginPath();
        ctx.rect(x, y, w, h * 0.7, w * 0.2);
        ctx.fillStyle = "#DFD0B8";
        ctx.fill();
        ctx.strokeStyle = "#ffffff";
        ctx.lineWidth = 4;
        ctx.stroke();

        ctx.globalCompositeOperation = mode;
        // Second item
        ctx.beginPath();
        const animationSine = 0.5 + 0.5 * Math.sin(fa.elapsedTime);
        const cx = x + w * 0.5;
        const cy = y + h * (0.6 + 0.4 * animationSine);
        const cr = h * 0.4;
        ctx.arc(cx, cy, cr, 0, 2 * Math.PI);
        let rg1 = ctx.createRadialGradient(cx, cy, 0, cx, cy, cr);
        rg1.addColorStop(0, "#ff8080");
        rg1.addColorStop(1, "#401010");
        ctx.fillStyle = rg1;
        ctx.fill();
    }

    function compositeModesTest(ctx) {
        const w = canvas2D.width;
        const rects = 3;
        const margin = w * 0.1;
        const sizeW = w / rects - margin;
        const sizeH = sizeW * 0.6;
        let posY = margin / 2;
        let posX = margin / 2;
        drawCompositeItem1(ctx, posX, posY, sizeW, sizeH, "source-over");
        posX += sizeW + margin;
        drawCompositeItem1(ctx, posX, posY, sizeW, sizeH, "source-atop");
        posX += sizeW + margin;
        drawCompositeItem1(ctx, posX, posY, sizeW, sizeH, "destination-out");

        // Repeat, but with reduced alpha
        ctx.globalAlpha = 0.5;
        posY += sizeH * 1.5;
        posX = margin / 2;
        drawCompositeItem1(ctx, posX, posY, sizeW, sizeH, "source-over");
        posX += sizeW + margin;
        drawCompositeItem1(ctx, posX, posY, sizeW, sizeH, "source-atop");
        posX += sizeW + margin;
        drawCompositeItem1(ctx, posX, posY, sizeW, sizeH, "destination-out");
        ctx.globalAlpha = 1.0;

        // Reset the default composition mode
        ctx.globalCompositeOperation = "source-over";
    }

    function boxGradientTest(ctx)
    {
        const w = canvas2D.width;
        const rects = 4;
        const margin = w * 0.02;
        const size = w / (rects+1) - margin;
        const animationSine = 0.5 + 0.5 * Math.sin(fa.elapsedTime);
        let posX = size * 0.5 + margin;
        let posY = size * 0.5;

        const g1 = ctx.createBoxGradient(posX, posY, size, size, size/2);
        ctx.fillStyle = g1;
        ctx.fillRect(posX, posY, size, size);
        posX += size + margin;

        const r2 = size/4;
        const f2 = size/2 - animationSine*size/3
        const g2 = ctx.createBoxGradient(posX, posY, size, size, f2, r2);
        g2.addColorStop(0.0, Qt.rgba(0, 1, 0, 1));
        g2.addColorStop(1.0, Qt.rgba(1, 0, 0, 1));
        ctx.fillStyle = g2;
        ctx.fillRect(posX, posY, size, size);
        posX += size + margin;

        const r3 = size/4;
        const f3 = size/4;
        const g3 = ctx.createBoxGradient(posX, posY, size, size, f3, r3);
        g3.addColorStop(0.0, Qt.rgba(animationSine, 1, 1, 1));
        g3.addColorStop(1.0, Qt.rgba(1, animationSine, 1, 1 - animationSine));
        ctx.fillStyle = g3;
        ctx.fillRect(posX, posY, size, size);
        posX += size + margin;

        const r4 = size/3;
        const f4 = size/4;
        const g4 = ctx.createBoxGradient(posX, posY, size, size, f4, r4);
        g4.addColorStop(0.0, Qt.rgba(1, 1, 1, 1));
        g4.addColorStop(0.2, Qt.rgba(0, 0, 0, 1));
        g4.addColorStop(0.4, Qt.rgba(1, animationSine, 0, 1));
        g4.addColorStop(0.6, Qt.rgba(0, 0, 0, 1));
        g4.addColorStop(0.8, Qt.rgba(1, 1, 1, 1));
        g4.addColorStop(1.0, Qt.rgba(0, 0, 0, 0));
        ctx.fillStyle = g4;
        ctx.fillRect(posX, posY, size, size);
        posX += size + margin;
    }

    function boxShadowTest(ctx)
    {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const rects = 3;
        const margin = w * 0.15;
        const sizeW = w / rects - margin;
        const sizeH = h - 2 * margin;
        const t = fa.elapsedTime * 2;
        const animationSine = 0.5 + 0.5 * Math.sin(t);
        let posX = margin * 0.5;
        let posY = margin;

        let bs1 = ctx.createBoxShadow(posX, posY, sizeW, sizeH, animationSine * 0.3 * sizeW, "black", 0.1 * sizeW);
        ctx.drawBoxShadow(bs1);

        ctx.translate(sizeW + margin, 0);
        bs1 = ctx.createBoxShadow(posX, posY, sizeW, sizeH, 0.1 * sizeW, "#2CDE85",
                                  (0.5 + 0.5 * Math.sin(t)) * 0.4 * sizeW,
                                  (0.5 + 0.5 * Math.sin(t + 0.5 * Math.PI)) * 0.4 * sizeW,
                                  (0.5 + 0.5 * Math.sin(t + 1.0 * Math.PI)) * 0.4 * sizeW,
                                  (0.5 + 0.5 * Math.sin(t + 1.5 * Math.PI)) * 0.4 * sizeW);
        ctx.drawBoxShadow(bs1);
        ctx.translate(sizeW + margin, 0);

        // Double shadows
        const blur = sizeW * 0.2;
        const radius = sizeW * 0.05;
        let shadowOffsetX = blur * 0.3;
        let shadowOffsetY = blur * 0.3;
        let c = "#202020";
        let cl = Qt.lighter(c, 200);
        let cd = Qt.darker(c, 250);
        ctx.globalAlpha = 0.1 * animationSine;
        let shadow1 = ctx.createBoxShadow(posX + shadowOffsetX, posY - shadowOffsetY, sizeW, sizeH, blur, cl, radius);
        ctx.drawBoxShadow(shadow1);
        ctx.globalAlpha = 0.6 * animationSine;
        let shadow2 = ctx.createBoxShadow(posX - shadowOffsetX, posY + shadowOffsetY, sizeW, sizeH, blur, cd, radius);
        ctx.drawBoxShadow(shadow2);
        ctx.globalAlpha = 1.0;

        // Button on top of shadows
        ctx.beginPath();
        ctx.roundRect(posX, posY, sizeW, sizeH, radius);
        ctx.fillStyle = c;
        ctx.fill();
    }

    function drawButton(ctx, x, y, w, h, label)
    {
        const border = h * 0.1;
        ctx.lineWidth = border;
        const cx = x + 0.5 * w;
        const cy = y + 0.5 * h;
        const g1 = ctx.createLinearGradient(cx - 0.25 * w,
                                            cy - 0.25 * w,
                                            cx + 0.25 * h,
                                            cy + 0.25 * w);
        g1.addColorStop(0.0, "#FF0000");
        g1.addColorStop(0.5, "#D0D000");
        g1.addColorStop(1.0, "#000000");
        const g2 = ctx.createLinearGradient(0,
                                            y,
                                            0,
                                            y + h);
        g2.addColorStop(0.0, "#00414A");
        g2.addColorStop(0.4, "#2CDE85");
        g2.addColorStop(1.0, "#00414A");
        ctx.beginPath();
        ctx.roundRect(x, y, w, h, border * 2);
        ctx.strokeStyle = g1;
        ctx.fillStyle = g2;
        ctx.fill();
        ctx.stroke();

        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        const fontSize = h * 0.4
        ctx.font = fontSize + "px sans-serif";
        ctx.fillStyle = "#FFFFFF";
        ctx.fillText(label, cx, cy);
    }

    function colorEffectsTest(ctx)
    {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const sizeW = w * 0.8;
        const sizeH = h * 0.15;
        const margin = h * 0.04;
        let posX = w * 0.5 - sizeW * 0.5;
        let posY = margin;
        const t = fa.elapsedTime * 2;
        const animationSine = 0.5 + 0.5 * Math.sin(t);
        ctx.globalAlpha = animationSine;
        drawButton(ctx, posX, posY, sizeW, sizeH, "OPACITY");
        ctx.globalAlpha = 1.0;
        posY += sizeH + margin;
        ctx.globalSaturate = 3 * animationSine;
        drawButton(ctx, posX, posY, sizeW, sizeH, "SATURATE");
        ctx.globalSaturate = 1.0;
        posY += sizeH + margin;
        ctx.globalBrightness = 2 * animationSine;
        drawButton(ctx, posX, posY, sizeW, sizeH, "BRIGHTNESS");
        ctx.globalBrightness = 1.0;
        posY += sizeH + margin;
        ctx.globalContrast = 3 * animationSine;
        drawButton(ctx, posX, posY, sizeW, sizeH, "CONTRAST");
        ctx.globalContrast = 1.0;
        posY += sizeH + margin;
        ctx.globalAlpha = animationSine;
        ctx.globalSaturate = 8 - 8 * animationSine;
        ctx.globalContrast = 2 * animationSine;
        drawButton(ctx, posX, posY, sizeW, sizeH, "MULTIPLE");
    }

    function pathHolesTest(ctx)
    {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const size = h * 0.3;
        const cx = w * 0.5;
        const cy = h * 0.5;
        ctx.beginPath();
        ctx.circle(cx, cy, size);
        ctx.beginHoleSubPath();
        ctx.rect(cx - size * 0.6, cy - size * 0.6, size * 1.2, size * 1.2);
        ctx.beginSolidSubPath();
        ctx.circle(cx, cy, size * 0.2);
        ctx.fillStyle = "#ff0000";
        ctx.strokeStyle = "#ffffff";
        ctx.lineWidth = 4;
        ctx.fill();
        ctx.stroke();
    }

    function antialiasTest(ctx)
    {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const m = w * 0.1;
        const my = h * 0.2;
        const w2 = w * 0.5;
        const t = fa.elapsedTime * 2;
        const animationSine = 0.5 + 0.5 * Math.sin(t);
        const lines = 9;
        ctx.lineWidth = h * 0.02;
        ctx.strokeStyle = "#2CDE85";
        ctx.fillStyle = "#00414A";

        for (let i = 0; i < lines; i++) {
            let y = my + i / (lines - 1) * (h - 2 * m);
            ctx.antialias = 1 + i;
            ctx.beginPath();
            ctx.moveTo(m, y);
            ctx.bezierCurveTo(m + w2 * 0.2, y + 0.1 * h * animationSine,
                              m + w2 * 0.6, y - 0.1 * h * animationSine,
                              m + w2 - m, y);
            ctx.stroke();
        }

        const aa = 9 * animationSine;
        ctx.antialias = 1 + aa;
        ctx.beginPath();
        ctx.roundRect(w2 + m, my, w2 - 2 * m, h - 2 * m,
                     w2 * 0.1, w2 * 0.2, w2 * 0.1, w2 * 0.3);
        ctx.lineWidth = h * 0.04;
        ctx.fill();
        ctx.stroke();
    }

    function gridPatternTest(ctx)
    {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const t = fa.elapsedTime * 2;
        const animationSine = 0.5 + 0.5 * Math.sin(t);
        const rects = 3;
        const margin = w * 0.04;
        const my = h * 0.2;
        const size = (w - 4 * margin) / rects;
        let posX = margin;
        let posY = my;

        const cellZoom = 0.2 + 1.0 * animationSine;
        const cp1x = posX + size * 0.5;
        const cp1y = posY + size * 0.5;
        // Minor grid
        const gp1 = ctx.createGridPattern(cp1x, cp1y, 0.1 * size * cellZoom, 0.1 * size * cellZoom,
                                          "#404040", "#202020");
        ctx.fillStyle = gp1;
        ctx.fillRect(posX, posY, size, size);
        // Major grid
        const gp2 = ctx.createGridPattern(cp1x, cp1y, size * cellZoom, size * cellZoom,
                                          "#d0d0d0", "transparent");
        ctx.fillStyle = gp2;
        ctx.fillRect(posX, posY, size, size);

        posX += size + margin;
        const cp2x = posX + size * 0.5;
        const cp2y = posY + size * 0.5;
        // Bars fill
        // Speed, taking 45-degrees rotation into account.
        const animSpeed = 0.5 * Math.sqrt(2) / Math.PI * t;
        const bar = size * 0.5;
        const gp3 = ctx.createGridPattern(animSpeed * bar, 0, bar, 0,
                                          "#2CDE85", "00414A",
                                          bar * 0.5, bar * 0.5, Math.PI / 4);
        ctx.fillStyle = gp3;
        ctx.lineWidth = 0.5 * bar;

        // Dashed stroke
        const strokeW = (size / 5) * animationSine;
        const gp4 = ctx.createGridPattern(cp2x, cp2y, size / 5, size / 5,
                                          "black", "white",
                                          strokeW);
        ctx.strokeStyle = gp4;
        ctx.lineWidth = 4;
        ctx.beginPath();
        ctx.roundRect(posX, posY, size, size, 10);
        ctx.fill();
        ctx.stroke();

        posX += size + margin;

        const bar2 = size * 0.2;
        const gp5 = ctx.createGridPattern(15 * t, 0, bar2, 0,
                                          "#D0D040", "black",
                                          bar2 * 0.5, 1, Math.PI / 4);
        const gp6 = ctx.createGridPattern(0, 0, 0, bar2 * 0.25,
                                          "#404010", "#505010",
                                          bar2 * 0.1, 1, Math.PI / 4);
        ctx.strokeStyle = gp5;
        ctx.fillStyle = gp6;
        ctx.lineWidth = 0.05 * size;
        ctx.beginPath();
        ctx.roundRect(posX, posY, size, size, bar2);
        ctx.fill();
        ctx.stroke();
    }

    property var heartPath: null
    property var personPath: null

    function path2DTest(ctx, complexity)
    {
        const w = canvas2D.width;
        const h = canvas2D.height;
        const t = fa.elapsedTime * 2;
        // Use -1 to not cache the paths.
        const personPathGroup = 1;
        const heartPathGroup = 2;

        if (!personPath || personPath.isEmpty()) {
            const path = "M152,84a36,36,0,1,0-36-36A36.04061,36.04061,0,0,0,152,84Zm0-48Z" +
                       "m64,112a12.00028,12.00028,0,0,1-12,12c-37.20215,0-55.50781-19.66406" +
                       "-70.21729-35.46484-.45459-.48828-.89794-.96192-1.34667-1.44239l-8.02393," +
                       "18.45484,34.5625,24.68774A11.999,11.999,0,0,1,164,176v56a12,12,0,0,1-24," +
                       "0V182.17578l-25.37207-18.12353L83.00488,236.78516a12.00021,12.00021,0,0," +
                       "1-22.00976-9.57032l37.55322-86.37255.02881-.06592,13.74463-31.61377c-" +
                       "8.09571-.96167-18.24219,2.0072-30.35889,8.94482a159.5463,159.5463,0,0,0-" +
                       "29.47754,22.37793,12.0001,12.0001,0,0,1-16.9707-16.9707A183.31075," +
                       "183.31075,0,0,1,70.03711,97.28027c36.06689-20.65234,58.03027-11.69433," +
                       "70.104-.54589,3.93116,3.63085,7.63037,7.60449,11.208,11.44726C164.667," +
                       "122.4873,177.24561,136,204,136A12.00028,12.00028,0,0,1,216,148Z";
            personPath = ctx.createPath2D(path);
        }
        var d = Math.min(w, h) * 0.1;
        ctx.lineWidth = d * 0.1;
        ctx.fillStyle = "#FFFFFF";

        for (let j = 0; j < complexity; j++) {
            let cx = 120;
            let cy = 120;

            let px = w * 0.2 - cx;
            let py = h * 0.5 - cy;
            let m = ctx.createTransform2D();
            m.translate(cx + px, cy + py);
            m.rotateRadians(Math.sin(j * 0.2 + t) * 0.5);
            m.scale(1.0 + 0.6 * Math.sin((j + 4 * t) * 0.1));
            m.translate(-cx, -cy);
            ctx.setTransform(m);

            ctx.strokeStyle = Qt.rgba(0, 1 - (j + 1) / complexity, 0);
            ctx.fill(personPath, personPathGroup);
            ctx.stroke(personPath, personPathGroup);
        }

        // Hearts

        ctx.fillStyle = "#E02020";
        ctx.strokeStyle = "#000000";

        // If canvas size has changed, path needs to be recreated
        if (heartPath && mainWindow.forceRepaint)
            heartPath.clear();

        if (!heartPath || heartPath.isEmpty()) {
            console.debug("Recreating the path");
            heartPath = ctx.createPath2D();
            heartPath.moveTo(0, d / 4);
            heartPath.quadraticCurveTo(0, 0, d / 4, 0);
            heartPath.quadraticCurveTo(d / 2, 0, d / 2, d / 4);
            heartPath.quadraticCurveTo(d / 2, 0, d * 3/4, 0);
            heartPath.quadraticCurveTo(d, 0, d, d / 4);
            heartPath.quadraticCurveTo(d, d / 2, d * 3/4, d * 3/4);
            heartPath.lineTo(d / 2, d);
            heartPath.lineTo(d / 4, d * 3/4);
            heartPath.quadraticCurveTo(0, d / 2, 0, d / 4);
        }
        let items = 20 * complexity;
        for (let i = 0; i < items; i++) {
            let posX = 0.4 * w + 0.5 * (w - d) * (i / items);
            let posY = (h - d) * 0.5 + (h - d) * 0.3 * Math.sin(i / items * 10 + t);
            ctx.resetTransform();
            ctx.translate(posX, posY);
            ctx.stroke(heartPath, heartPathGroup);
            ctx.fill(heartPath, heartPathGroup);
        }
    }

    Canvas2D {
        id: canvas2D
        anchors.fill: parent
        visible: rootItem.showCanvas2D
        alphaBlending: true
        fillColor: "transparent"
        onPaint: {
            var ctx = canvas2D.getContext("2d");
            paintView(ctx, true);
        }
        onPainted: {
            rootItem.showCanvas2D = mainWindow.requestCanvas2D;
        }
        Component.onCompleted: {
            loadImage("qtlogo.png", Qt.size(32, 23))
            loadImage("face-smile.png")
            canvas2D.requestPaint();
        }
    }
    Canvas {
        id: legacyCanvas
        anchors.fill: parent
        visible: !rootItem.showCanvas2D
        onPaint: {
            var ctx = legacyCanvas.getContext("2d");
            paintView(ctx, false);
        }
        onPainted: {
            rootItem.showCanvas2D = mainWindow.requestCanvas2D;
        }
        Component.onCompleted: {
            loadImage("qtlogo.png", Qt.size(32, 23))
            loadImage("face-smile.png")
            legacyCanvas.requestPaint();
        }
    }
    Text {
        anchors.centerIn: legacyCanvas
        color: "#ffffff"
        font.pixelSize: 20
        visible: !rootItem.showCanvas2D && testCase >= 200
        text: qsTr("The features in this view are only available on Canvas2D.")
    }
}
