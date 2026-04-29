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
            const margin = w * 0.2;
            const marginY = h * 0.05;
            const size = w / 3 - margin;
            const lineWidth = 1 + w * 0.004;
            const renders = 20;
            for (let i = 0; i < renders; i++) {
                let posX = margin / 2 + (i / renders) * margin;
                let posY = marginY + (i / (renders - 1)) * (h - size - 2 * marginY);

                ctx.fillStyle = "#D0D0D0";
                ctx.strokeStyle = "#f0f0f0";
                ctx.lineWidth = lineWidth;
                ctx.beginPath();
                ctx.rect(posX, posY, size, size);
                ctx.fill();
                ctx.stroke();
                ctx.save();

                posX += size + margin * 0.5;
                ctx.fillStyle = "#900000";
                ctx.strokeStyle = "#E00000";
                ctx.lineWidth = lineWidth * 0.5;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                ctx.translate(posX + size / 2, posY + size / 2);
                ctx.rotate(1.0 + t);
                const scale = 0.75 + 0.25 * Math.sin(10 * (i / renders) + t);
                ctx.scale(scale, scale);
                ctx.translate(-(posX + size / 2), -(posY + size / 2));
                ctx.beginPath();
                ctx.rect(posX, posY, size, size);
                ctx.fill();
                ctx.stroke();

                posX += size + margin * 0.5;
                ctx.restore();
                ctx.beginPath();
                ctx.rect(posX, posY, size, size);
                ctx.fill();
                ctx.stroke();
            }
        }
    }
}
