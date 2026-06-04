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

            // Test winding
            ctx.resetTransform();
            ctx.beginPath();
            ctx.roundRect(260, 20, 120, 100, 20);
            ctx.setPathWinding("clockwise");
            ctx.circle(320, 70, 40);
            ctx.fill();
            ctx.stroke();

            // Test winding - with path
            ctx.translate(0, 140);
            const windingPath = ctx.createPath2D();
            windingPath.roundRect(260, 20, 120, 100, 20);
            windingPath.setPathWinding("clockwise");
            windingPath.circle(320, 70, 40);
            ctx.fill(windingPath);
            ctx.stroke(windingPath);

            // Test highQualityStroking
            ctx.resetTransform();
            // Increase the line width and reduce opacity to get
            // the stroking flaws visible.
            ctx.lineWidth = 20;
            ctx.globalAlpha = 0.5;
            ctx.lineCap = "round";
            ctx.lineJoin = "round";
            let myPath = ctx.createPath2D();
            myPath.moveTo(30, 300);
            myPath.lineTo(40, 300);
            myPath.lineTo(30, 320);
            myPath.lineTo(80, 300);
            myPath.lineTo(80, 310);
            ctx.highQualityStroking = true;
            ctx.stroke(myPath);
            ctx.highQualityStroking = false;
            ctx.translate(0, 50);
            ctx.stroke(myPath);

            // Test windingEnforce
            ctx.reset();
            ctx.fillStyle = "red";
            ctx.strokeStyle = "white";
            function paintTriangles(ctx) {
                ctx.beginPath();
                // Outer shape, counterclockwise
                ctx.moveTo(120, 300);
                ctx.lineTo(180, 380);
                ctx.lineTo(240, 300);
                ctx.closePath();
                // Inner shape, clockwise
                ctx.moveTo(180, 305);
                ctx.lineTo(210, 335);
                ctx.lineTo(150, 335);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();
            }
            ctx.windingEnforce = false;
            paintTriangles(ctx)
            ctx.translate(140, 0);
            ctx.windingEnforce = true;
            paintTriangles(ctx)
        }
    }
}
