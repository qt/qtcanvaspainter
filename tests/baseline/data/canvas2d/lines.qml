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
            const t = 0.0;
            const lines = 5;
            const lineCount = 1000;
            const xMovement = w / lineCount;
            ctx.lineWidth = 2;
            ctx.lineJoin = "miter";
            ctx.lineCap = "butt";
            for (let j = 0; j < lines; j++) {
                ctx.beginPath();
                ctx.strokeStyle = Qt.rgba((j / lines), 0.5, 1, 1);
                for (let i = 0; i < lineCount; i++) {
                    let posY = h * 0.5 + h * 0.4 * Math.sin(0.05 * xMovement * i + t);
                    let posX = i * xMovement + j * (100 / lines);
                    if (i === 0)
                        ctx.moveTo(posX, posY);
                    else
                        ctx.lineTo(posX, posY);
                }
                ctx.stroke();
            }
        }
    }
}
