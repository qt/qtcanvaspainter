import QtQuick
import QtCanvas2D

Rectangle {
    id: rootItem
    height: 400
    width: 400
    color: "black"

    Canvas2D {
        id: canvas
        anchors.fill: parent
        alphaBlending: true
        fillColor: "black"
        onPaint: {
            var ctx = canvas.getContext("2d");
            drawLinearGradientRects(ctx);
            drawRadialGradientRects(ctx);
            drawConicalGradientRects(ctx);
        }

        function drawLinearGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 5;
            const types = 3;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - 4 * margin) / types;
            const animationSine = 0.5;
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

            let g3 = ctx.createLinearGradient(posX, posY, posX + sizeW, posY + sizeH);
            g3.addColorStop(0, Qt.rgba(animationSine, 1, 1, 1));
            g3.addColorStop(1, Qt.rgba(1, animationSine, 1, 1 - animationSine));
            ctx.fillStyle = g3;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            let g4 = ctx.createLinearGradient(posX, posY, posX, posY + sizeH);
            g4.addColorStop(0.0, "black");
            g4.addColorStop(0.2, "red");
            g4.addColorStop(0.4, "lime");
            g4.addColorStop(0.6, "blue");
            g4.addColorStop(0.8, "white");
            g4.addColorStop(1.0, "transparent");
            ctx.fillStyle = g4;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }

        function drawRadialGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 5;
            const types = 3;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - 4 * margin) / types;
            const animationSine = 0.5;
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

        function drawConicalGradientRects(ctx) {
            const w = canvas.width;
            const h = canvas.height;
            const rects = 5;
            const types = 3;
            const margin = w * 0.02;
            const sizeW = w / (rects + 1) - margin;
            const sizeH = (h - 4 * margin) / types;
            const animationSine = 0.5;
            let posX = sizeW * 0.5 + margin;
            let posY = 2 * sizeH + 3 * margin;

            let cX = posX + sizeW / 2;
            let cY = posY + sizeH / 2;
            let g1 = ctx.createConicalGradient(cX, cY, 0);
            g1.addColorStop(0, Qt.rgba(1, 1, 1, 1));
            g1.addColorStop(1, Qt.rgba(0, 0, 0, 0));
            ctx.fillStyle = g1;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            cX = posX + sizeW / 2;
            let g2 = ctx.createConicalGradient(cX, cY, animationSine);
            g2.addColorStop(0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1, Qt.rgba(1, 0, 0, 1));
            ctx.fillStyle = g2;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            cX = posX + sizeW / 2;
            let g3 = ctx.createConicalGradient(cX, cY, -0.5 * Math.PI);
            g3.addColorStop(0, Qt.rgba(animationSine, 1, 1));
            g3.addColorStop(1, Qt.rgba(1, animationSine, 1, 1 - animationSine));
            ctx.fillStyle = g3;
            ctx.fillRect(posX, posY, sizeW, sizeH);
            posX += sizeW + margin;

            cX = posX + sizeW / 2;
            let g4 = ctx.createConicalGradient(posX + animationSine * sizeW, cY, 0);
            g4.addColorStop(0.00, Qt.rgba(1, 1, 0));
            g4.addColorStop(0.25, Qt.rgba(0, 1, 0));
            g4.addColorStop(0.50, Qt.rgba(0, 0, 1));
            g4.addColorStop(0.75, Qt.rgba(1, 0, 1));
            g4.addColorStop(1.00, Qt.rgba(1, 1, 0));
            ctx.fillStyle = g4;
            ctx.fillRect(posX, posY, sizeW, sizeH);
        }
    }
}
