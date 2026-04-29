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
            const cX = w * 0.5;
            const cY = h * 0.5;
            const s = Math.min(w, h);
            ctx.strokeStyle = "#000000";
            ctx.fillStyle = "#e0e0e0";
            drawFlower(ctx, cX, cY, 26);
            const flowers = 5;
            for (let i = 0; i < flowers; i++) {
                ctx.resetTransform();
                ctx.translate(cX, cY);
                ctx.rotate((i / flowers) * Math.PI * 0.5);
                ctx.translate(-cX, -cY);
                ctx.fillStyle = Qt.rgba(1 - (i / flowers), (i / flowers), 1, 1);
                const sr = s * 0.25;
                drawFlower(ctx, cX + sr, cY - sr, 6);
                drawFlower(ctx, cX + sr, cY + sr, 10);
                drawFlower(ctx, cX - sr, cY + sr, 14);
                drawFlower(ctx, cX - sr, cY - sr, 18);
            }
            ctx.resetTransform();
        }

        function drawFlower(ctx, cX, cY, leafs) {
            const h = canvas.height;
            const rotateAngle = (2 * Math.PI) / leafs;
            const leafHeight = h / 2;
            const leafWidth = h / leafs * 2;
            const cRadius = h / 20;
            ctx.save();
            ctx.translate(cX, cY);
            ctx.scale(0.4, 0.4);
            ctx.translate(-cX, -cY);
            ctx.beginPath();
            for (let i = 0; i < leafs; i++) {
                ctx.translate(cX, cY);
                ctx.rotate(rotateAngle);
                ctx.moveTo(0.5 * leafWidth, 0.5 * leafHeight);
                ctx.ellipse(0, 0.5 * leafHeight, 0.5 * leafWidth, 0.5 * leafHeight, 0, 0, 2 * Math.PI);
                ctx.translate(-cX, -cY);
            }
            ctx.lineWidth = 8;
            ctx.fill();
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(cX, cY, cRadius, 0, 2 * Math.PI);
            ctx.fillStyle = "#f0c060";
            ctx.fill();
            ctx.restore();
        }
    }
}
