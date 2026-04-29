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
            const rects = 3;
            const margin = w * 0.1;
            const sizeW = w / rects - margin;
            const sizeH = sizeW * 0.6;
            let posY = margin / 2;
            let posX = margin / 2;
            drawCompositeItem(ctx, posX, posY, sizeW, sizeH, "source-over");
            posX += sizeW + margin;
            drawCompositeItem(ctx, posX, posY, sizeW, sizeH, "source-atop");
            posX += sizeW + margin;
            drawCompositeItem(ctx, posX, posY, sizeW, sizeH, "destination-out");

            ctx.globalAlpha = 0.5;
            posY += sizeH * 1.5;
            posX = margin / 2;
            drawCompositeItem(ctx, posX, posY, sizeW, sizeH, "source-over");
            posX += sizeW + margin;
            drawCompositeItem(ctx, posX, posY, sizeW, sizeH, "source-atop");
            posX += sizeW + margin;
            drawCompositeItem(ctx, posX, posY, sizeW, sizeH, "destination-out");
            ctx.globalAlpha = 1.0;

            ctx.globalCompositeOperation = "source-over";
        }

        function drawCompositeItem(ctx, x, y, w, h, mode) {
            ctx.globalCompositeOperation = "source-over";
            ctx.beginPath();
            ctx.rect(x, y, w, h * 0.7, w * 0.2);
            ctx.fillStyle = "#DFD0B8";
            ctx.fill();
            ctx.strokeStyle = "#ffffff";
            ctx.lineWidth = 4;
            ctx.stroke();

            ctx.globalCompositeOperation = mode;
            ctx.beginPath();
            const cx = x + w * 0.5;
            const cy = y + h * 0.8;
            const cr = h * 0.4;
            ctx.arc(cx, cy, cr, 0, 2 * Math.PI);
            let rg1 = ctx.createRadialGradient(cx, cy, 0, cx, cy, cr);
            rg1.addColorStop(0, "#ff8080");
            rg1.addColorStop(1, "#401010");
            ctx.fillStyle = rg1;
            ctx.fill();
        }
    }
}
