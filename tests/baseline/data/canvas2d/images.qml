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
            ctx.fillStyle = "#000000";
            ctx.fillRect(0, m, w, h - m);
            var pattern = ctx.createPattern("../shared/qtlogo.png", 'repeat');
            ctx.fillStyle = pattern;
            ctx.fillRect(0, m, w, h - m);
            const images = 100;
            const imageSize = w * 0.05;
            const yMovement = (h - m * 2) / images;
            for (let i = 0; i < images; i++) {
                const xPos = -0.5 * imageSize + w * 0.5 + w * 0.4 * Math.sin(0.1 * t + i);
                const yPos = m + i * yMovement;
                ctx.drawImage("../shared/face-smile.png", xPos, yPos, imageSize, imageSize);
            }
        }
        Component.onCompleted: {
            loadImage("../shared/qtlogo.png", Qt.size(32, 23));
            loadImage("../shared/face-smile.png");
            canvas.requestPaint();
        }
        onImageLoaded: canvas.requestPaint()
    }
}
