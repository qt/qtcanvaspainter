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
            const m = h * 0.3;
            ctx.save();
            ctx.beginPath();
            ctx.strokeStyle = "#b08020";
            ctx.lineWidth = 2;
            ctx.lineJoin = "miter";
            ctx.rect(m, m, w - 2 * m, h - 2 * m);
            ctx.stroke();
            ctx.setClipRect(m, m, w - 2 * m, h - 2 * m);
            // circles inside the clip region
            const cm = w * 0.1;
            const cw = w - 2 * cm;
            const circleCount = 200;
            const xMovement = cw / circleCount;
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.strokeStyle = "#2CDE85";
            for (let i = 0; i < circleCount; i++) {
                let posY = h * 0.5 + h * 0.3 * Math.sin(0.05 * xMovement * i);
                let posX = cm + i * xMovement;
                let circleSize = 50 + 40 * Math.sin(i * 0.05 * xMovement);
                ctx.arc(posX, posY, circleSize, 0, 2 * Math.PI);
            }
            ctx.stroke();
            ctx.restore();
        }
    }
}
