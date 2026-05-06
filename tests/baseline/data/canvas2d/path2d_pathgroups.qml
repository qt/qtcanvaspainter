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
            const t = 0.0;
            const complexity = 1;
            const personPathGroup = 1;
            const heartPathGroup = 2;

            // Person icon rendered via an SVG path string
            const svgPath = "M152,84a36,36,0,1,0-36-36A36.04061,36.04061,0,0,0,152,84Zm0-48Z" +
                "m64,112a12.00028,12.00028,0,0,1-12,12c-37.20215,0-55.50781-19.66406" +
                "-70.21729-35.46484-.45459-.48828-.89794-.96192-1.34667-1.44239l-8.02393," +
                "18.45484,34.5625,24.68774A11.999,11.999,0,0,1,164,176v56a12,12,0,0,1-24," +
                "0V182.17578l-25.37207-18.12353L83.00488,236.78516a12.00021,12.00021,0,0," +
                "1-22.00976-9.57032l37.55322-86.37255.02881-.06592,13.74463-31.61377c-" +
                "8.09571-.96167-18.24219,2.0072-30.35889,8.94482a159.5463,159.5463,0,0,0-" +
                "29.47754,22.37793,12.0001,12.0001,0,0,1-16.9707-16.9707A183.31075," +
                "183.31075,0,0,1,70.03711,97.28027c36.06689-20.65234,58.03027-11.69433," +
                "70.104-.54589,3.93116,3.63085,7.63037,7.60449,11.208,11.44726C164.667," +
                "122.4873,177.24561,136,204,136A12.00028,12.00028,0,0,1,216,148Z";
            const personPath = ctx.createPath2D(svgPath);

            var d = Math.min(w, h) * 0.1;
            ctx.lineWidth = d * 0.1;
            ctx.fillStyle = "#FFFFFF";

            for (let j = 0; j < complexity; j++) {
                let cx = 120;
                let cy = 120;
                let px = w * 0.2 - cx;
                let py = h * 0.5 - cy;
                ctx.translate(px, py);
                ctx.translate(cx, cy);
                ctx.rotate(Math.sin(j * 0.2 + t) * 0.5);
                ctx.scale(1.0 + 0.6 * Math.sin((j + t) * 0.1));
                ctx.translate(-cx, -cy);
                ctx.strokeStyle = Qt.rgba(0, 1 - (j + 1) / complexity, 0);
                ctx.fill(personPath, personPathGroup);
                ctx.stroke(personPath, personPathGroup);
                ctx.resetTransform();
            }

            // Heart shapes built incrementally via path2D
            ctx.fillStyle = "#E02020";
            ctx.strokeStyle = "#000000";
            ctx.lineWidth = d * 0.05;

            const heartPath = ctx.createPath2D();
            heartPath.moveTo(0, d / 4);
            heartPath.quadraticCurveTo(0, 0, d / 4, 0);
            heartPath.quadraticCurveTo(d / 2, 0, d / 2, d / 4);
            heartPath.quadraticCurveTo(d / 2, 0, d * 3/4, 0);
            heartPath.quadraticCurveTo(d, 0, d, d / 4);
            heartPath.quadraticCurveTo(d, d / 2, d * 3/4, d * 3/4);
            heartPath.lineTo(d / 2, d);
            heartPath.lineTo(d / 4, d * 3/4);
            heartPath.quadraticCurveTo(0, d / 2, 0, d / 4);

            const items = 20 * complexity;
            for (let i = 0; i < items; i++) {
                let posX = 0.4 * w + 0.5 * (w - d) * (i / items);
                let posY = (h - d) * 0.5 + (h - d) * 0.3 * Math.sin(i / items * 10 + t);
                ctx.resetTransform();
                ctx.translate(posX, posY);
                ctx.stroke(heartPath, heartPathGroup);
                ctx.fill(heartPath, heartPathGroup);
            }
        }
    }
}
