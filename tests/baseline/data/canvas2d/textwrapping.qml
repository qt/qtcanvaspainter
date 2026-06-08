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

            ctx.fillStyle = "black";
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.strokeStyle = "gray";
            ctx.fillStyle = "white";
            ctx.lineWidth = 1;
            ctx.textLineHeight = -10;
            ctx.font = "20px 'Titillium Web'";

            // canvas2d-textwrapmode
            ctx.strokeRect(50, 5, 100, 60);
            ctx.strokeRect(50, 70, 100, 60);
            ctx.strokeRect(50, 135, 100, 60);
            let s = "This is a long string.";
            ctx.textWrapMode = "nowrap";
            ctx.fillText(s, 50, 5, 100, 60);
            ctx.textWrapMode = "wrap";
            ctx.fillText(s, 50, 70, 100, 60);
            ctx.textWrapMode = "wrapanywhere";
            ctx.fillText(s, 50, 135, 100, 60);

            // canvas2d-textlineheight
            ctx.textWrapMode = "wrap";
            ctx.strokeRect(240, 5, 120, 60);
            ctx.strokeRect(240, 70, 120, 60);
            ctx.strokeRect(240, 135, 120, 60);
            ctx.textLineHeight = -10;
            ctx.fillText("Text with line height: -10", 240, 5, 120, 60);
            ctx.textLineHeight = 0;
            ctx.fillText("Text with line height: 0", 240, 70, 120, 60);
            ctx.textLineHeight = 10;
            ctx.fillText("Text with line height: 10", 240, 135, 120, 60);
            ctx.textWrapMode = "nowrap";

            // canvas2d-textdirection
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.direction = "ltr";
            ctx.fillText("Hi ltr!", 100, 250);
            ctx.direction = "rtl";
            ctx.fillText("Hi rtl!", 300, 250);
            ctx.direction = "inherit";
        }
    }
}
