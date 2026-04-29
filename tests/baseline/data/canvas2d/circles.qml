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
            const m = canvas.width * 0.1;
            const w = canvas.width - 2 * m;
            const h = canvas.height;
            const circleCount = 200;
            const xMovement = w / circleCount;
            ctx.lineWidth = 2;
            ctx.lineJoin = "miter";
            ctx.lineCap = "butt";
            ctx.beginPath();
            ctx.strokeStyle = "#2CDE85";
            for (let i = 0; i < circleCount; i++) {
                let posY = h * 0.5 + h * 0.3 * Math.sin(0.05 * xMovement * i);
                let posX = m + i * xMovement;
                let circleSize = 50 + 40 * Math.sin(i * 0.05 * xMovement);
                ctx.arc(posX, posY, circleSize, 0, 2 * Math.PI);
            }
            ctx.stroke();
        }
    }
}
