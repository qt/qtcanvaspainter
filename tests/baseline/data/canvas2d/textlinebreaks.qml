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

            ctx.font = "14px 'Titillium Web'";
            ctx.lineWidth = 1;

            function marker(x, y) {
                ctx.strokeStyle = "red";
                ctx.beginPath();
                ctx.moveTo(x - 6, y);
                ctx.lineTo(x + 6, y);
                ctx.moveTo(x, y - 6);
                ctx.lineTo(x, y + 6);
                ctx.stroke();
            }

            // Line breaks with each text align and baseline,
            // in the default "nowrap" wrap mode
            let aligns = [ "left", "center", "right" ];
            let baselines = [ "top", "hanging", "middle", "alphabetic", "bottom" ];
            ctx.fillStyle = "white";
            for (let a = 0; a < aligns.length; ++a) {
                for (let b = 0; b < baselines.length; ++b) {
                    let x = 70 + a * 130;
                    let y = 25 + b * 62;
                    ctx.textAlign = aligns[a];
                    ctx.textBaseline = baselines[b];
                    ctx.fillText(aligns[a] + "\n" + baselines[b], x, y);
                    marker(x, y);
                }
            }

            // "\r\n" and consecutive line breaks
            ctx.textAlign = "left";
            ctx.textBaseline = "top";
            ctx.strokeStyle = "gray";
            ctx.fillStyle = "lightgreen";
            ctx.strokeRect(10, 305, 120, 90);
            ctx.fillText("CR LF\r\nline\n\nafter empty", 10, 305, 120, 90);

            // Line breaks combined with word wrapping
            ctx.textWrapMode = "wordwrap";
            ctx.strokeRect(140, 305, 120, 90);
            ctx.fillText("Wrapped words\nand a line break", 140, 305, 120, 90);
            ctx.textAlign = "right";
            ctx.textBaseline = "bottom";
            ctx.strokeRect(270, 305, 120, 90);
            ctx.fillText("Right bottom\nwrapped words here", 270, 305, 120, 90);
            ctx.textWrapMode = "nowrap";
        }
    }
}
