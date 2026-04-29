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
            const w = canvas.width;
            const h = canvas.height;
            const t = 0.0;
            const rectSize = 80;
            const rectCount = 500;
            const yMovement = (h - rectSize) / rectCount;
            ctx.lineWidth = 1;
            ctx.lineJoin = "miter";
            ctx.lineCap = "butt";
            for (let i = 0; i < rectCount; i++) {
                ctx.fillStyle = Qt.rgba((i * 0.1) % 1, (i * 0.01) % 1, (i * 0.001) % 1, 0.1 + 0.9 * (i * 0.02) % 1);
                ctx.strokeStyle = ctx.fillStyle;
                if (i % 2 === 0)
                    ctx.fillRect(w / 2 + (w / 2) * Math.sin(0.1 * t + i * 2.0), i * yMovement, rectSize, rectSize);
                else
                    ctx.strokeRect(w / 2 + (w / 2) * Math.sin(0.1 * t + i * 2.0), i * yMovement, rectSize, rectSize);
            }
        }
    }
}
