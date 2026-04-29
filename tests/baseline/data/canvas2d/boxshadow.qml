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
        fillColor: "gray"
        onPaint: {
            var ctx = canvas.getContext("2d");
            const w = canvas.width;
            const h = canvas.height;
            const rects = 3;
            const margin = w * 0.15;
            const sizeW = w / rects - margin;
            const sizeH = h - 2 * margin;
            const animationSine = 0.5;
            let posX = margin * 0.5;
            let posY = margin;

            let bs1 = ctx.createBoxShadow(posX, posY, sizeW, sizeH, animationSine * 0.3 * sizeW, "black", 0.1 * sizeW);
            ctx.drawBoxShadow(bs1);

            ctx.translate(sizeW + margin, 0);
            bs1 = ctx.createBoxShadow(posX, posY, sizeW, sizeH, 0.1 * sizeW, "#2CDE85",
                                      0.2 * sizeW,
                                      0.4 * sizeW,
                                      0.2 * sizeW,
                                      0.0 * sizeW);
            ctx.drawBoxShadow(bs1);
            ctx.translate(sizeW + margin, 0);

            const blur = sizeW * 0.2;
            const radius = sizeW * 0.05;
            const shadowOffsetX = blur * 0.3;
            const shadowOffsetY = blur * 0.3;
            const c = "#202020";
            ctx.globalAlpha = 0.05;
            let shadow1 = ctx.createBoxShadow(posX + shadowOffsetX, posY - shadowOffsetY, sizeW, sizeH, blur, Qt.lighter(c, 200), radius);
            ctx.drawBoxShadow(shadow1);
            ctx.globalAlpha = 0.3;
            let shadow2 = ctx.createBoxShadow(posX - shadowOffsetX, posY + shadowOffsetY, sizeW, sizeH, blur, Qt.darker(c, 250), radius);
            ctx.drawBoxShadow(shadow2);
            ctx.globalAlpha = 1.0;

            ctx.beginPath();
            ctx.roundRect(posX, posY, sizeW, sizeH, radius);
            ctx.fillStyle = c;
            ctx.fill();
        }
    }
}
