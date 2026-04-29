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
            const animationSine = 0.5;
            const rects = 3;
            const margin = w * 0.04;
            const my = h * 0.2;
            const size = (w - 4 * margin) / rects;
            let posX = margin;
            let posY = my;

            const cellZoom = 0.2 + 1.0 * animationSine;
            const cp1x = posX + size * 0.5;
            const cp1y = posY + size * 0.5;
            const gp1 = ctx.createGridPattern(cp1x, cp1y, 0.1 * size * cellZoom, 0.1 * size * cellZoom,
                                              "#404040", "#202020");
            ctx.fillStyle = gp1;
            ctx.fillRect(posX, posY, size, size);
            const gp2 = ctx.createGridPattern(cp1x, cp1y, size * cellZoom, size * cellZoom,
                                              "#d0d0d0", "transparent");
            ctx.fillStyle = gp2;
            ctx.fillRect(posX, posY, size, size);

            posX += size + margin;
            const cp2x = posX + size * 0.5;
            const cp2y = posY + size * 0.5;
            const bar = size * 0.5;
            const gp3 = ctx.createGridPattern(0, 0, bar, 0,
                                              "#2CDE85", "00414A",
                                              bar * 0.5, bar * 0.5, Math.PI / 4);
            ctx.fillStyle = gp3;
            ctx.lineWidth = 0.5 * bar;
            const strokeW = (size / 5) * animationSine;
            const gp4 = ctx.createGridPattern(cp2x, cp2y, size / 5, size / 5,
                                              "black", "white",
                                              strokeW);
            ctx.strokeStyle = gp4;
            ctx.lineWidth = 4;
            ctx.beginPath();
            ctx.roundRect(posX, posY, size, size, 10);
            ctx.fill();
            ctx.stroke();

            posX += size + margin;
            const bar2 = size * 0.2;
            const gp5 = ctx.createGridPattern(15 * t, 0, bar2, 0,
                                              "#D0D040", "black",
                                              bar2 * 0.5, 1, Math.PI / 4);
            const gp6 = ctx.createGridPattern(0, 0, 0, bar2 * 0.25,
                                              "#404010", "#505010",
                                              bar2 * 0.1, 1, Math.PI / 4);
            ctx.strokeStyle = gp5;
            ctx.fillStyle = gp6;
            ctx.lineWidth = 0.05 * size;
            ctx.beginPath();
            ctx.roundRect(posX, posY, size, size, bar2);
            ctx.fill();
            ctx.stroke();
        }
    }
}
