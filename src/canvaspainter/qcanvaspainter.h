// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTER_H
#define QCANVASPAINTER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtGui/qcolor.h>
#include <QtCore/qrect.h>
#include <QtGui/qtransform.h>
#include <QtGui/qfont.h>

#include <QtCanvasPainter/qcanvasimage.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCanvasBrush;
class QCanvasImage;
class QCanvasPath;
class QCanvasBoxShadow;
class QRhiTexture;
class QCanvasPainterPrivate;
class QVectorPath;

class QCanvasPainter
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasPainter();
    Q_CANVASPAINTER_EXPORT ~QCanvasPainter();

    enum class PathWinding : quint8 { CounterClockWise, ClockWise };
    enum class PathConnection : quint8 { NotConnected, Connected };
    enum class LineCap : quint8 { Butt, Round, Square };
    enum class LineJoin : quint8 { Round, Bevel, Miter };
    enum class TextAlign : quint8 { Left, Right, Center, Start, End };
    enum class TextBaseline : quint8 { Top, Hanging, Middle, Alphabetic, Bottom };
    enum class TextDirection : quint8 { LeftToRight, RightToLeft, Inherit, Auto };
    enum class FillRule : quint8 { NonZero, EvenOdd };

    enum class CompositeOperation : quint8 {
        SourceOver,
        SourceAtop,
        DestinationOut,
    };

    enum class WrapMode : quint8 {
        NoWrap,
        Wrap,
        WordWrap,
        WrapAnywhere,
    };

    enum class ImageFlag {
        GenerateMipmaps = 1 << 0,
        RepeatX = 1 << 1,
        RepeatY = 1 << 2,
        Repeat = RepeatX | RepeatY,
        FlipY = 1 << 3,
        Premultiplied = 1 << 4,
        Nearest = 1 << 5,
    };
    Q_DECLARE_FLAGS(ImageFlags, ImageFlag)

    // *** State Handling ***

    Q_CANVASPAINTER_EXPORT void save();
    Q_CANVASPAINTER_EXPORT void restore();
    Q_CANVASPAINTER_EXPORT void reset();

    // *** Render styles ***

    Q_CANVASPAINTER_EXPORT void setStrokeStyle(QColor color);
    Q_CANVASPAINTER_EXPORT void setStrokeStyle(const QCanvasBrush &brush);
    Q_CANVASPAINTER_EXPORT void setFillStyle(QColor color);
    Q_CANVASPAINTER_EXPORT void setFillStyle(const QCanvasBrush &brush);
    Q_CANVASPAINTER_EXPORT void setMiterLimit(qreal limit);
    Q_CANVASPAINTER_EXPORT void setLineWidth(qreal width);
    Q_CANVASPAINTER_EXPORT void setLineCap(LineCap cap);
    Q_CANVASPAINTER_EXPORT void setLineJoin(LineJoin join);
    Q_CANVASPAINTER_EXPORT void setGlobalAlpha(qreal alpha);
    Q_CANVASPAINTER_EXPORT void setGlobalCompositeOperation(CompositeOperation operation);
    Q_CANVASPAINTER_EXPORT void setGlobalBrightness(qreal value);
    Q_CANVASPAINTER_EXPORT void setGlobalContrast(qreal value);
    Q_CANVASPAINTER_EXPORT void setGlobalSaturation(qreal value);
    Q_CANVASPAINTER_EXPORT void setFillRule(FillRule fillRule);

    // *** Transforms ***

    Q_CANVASPAINTER_EXPORT void resetTransform();
    Q_CANVASPAINTER_EXPORT void setTransform(const QTransform &transform);
    Q_CANVASPAINTER_EXPORT void transform(const QTransform &transform);
    Q_CANVASPAINTER_EXPORT void translate(qreal x, qreal y);
    inline void translate(QPointF point);
    Q_CANVASPAINTER_EXPORT void rotate(qreal angle);
    Q_CANVASPAINTER_EXPORT void skew(qreal angleX, qreal angleY = 0.0);
    Q_CANVASPAINTER_EXPORT void scale(qreal scale);
    Q_CANVASPAINTER_EXPORT void scale(qreal scaleX, qreal scaleY);
    Q_CANVASPAINTER_EXPORT QTransform getTransform() const;

    // *** Clipping ***

    Q_CANVASPAINTER_EXPORT void setClipRect(qreal x, qreal y, qreal width, qreal height);
    inline void setClipRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void resetClipping();

    //  *** Paths ***

    Q_CANVASPAINTER_EXPORT void beginPath();
    // These should match to path methods of QCanvasPath for consistency.
    Q_CANVASPAINTER_EXPORT void closePath();
    Q_CANVASPAINTER_EXPORT void moveTo(qreal x, qreal y);
    inline void moveTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void lineTo(qreal x, qreal y);
    inline void lineTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void bezierCurveTo(qreal cp1X, qreal cp1Y, qreal cp2X, qreal cp2Y, qreal x, qreal y);
    inline void bezierCurveTo(
        QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void quadraticCurveTo(qreal cpX, qreal cpY, qreal x, qreal y);
    inline void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal radius);
    inline void arcTo(QPointF controlPoint1, QPointF controlPoint2, qreal radius);

    Q_CANVASPAINTER_EXPORT void arc(
        qreal centerX,
        qreal centerY,
        qreal radius,
        qreal a0,
        qreal a1,
        PathWinding direction = PathWinding::ClockWise,
        PathConnection connection = PathConnection::Connected);
    inline void arc(
        QPointF centerPoint,
        qreal radius,
        qreal a0,
        qreal a1,
        PathWinding direction = PathWinding::ClockWise,
        PathConnection connection = PathConnection::Connected);
    Q_CANVASPAINTER_EXPORT void rect(qreal x, qreal y, qreal width, qreal height);
    inline void rect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void roundRect(qreal x, qreal y, qreal width, qreal height, qreal radius);
    inline void roundRect(const QRectF &rect, qreal radius);
    Q_CANVASPAINTER_EXPORT void roundRect(
        qreal x,
        qreal y,
        qreal width,
        qreal height,
        qreal radiusTopLeft,
        qreal radiusTopRight,
        qreal radiusBottomRight,
        qreal radiusBottomLeft);
    inline void roundRect(
        const QRectF &rect,
        qreal radiusTopLeft,
        qreal radiusTopRight,
        qreal radiusBottomRight,
        qreal radiusBottomLeft);
    Q_CANVASPAINTER_EXPORT void ellipse(qreal centerX, qreal centerY, qreal radiusX, qreal radiusY);
    inline void ellipse(QPointF centerPoint, qreal radiusX, qreal radiusY);
    inline void ellipse(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void circle(qreal centerX, qreal centerY, qreal radius);
    inline void circle(QPointF centerPoint, qreal radius);

    Q_CANVASPAINTER_EXPORT void addPath(const QPainterPath &path);
    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path,
                                        const QTransform &transform = QTransform());
    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path,
                                        qsizetype start, qsizetype count,
                                        const QTransform &transform = QTransform());

    Q_CANVASPAINTER_EXPORT void setPathWinding(PathWinding winding);
    Q_CANVASPAINTER_EXPORT void beginSolidSubPath();
    Q_CANVASPAINTER_EXPORT void beginHoleSubPath();

    Q_CANVASPAINTER_EXPORT void fill();
    Q_CANVASPAINTER_EXPORT void fill(FillRule fillRule);
    Q_CANVASPAINTER_EXPORT void stroke();

    Q_CANVASPAINTER_EXPORT void fill(const QCanvasPath &path, int pathGroup = -1);
    Q_CANVASPAINTER_EXPORT void fill(const QCanvasPath &path, FillRule fillRule, int pathGroup = -1);
    Q_CANVASPAINTER_EXPORT void stroke(const QCanvasPath &path, int pathGroup = -1);

    // *** Direct drawing ***

    Q_CANVASPAINTER_EXPORT void fillRect(qreal x, qreal y, qreal width, qreal height);
    inline void fillRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void clearRect(qreal x, qreal y, qreal width, qreal height);
    inline void clearRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void strokeRect(qreal x, qreal y, qreal width, qreal height);
    inline void strokeRect(const QRectF &rect);

    // *** Shadows ***

    Q_CANVASPAINTER_EXPORT void drawBoxShadow(const QCanvasBoxShadow &shadow);

    // *** Images ***

    Q_CANVASPAINTER_EXPORT void drawImage(const QCanvasImage &image, qreal x, qreal y);
    Q_CANVASPAINTER_EXPORT void drawImage(const QCanvasImage &image, qreal x, qreal y, qreal width, qreal height);
    inline void drawImage(const QCanvasImage &image, const QRectF &destinationRect);
    Q_CANVASPAINTER_EXPORT void drawImage(const QCanvasImage &image, const QRectF &sourceRect, const QRectF &destinationRect);

    // *** Text ***

    Q_CANVASPAINTER_EXPORT void setFont(const QFont &font);
    Q_CANVASPAINTER_EXPORT void setTextAlign(QCanvasPainter::TextAlign align);
    Q_CANVASPAINTER_EXPORT void setTextBaseline(QCanvasPainter::TextBaseline baseline);
    Q_CANVASPAINTER_EXPORT void setTextDirection(QCanvasPainter::TextDirection direction);
    Q_CANVASPAINTER_EXPORT void setTextWrapMode(QCanvasPainter::WrapMode wrapMode);
    Q_CANVASPAINTER_EXPORT void setTextLineHeight(qreal height);
    Q_CANVASPAINTER_EXPORT void setTextAntialias(qreal antialias);

    Q_CANVASPAINTER_EXPORT void fillText(const QString &text, qreal x, qreal y, qreal maxWidth = 0);
    inline void fillText(const QString &text, QPointF point, qreal maxWidth = 0);
    Q_CANVASPAINTER_EXPORT void fillText(const QString &text, const QRectF &rect);

    Q_CANVASPAINTER_EXPORT QRectF textBoundingBox(const QString &text, qreal x, qreal y, qreal maxWidth = 0) const;
    inline QRectF textBoundingBox(const QString &text, QPointF point, qreal maxWidth = 0) const;
    Q_CANVASPAINTER_EXPORT QRectF textBoundingBox(const QString &text, const QRectF &rect) const;

    // *** Other ***

    Q_CANVASPAINTER_EXPORT void setAntialias(qreal antialias);
    Q_CANVASPAINTER_EXPORT void setWindingEnforce(bool enabled);
    Q_CANVASPAINTER_EXPORT void setHighQualityStroking(bool enabled);

    // *** Image & path cache handling ***

    Q_CANVASPAINTER_EXPORT QCanvasImage addImage(const QImage &image, QCanvasPainter::ImageFlags flags = {});
    Q_CANVASPAINTER_EXPORT QCanvasImage addImage(QRhiTexture *texture, QCanvasPainter::ImageFlags flags = {});
    Q_CANVASPAINTER_EXPORT QCanvasImage addImage(const QCanvasOffscreenCanvas &canvas, QCanvasPainter::ImageFlags flags = {});
    Q_CANVASPAINTER_EXPORT void removeImage(const QCanvasImage &image);
    Q_CANVASPAINTER_EXPORT void cleanupResources();
    Q_CANVASPAINTER_EXPORT qsizetype activeImageMemoryUsage() const;
    Q_CANVASPAINTER_EXPORT qsizetype activeImageCount() const;
    Q_CANVASPAINTER_EXPORT void removePathGroup(int pathGroup);

    Q_CANVASPAINTER_EXPORT QCanvasOffscreenCanvas createCanvas(QSize pixelSize, int sampleCount = 1, QCanvasOffscreenCanvas::Flags flags = {});
    Q_CANVASPAINTER_EXPORT void destroyCanvas(QCanvasOffscreenCanvas &canvas);

