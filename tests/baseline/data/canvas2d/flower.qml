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
        fillColor: "white"
        onPaint: {
            var ctx = canvas.getContext("2d");
            const w = rootItem.width;
            const h = rootItem.height;
            const s = Math.min(w, h);
            const t = 0;
            const flowerSize = 80.0 + s * 0.6;
            drawFlower(ctx, w / 2 - flowerSize / 2, h - flowerSize, flowerSize, flowerSize, t);
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

        function _flowerPos(i)
        {
            const items = 12;
            return (2 * Math.PI) * (1 - i / items) - Math.PI / 2;
        }
    }
}
