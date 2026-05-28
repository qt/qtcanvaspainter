import QtQuick
import QtCanvas2D

Rectangle {
    id: rootItem
    height: 400
    width: 400
    color: "black"

    Canvas2D {
        id: canvas

        property transform2d t1

        anchors.fill: parent
        alphaBlending: true
        fillColor: "black"
        onPaint: {
            var ctx = canvas.getContext("2d");
            const w = canvas.width;
            const h = canvas.height;
            const cX = w * 0.5;
            const cY = h * 0.5;
            const s = Math.min(w, h);
            ctx.fillStyle = "#ff0000";
            ctx.strokeStyle = "#ffffff";
            ctx.lineWidth = 10;
            // Rectangles
            ctx.fillRect(10, 10, 90, 90)
            ctx.translate(100, 0);
            ctx.fillRect(10, 10, 90, 90)
            ctx.translate(100, 0);
            ctx.scale(0.5, 0.5);
            ctx.fillRect(10, 10, 90, 90)
            ctx.translate(100, 0);
            ctx.scale(2.0, 2.0);
            ctx.fillRect(10, 10, 90, 90)
            ctx.resetTransform();
            ctx.strokeRect(10, 10, 90, 90)

            // translate + transform (QTBUG-28511)
            ctx.translate(10, 250);
            ctx.save();
            ctx.fillStyle = "red";
            ctx.fillRect(0, 0, 100, 100);
            ctx.restore();
            ctx.save();
            ctx.transform(1, -1, 0, 1, 0, 0);
            ctx.fillStyle = "green";
            ctx.fillRect(0, 0, 100, 100);
            ctx.restore();

            // transform2d
            ctx.resetTransform();
            ctx.fillStyle = "#0000ff";
            t1.reset();
            t1.translate(250, 250)
            t1.rotate(45)
            ctx.setTransform(t1);
            ctx.fillRect(-90, -90, 180, 180)
            t1.rotateRadians(Math.PI * 0.25)
            t1.scale(0.5)
            ctx.setTransform(t1);
            ctx.strokeRect(-90, -90, 180, 180)
            t1.shear(0.5, 0);
            ctx.setTransform(t1);
            ctx.strokeRect(-90, -90, 180, 180)
        }
    }
}