private:
    Q_DISABLE_COPY_MOVE(QCanvasPainter)
    Q_DECLARE_PRIVATE(QCanvasPainter)
    std::unique_ptr<QCanvasPainterPrivate> d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasPainter::ImageFlags)

void QCanvasPainter::translate(QPointF point)
{
    translate(point.x(), point.y());
}

void QCanvasPainter::setClipRect(const QRectF &rect)
{
    setClipRect(rect.x(),
                rect.y(),
                rect.width(),
                rect.height());
}

void QCanvasPainter::moveTo(QPointF point)
{
    moveTo(point.x(), point.y());
}

void QCanvasPainter::lineTo(QPointF point)
{
    lineTo(point.x(), point.y());
}

void QCanvasPainter::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(controlPoint1.x(),
                  controlPoint1.y(),
                  controlPoint2.x(),
                  controlPoint2.y(),
                  endPoint.x(),
                  endPoint.y());
}

void QCanvasPainter::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(controlPoint.x(),
                     controlPoint.y(),
                     endPoint.x(),
                     endPoint.y());
}

void QCanvasPainter::arcTo(QPointF controlPoint1, QPointF controlPoint2, qreal radius)
{
    arcTo(controlPoint1.x(),
          controlPoint1.y(),
          controlPoint2.x(),
          controlPoint2.y(),
          radius);
}

