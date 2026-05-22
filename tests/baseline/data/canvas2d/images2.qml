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
            const sizeW = w / 3;
            let posX = 0;

            // Create and use patterns in different order
            var pattern1 = ctx.createPattern("../shared/qtlogo.png", 'repeat');
            var pattern2 = ctx.createPattern("../shared/face-smile.png", 'repeat');
            var pattern3 = ctx.createPattern("../shared/pattern1.png", 'repeat');
            pattern1.setImageSize(64, 46);
            ctx.fillStyle = pattern1;
            ctx.fillRect(posX, 0, sizeW, h * 0.5);
            ctx.drawImage("../shared/qtlogo.png", posX, h * 0.6, sizeW, h * 0.3);
            posX += sizeW;
            ctx.fillStyle = pattern3;
            ctx.fillRect(posX, 0, sizeW, h * 0.5);
            ctx.drawImage("../shared/pattern1.png", posX, h * 0.6, sizeW, h * 0.3);
            posX += sizeW;
            ctx.fillStyle = pattern2;
            ctx.fillRect(posX, 0, sizeW, h * 0.5);
            ctx.drawImage("../shared/face-smile.png", posX, h * 0.6, sizeW, h * 0.3);
        }
        Component.onCompleted: {
            loadImage("../shared/qtlogo.png");
            loadImage("../shared/face-smile.png");
            loadImage("../shared/pattern1.png");
            canvas.requestPaint();
        }
        onImageLoaded: canvas.requestPaint()
    }
}
