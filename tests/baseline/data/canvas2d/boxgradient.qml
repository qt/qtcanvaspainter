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
            const rects = 4;
            const margin = w * 0.02;
            const size = w / (rects + 1) - margin;
            const animationSine = 0.5;
            let posX = size * 0.5 + margin;
            let posY = size * 0.5;

            // No stops, so transparent
            const g1 = ctx.createBoxGradient(posX, posY, size, size, size / 2);
            ctx.fillStyle = g1;
            ctx.fillRect(posX, posY, size, size);
            posX += size + margin;

            const r2 = size / 4;
            const f2 = size / 2 - animationSine * size / 3;
            const g2 = ctx.createBoxGradient(posX, posY, size, size, f2, r2);
            g2.addColorStop(0.0, Qt.rgba(0, 1, 0, 1));
            g2.addColorStop(1.0, Qt.rgba(1, 0, 0, 1));
            ctx.fillStyle = g2;
            ctx.fillRect(posX, posY, size, size);
            posX += size + margin;

            const r3 = size / 4;
            const f3 = size / 4;
            const g3 = ctx.createBoxGradient(posX, posY, size, size, f3, r3);
            g3.addColorStop(0.0, Qt.rgba(animationSine, 1, 1, 1));
            g3.addColorStop(1.0, Qt.rgba(1, animationSine, 1, 1 - animationSine));
            ctx.fillStyle = g3;
            ctx.fillRect(posX, posY, size, size);
            posX += size + margin;

            const r4 = size / 3;
            const f4 = size / 4;
            const g4 = ctx.createBoxGradient(posX, posY, size, size, f4, r4);
            g4.addColorStop(0.0, Qt.rgba(1, 1, 1, 1));
            g4.addColorStop(0.2, Qt.rgba(0, 0, 0, 1));
            g4.addColorStop(0.4, Qt.rgba(1, animationSine, 0, 1));
            g4.addColorStop(0.6, Qt.rgba(0, 0, 0, 1));
            g4.addColorStop(0.8, Qt.rgba(1, 1, 1, 1));
            g4.addColorStop(1.0, Qt.rgba(0, 0, 0, 0));
            ctx.fillStyle = g4;
            ctx.fillRect(posX, posY, size, size);
        }
    }
}
