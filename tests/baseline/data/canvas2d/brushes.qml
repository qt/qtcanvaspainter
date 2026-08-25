import QtQuick
import QtCanvas2D

Rectangle {
    id: rootItem
    height: 400
    width: 400
    color: "black"

    Canvas2D {
        id: canvas

        property lineargradient2d lg1
        property radialgradient2d rg1
        property conicalgradient2d cg1
        property boxgradient2d bg1
        property gridpattern2d gp1
        property boxshadow2d bs1

        anchors.fill: parent
        alphaBlending: true
        fillColor: "black"

        Component.onCompleted: {
            loadImage("../shared/qtlogo.png", Qt.size(32, 23));
            canvas.requestPaint();
        }
        onImageLoaded: canvas.requestPaint()

        onPaint: {
            var ctx = canvas.getContext("2d");
            drawLinearGradientRects(ctx);
            drawRadialGradientRects(ctx);
            drawConicalGradientRects(ctx);
            drawBoxGradientRects(ctx);
            drawGridPatternRects(ctx);
            drawBoxShadowRects(ctx);
            drawImagePatternRects(ctx);
        }

        function drawLinearGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 2;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = margin;

            // property
            lg1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
            lg1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
            lg1.setStartPosition(0, posY + 0.3 * sizeH);
            lg1.setEndPosition(0, posY + 0.7 * sizeH);
            ctx.fillStyle = lg1;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            let g2 = ctx.createLinearGradient(posX + (sizeW * 0.4 * animationSine), 0,
                                              posX + sizeW - (sizeW * 0.4 * animationSine), 0);
            g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
            // Switch start & end
            let prevStart = g2.startPosition();
            g2.setStartPosition(g2.endPosition());
            g2.setEndPosition(prevStart);
            ctx.fillStyle = g2;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }

        function drawRadialGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 3;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = sizeH + 2 * margin;

            // property
            rg1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
            rg1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
            rg1.setOuterRadius(sizeW);
            rg1.setCenterPosition(posX + 0.5 * sizeW, posY + 0.5 * sizeW);
            ctx.fillStyle = rg1;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            let cX = posX + sizeW / 2;
            let cY = posY + sizeH / 2;
            let g2 = ctx.createRadialGradient(cX, cY, 0.4 * sizeW * animationSine, cX, cY, 0.5 * sizeW);
            g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
            g2.setInnerCenterPosition(cX, cY + 0.1 * sizeW);
            g2.setOuterCenterPosition(cX, cY - 0.1 * sizeW);

            ctx.fillStyle = g2;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            cX = posX + sizeW / 2;
            let g3 = ctx.createRadialGradient(cX, cY, sizeW / 4, cX, cY, sizeW / 2);
            g3.addColorStop(0, Qt.rgba(animationSine, 1, 1));
            g3.addColorStop(1, Qt.rgba(1, animationSine, 1, 1 - animationSine));
            g3.setInnerRadius(sizeW * 0.3);
            g3.setOuterRadius(sizeW * 0.4);
            ctx.fillStyle = g3;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }

        function drawConicalGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 2;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = 2 * sizeH + 3 * margin;
            let cX = posX + sizeW / 2;
            let cY = posY + sizeH / 2;

            // property
            cg1.setCenterPosition(cX, cY);
            cg1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
            cg1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
            cg1.setStartAngle(Math.PI);
            ctx.fillStyle = cg1;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            cX = posX + sizeW / 2;
            let g2 = ctx.createConicalGradient(cX, cY, animationSine);
            g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
            let oldPos = g2.centerPosition();
            g2.setCenterPosition(oldPos.x + sizeW * 0.25, oldPos.y + sizeH * 0.25);
            ctx.fillStyle = g2;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }

        function drawBoxGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 2;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = 3 * sizeH + 4 * margin;

            // property
            bg1.setRect(posX, posY, sizeW, sizeH);
            bg1.setFeather(sizeH / 3);
            bg1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
            bg1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
            ctx.fillStyle = bg1;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            const r2 = sizeW / 4;
            const f2 = sizeW / 2 - animationSine * sizeW / 3;
            const g2 = ctx.createBoxGradient(posX, posY, sizeW, sizeH, f2, r2);
            g2.addColorStop(0.0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1.0, Qt.rgba(1, 0, 0, 1));
            g2.setFeather(sizeW * 0.05);
            g2.setRadius(sizeW * 0.15);
            ctx.fillStyle = g2;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }

        function drawGridPatternRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 2;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = 4 * sizeH + 5 * margin;
            const cp1x = posX + sizeW * 0.5;
            const cp1y = posY + sizeH * 0.5;

            // property
            gp1.setStartPosition(cp1x, cp1y);
            gp1.setCellSize(0.1 * sizeW, 0.1 * sizeH);
            gp1.setLineColor("#404040");
            gp1.setBackgroundColor("#202020");
            ctx.fillStyle = gp1;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            const gp2 = ctx.createGridPattern(cp1x, cp1y, sizeW, sizeH,
                                              "red", "green");
            gp2.setLineColor("#d0d0d0");
            gp2.setBackgroundColor("transparent");
            ctx.fillStyle = gp2;
            ctx.fillRect(posX, posY, sizeW, sizeH);

            posX += sizeW + margin;
            const cp2x = posX + sizeW * 0.5;
            const cp2y = posY + sizeH * 0.5;
            const bar = sizeW * 0.2;
            const gp3 = ctx.createGridPattern(0, 0, bar, 0,
                                              "#2CDE85", "00414A",
                                              0, 0, 0);
            gp3.setLineWidth(bar * 0.4);
            gp3.setFeather(bar * 0.2);
            gp3.setRotation(-Math.PI / 4);
            ctx.fillStyle = gp3;
            const strokeW = (sizeW / 5) * 0.2;
            const gp4 = ctx.createGridPattern(cp2x, cp2y, sizeW / 5, sizeH / 5);
            gp4.setLineColor("red");
            gp4.setBackgroundColor("white");
            gp4.setLineWidth(strokeW);
            ctx.strokeStyle = gp4;
            ctx.lineWidth = 4;
            ctx.beginPath();
            ctx.roundRect(posX, posY, sizeW, sizeH, 10);
            ctx.fill();
            ctx.stroke();
        }

        function drawBoxShadowRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 2;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = 5 * sizeH + 6 * margin;

            // property
            bs1.setRect(posX, posY, sizeW, sizeH);
            bs1.setRadius(0.05 * sizeW);
            bs1.setBlur(0.1 * sizeW);
            bs1.setColor("white");
            ctx.drawBoxShadow(bs1);
            posX += sizeW + margin;

            let bs2 = ctx.createBoxShadow(posX, posY, sizeW, sizeH, 0, "#2CDE85");
            bs2.setTopLeftRadius(0.1 * sizeH);
            bs2.setTopRightRadius(0.2 * sizeH);
            bs2.setBottomRightRadius(0.3 * sizeH);
            bs2.setBottomLeftRadius(0.4 * sizeH);
            ctx.drawBoxShadow(bs2);

        }
        function drawImagePatternRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 2;
            const types = 7;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - (types + 1) * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = 6 * sizeH + 7 * margin;

            let ip2 = ctx.createPattern("../shared/qtlogo.png", 'repeat');
            ctx.fillStyle = ip2;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            let ip3 = ctx.createPattern("../shared/qtlogo.png", 'repeat');
            ip3.setImageSize(16, 12);
            ip3.setStartPosition(5, 5);
            ip3.setRotation(0.25 * Math.PI);
            ip3.setTintColor("#ff80d080");
            ctx.fillStyle = ip3;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }
    }
}
