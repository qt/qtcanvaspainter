// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasconicalgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasConicalGradient
    \since 6.11
    \brief QCanvasConicalGradient is a brush for conical gradient painting.
    \inmodule QtCanvasPainter

    Conical gradient interpolate colors between start and end points
    around a point with given coordinates. The angle starts from a line
    going horizontally right from the center, and proceeds clockwise.

    \image qcconicalgradient-gallery.webp
           {Four squares with conical gradients sweeping around a center
           point, varying in color}

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcconicalgradient-example.webp
        {Circle with a conical gradient sweeping from yellow through blue
        and back to yellow}
    \li
    \code
    QRectF rect(20, 20, 160, 160);
    QCanvasConicalGradient cg(rect.center(), 1.75 * M_PI);
    cg.setColorAt(0.0, "#fdbb2d");
    cg.setColorAt(0.5, "#1a2a6c");
    cg.setColorAt(1.0, "#fdbb2d");
    p->setFillStyle(cg);
    p->beginPath();
    p->ellipse(rect);
    p->fill();
    \endcode
    \endtable
*/

#define DECONST(d) const_cast<QCanvasConicalGradientBrushPrivate *>(d)

/*!
    Constructs a default conical gradient.
    Gradient center position is (0, 0).
    Gradient angle is 0.0.
*/

QCanvasConicalGradient::QCanvasConicalGradient()
    : QCanvasConicalGradient(0.0, 0.0, 0.0)
{
}

/*!
    Constructs a conical gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient start angle is \a startAngle.
*/

QCanvasConicalGradient::QCanvasConicalGradient(qreal centerX, qreal centerY, qreal startAngle)
    : QCanvasGradient(QCanvasBrush::BrushType::ConicalGradient)
{
    auto &conical = QCanvasGradientBrushPrivate::get(*this)->data.conical;
    conical.cx = centerX;
    conical.cy = centerY;
    conical.angle = startAngle;
}

/*!
    Constructs a conical gradient.
    Gradient center position is \a center.
    Gradient start angle is \a startAngle.
*/

QCanvasConicalGradient::QCanvasConicalGradient(QPointF center, qreal startAngle)
    : QCanvasConicalGradient(center.x(), center.y(), startAngle)
{
}

/*!
    Returns the center point of conical gradient.
    \sa setCenterPosition()
*/

QPointF QCanvasConicalGradient::centerPosition() const
{
    const auto &conical = QCanvasGradientBrushPrivate::get(*this)->data.conical;
    return QPointF(conical.cx, conical.cy);
}

/*!
    Sets the center point of conical gradient to ( \a x, \a y).
*/
void QCanvasConicalGradient::setCenterPosition(qreal x, qreal y)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.conical.cx = x;
    d->data.conical.cy = y;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasConicalGradient::setCenterPosition(QPointF center)
    \overload

    Sets the center point of conical gradient to \a center.
*/

/*!
    Returns the start angle of conical gradient in radians.
*/

qreal QCanvasConicalGradient::startAngle() const
{
    return QCanvasGradientBrushPrivate::get(*this)->data.conical.angle;
}

/*!
    Sets the start angle of conical gradient to \a angle in radians.
    The angle starts from a line going horizontally right from the
    center, and proceeds clockwise.
*/

void QCanvasConicalGradient::setStartAngle(qreal angle)
{
    auto *d = QCanvasGradientBrushPrivate::get(*this);
    d->data.conical.angle = angle;
    d->dirty |= QCanvasGradientBrushPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasConicalGradientBrushPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 0, 0, 0, 0 };
            QColor ocol = { 0, 0, 0, 0 };
            // Note: Without stops, custom image might be used.
            createConicalGradient(icol, ocol, d->imageId);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createConicalGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createConicalGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createConicalGradient(col, col, d->textureId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->paint.imageId > 0) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->paint.imageId);
    }
    return d->paint;
}

void QCanvasConicalGradientBrushPrivate::createConicalGradient(QColor iColor, QColor oColor,
                                              int imageId) const
{
    auto *d = this;
    const auto dd = d->data.conical;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushConicalGradient;
    p.transform = QTransform::fromTranslate(dd.cx, dd.cy);

    // Note: p.extent and p.radius not used

    // Rotating clockwise, starting from east
    p.transform.rotateRadians(dd.angle + M_PI_2);

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
        p.innerColor.a = float(d->imageY);
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef DECONST

QT_END_NAMESPACE
