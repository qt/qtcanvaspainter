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
            const w = canvas.width;
            const h = canvas.height;
            const sizeW = w * 0.8;
            const sizeH = h * 0.15;
            const margin = h * 0.04;
            let posX = w * 0.5 - sizeW * 0.5;
            let posY = margin;
            const animationSine = 0.2;
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

        function drawButton(ctx, x, y, w, h, label) {
            const border = h * 0.1;
            ctx.lineWidth = border;
            const cx = x + 0.5 * w;
            const cy = y + 0.5 * h;
            const g1 = ctx.createLinearGradient(cx - 0.25 * w, cy - 0.25 * w,
                                                cx + 0.25 * h, cy + 0.25 * w);
            g1.addColorStop(0.0, "#FF0000");
            g1.addColorStop(0.5, "#D0D000");
            g1.addColorStop(1.0, "#000000");
            const g2 = ctx.createLinearGradient(0, y, 0, y + h);
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
            ctx.font = (h * 0.4) + "px sans-serif";
            ctx.fillStyle = "#FFFFFF";
            ctx.fillText(label, cx, cy);
        }
    }
}
