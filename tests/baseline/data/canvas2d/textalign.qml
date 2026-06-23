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
            const cX = w * 0.7;

            ctx.beginPath();
            ctx.lineWidth = 2;
            ctx.strokeStyle = "red";
            ctx.moveTo(cX, h * 0.1);
            ctx.lineTo(cX, h * 0.9);
            ctx.stroke();

            ctx.lineWidth = 2;
            ctx.font = "30px sans-serif";
            ctx.fillStyle = "#e0e0e0";
            ctx.textBaseline = "top";
            const alignments = ["start", "end", "left", "center", "right"];
            alignments.forEach((alignment, index) => {
                const y = h * 0.1 + index * h * 0.15;
                ctx.textAlign = alignment;
                const s = "Align (" + alignment + ")";
                ctx.fillText(s, cX, y);
            });

            ctx.textAlign = "left";
            const baselines = ["top", "hanging", "middle", "alphabetic", "ideographic", "bottom"];
            baselines.forEach((baseline, index) => {
                ctx.textBaseline = baseline;
                const y = h * 0.1 + index * h * 0.15;
                ctx.beginPath();
                ctx.moveTo(m, y + 0.5);
                ctx.lineTo(m + w * 0.4, y + 0.5);
                ctx.strokeStyle = "red";
                ctx.stroke();
                ctx.lineWidth = 2;
                const s = "Baseline (" + baseline + ")";
                ctx.fillText(s, m, y);
            });
        }
    }
}
