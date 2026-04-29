import QtQuick
import QtCanvas2D

Rectangle {
    id: rootItem
    height: 600
    width: 600
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
            const m = w * 0.1;
            const my = h * 0.15;
            const w2 = w * 0.5;
            const animationSine = 0.5;
            const lines = 9;
            ctx.lineWidth = h * 0.02;
            ctx.strokeStyle = "#2CDE85";
            ctx.fillStyle = "#00414A";
            for (let i = 0; i < lines; i++) {
                let y = my + i / (lines - 1) * (h - 2 * m);
                ctx.antialias = 1 + i;
                ctx.beginPath();
                ctx.moveTo(m, y);
                ctx.bezierCurveTo(m + w2 * 0.2, y + 0.1 * h * animationSine,
                                  m + w2 * 0.6, y - 0.1 * h * animationSine,
                                  m + w2 - m, y);
                ctx.stroke();
            }
            ctx.antialias = 5.5;
            ctx.beginPath();
            ctx.roundRect(w2 + m, my, w2 - 2 * m, h - 2 * m,
                          w2 * 0.1, w2 * 0.2, w2 * 0.1, w2 * 0.3);
            ctx.lineWidth = h * 0.04;
            ctx.fill();
            ctx.stroke();
        }
    }
}