void QCanvasPainter::arc(QPointF centerPoint, qreal radius, qreal a0, qreal a1, PathWinding direction, PathConnection connection)
{
    arc(centerPoint.x(),
        centerPoint.y(),
        radius, a0, a1, direction,
        connection);
}

void QCanvasPainter::rect(const QRectF &rect)
{
    this->rect(rect.x(),
               rect.y(),
               rect.width(),
               rect.height());
}

void QCanvasPainter::roundRect(const QRectF &rect, qreal radius)
{
    roundRect(rect.x(),
              rect.y(),
              rect.width(),
              rect.height(),
              radius);
}

void QCanvasPainter::roundRect(const QRectF &rect, qreal radiusTopLeft, qreal radiusTopRight, qreal radiusBottomRight, qreal radiusBottomLeft)
{
    roundRect(rect.x(),
              rect.y(),
              rect.width(),
              rect.height(),
              radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

void QCanvasPainter::ellipse(QPointF centerPoint, qreal radiusX, qreal radiusY)
{
    ellipse(centerPoint.x(),
            centerPoint.y(),
            radiusX,
            radiusY);
}

void QCanvasPainter::ellipse(const QRectF &rect)
{
    ellipse(rect.x() + rect.width() * 0.5,
            rect.y() + rect.height() * 0.5,
            rect.width() * 0.5,
            rect.height() * 0.5);
}

void QCanvasPainter::circle(QPointF centerPoint, qreal radius)
{
    circle(centerPoint.x(),
           centerPoint.y(), radius);
}

void QCanvasPainter::fillRect(const QRectF &rect)
{
    fillRect(rect.x(),
             rect.y(),
             rect.width(),
             rect.height());
}

void QCanvasPainter::clearRect(const QRectF &rect)
{
    clearRect(rect.x(),
              rect.y(),
              rect.width(),
              rect.height());
}

void QCanvasPainter::strokeRect(const QRectF &rect)
{
    strokeRect(rect.x(),
               rect.y(),
               rect.width(),
               rect.height());
}

void QCanvasPainter::drawImage(const QCanvasImage &image, const QRectF &destinationRect)
{
    drawImage(image,
              destinationRect.x(),
              destinationRect.y(),
              destinationRect.width(),
              destinationRect.height());
}

void QCanvasPainter::fillText(const QString &text, QPointF point, qreal maxWidth)
{
    fillText(text, point.x(), point.y(), maxWidth);
}

QRectF QCanvasPainter::textBoundingBox(const QString &text, QPointF point, qreal maxWidth) const
{
    return textBoundingBox(text, point.x(), point.y(), maxWidth);
}

QT_END_NAMESPACE

#endif // QCANVASPAINTER_H
