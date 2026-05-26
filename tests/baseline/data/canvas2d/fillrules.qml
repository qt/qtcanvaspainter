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
            ctx.lineWidth = 4;
            ctx.fillStyle = "#e0e040";
            ctx.strokeStyle = "#f04000";

            function paintStar(ctx, fillRule) {
                ctx.beginPath();
                ctx.moveTo(120, 60);
                for (let i = 1; i < 6; ++i) {
                    ctx.lineTo(60 + 60 * Math.cos(0.8 * i * Math.PI),
                               60 + 60 * Math.sin(0.8 * i * Math.PI));
                }
                if (fillRule)
                    ctx.fill(fillRule);
                else
                    ctx.fill();
                ctx.stroke();
            };
            // Set fillRule
            ctx.fillRule = "nonzero";
            paintStar(ctx, "");
            ctx.translate(0, 140);
            ctx.fillRule = "evenodd";
            paintStar(ctx, "");
            // Override fillRule with fill() parameter
            ctx.translate(120, -140);
            paintStar(ctx, "nonzero");
            ctx.translate(0, 140);
            paintStar(ctx, "evenodd");
        }
    }
}
