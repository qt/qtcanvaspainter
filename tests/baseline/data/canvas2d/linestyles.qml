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
            const m = h * 0.1;
            const lines = 3;
            const lineCount = 100;
            const xMovement = (w - 2 * m) / lineCount;
            const lineJoins = ["bevel", "round", "miter"];
            const lineCaps = ["butt", "round", "square"];
            ctx.lineWidth = 15;
            ctx.miterLimit = 5;
            const setH = (h - 2 * m) / lines;
            let setY = m + 0.5 * setH;
            for (let j = 0; j < lines; j++) {
                ctx.lineJoin = lineJoins[j % 3];
                ctx.lineCap = lineCaps[j % 3];
                ctx.beginPath();
                ctx.strokeStyle = Qt.rgba(1, 0.2 * (j % 3), 0.6 - 0.2 * (j % 3), 1);
                for (let i = 0; i < lineCount; i++) {
                    let posY = setY + 0.5 * setH * Math.sin(1.2 * i + t) * Math.sin(0.1 * Math.PI * i);
                    let posX = m + i * xMovement;
                    if (i === 0)
                        ctx.moveTo(posX, posY);
                    else
                        ctx.lineTo(posX, posY);
                }
                setY += setH;
                ctx.stroke();
            }
        }
    }
}
