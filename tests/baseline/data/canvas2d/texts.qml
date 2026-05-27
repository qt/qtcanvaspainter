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
            ctx.font = "16px sans-serif";
            ctx.fillStyle = "#e0e0e0";
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            const texts = 10;
            const yMovement = (h - m * 2) / texts;
            for (let i = 0; i < texts; i++) {
                const xPos = w * 0.5 + w * 0.4 * Math.sin(0.1 * t + i * 0.5);
                const yPos = m + i * yMovement;
                const s = "pos:(" + xPos.toFixed(0) + ", " + yPos.toFixed(0) + ")";
                ctx.fillText(s, xPos, yPos);
            }

            // Text antialias
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.font = "22px sans-serif";
            ctx.textAntialias = 1.0;
            ctx.fillText("Antialiasing: 1.0", 100, 100);
            ctx.textAntialias = 2.0;
            ctx.fillText("Antialiasing: 2.0", 100, 130);
            ctx.textAntialias = 4.0;
            ctx.fillText("Antialiasing: 4.0", 100, 160);
        }
    }
}
