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
    }
}
