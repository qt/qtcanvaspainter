// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCPAINTER_H
#define QCPAINTER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtGui/qcolor.h>
#include <QtCore/qrect.h>
#include <QtGui/qtransform.h>
#include <QtGui/qfont.h>

#include <QtCanvasPainter/qcimage.h>
#include <QtCanvasPainter/qcoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCBrush;
class QCImage;
class QCPainterPath;
class QCBoxShadow;
class QRhiTexture;
class QCPainterPrivate;

class Q_CANVASPAINTER_EXPORT QCPainter
{
public:
    QCPainter();
    ~QCPainter();

    enum class PathWinding { CounterClockWise, ClockWise };
    enum class LineCap { Butt, Round, Square };
    enum class LineJoin { Round, Bevel, Miter };
    enum class TextAlign { Left, Right, Center, Start, End };
    enum class TextBaseline { Top, Hanging, Middle, Alphabetic, Bottom };
    enum class TextDirection { LeftToRight, RightToLeft, Inherit, Auto };

    enum class CompositeOperation {
        SourceOver,
        SourceAtop,
        DestinationOut,
    };

    enum class WrapMode {
        NoWrap,
        Wrap,
        WordWrap,
        WrapAnywhere,
    };

    // TODO: NativeTexture is used internally, so remove from this public API?
    enum class ImageFlag {
        GenerateMipmaps = 1 << 0,
        RepeatX = 1 << 1,
        RepeatY = 1 << 2,
        Repeat = RepeatX | RepeatY,
        FlipY = 1 << 3,
        Premultiplied = 1 << 4,
        Nearest = 1 << 5,
        NativeTexture = 1 << 6,
    };
    Q_DECLARE_FLAGS(ImageFlags, ImageFlag)

    enum class RenderHint {
        Antialiasing = 1 << 0,
        HighQualityStroking = 1 << 1,
        DisableWindingEnforce = 1 << 2,
    };
    Q_DECLARE_FLAGS(RenderHints, RenderHint)

    // *** State Handling ***

    void save();
    void restore();
    void reset();

    // *** Render styles ***

    void setStrokeStyle(const QColor &color);
    void setStrokeStyle(const QCBrush &brush);
    void setFillStyle(const QColor &color);
    void setFillStyle(const QCBrush &brush);
    void setMiterLimit(float limit);
    void setLineWidth(float width);
    void setLineCap(LineCap cap);
    void setLineJoin(LineJoin join);
    void setGlobalAlpha(float alpha);
    void setGlobalCompositeOperation(CompositeOperation operation);
    void setGlobalBrightness(float value);
    void setGlobalContrast(float value);
    void setGlobalSaturate(float value);

    // *** Transforms ***

    void resetTransform();
    void setTransform(const QTransform &transform);
    void transform(const QTransform &transform);
    void translate(float x, float y);
    inline void translate(QPointF point);
    void rotate(float angle);
    void skew(float angleX, float angleY = 0.0f);
    void scale(float scale);
    void scale(float scaleX, float scaleY);
    const QTransform getTransform() const;

    void setBrushTransform(const QTransform &transform);

    // *** Clipping ***

    void setClipRect(float x, float y, float width, float height);
    inline void setClipRect(const QRectF &rect);
    void resetClipping();

    //  *** Paths ***

    void beginPath();
    // These should match to path methods of QCPainterPath for consistency.
    void closePath();
    void moveTo(float x, float y);
    inline void moveTo(QPointF point);
    void lineTo(float x, float y);
    inline void lineTo(QPointF point);
    void bezierCurveTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y);
    inline void bezierCurveTo(
        QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    void quadraticCurveTo(float cpX, float cpY, float x, float y);
    inline void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    void arcTo(float x1, float y1, float x2, float y2, float radius);
    inline void arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius);

    void arc(
        float centerX,
        float centerY,
        float radius,
        float a0,
        float a1,
        PathWinding direction = PathWinding::ClockWise,
        bool isConnected = true);
    inline void arc(
        QPointF centerPoint,
        float radius,
        float a0,
        float a1,
        PathWinding direction = PathWinding::ClockWise,
        bool isConnected = true);
    void rect(float x, float y, float width, float height);
    inline void rect(const QRectF &rect);
    void roundRect(float x, float y, float width, float height, float radius);
    inline void roundRect(const QRectF &rect, float radius);
    void roundRect(
        float x,
        float y,
        float width,
        float height,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    inline void roundRect(
        const QRectF &rect,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    void ellipse(float centerX, float centerY, float radiusX, float radiusY);
    inline void ellipse(QPointF centerPoint, float radiusX, float radiusY);
    inline void ellipse(const QRectF &rect);
    void circle(float centerX, float centerY, float radius);
    inline void circle(QPointF centerPoint, float radius);

    // TODO: Consider if we want to have this as functionality
    // doesn't fully cover QPainterPath (like fillRule).
    void addPath(const QPainterPath &path);
    void addPath(const QCPainterPath &path,
                 const QTransform &transform = QTransform());
    void addPath(const QCPainterPath &path,
                 qsizetype start, qsizetype count,
                 const QTransform &transform = QTransform());

    void setPathWinding(PathWinding winding);
    void beginSolidSubPath();
    void beginHoleSubPath();

    void fill();
    void stroke();

    void fill(const QCPainterPath &path, int pathGroup = 0);
    void stroke(const QCPainterPath &path, int pathGroup = 0);

    // *** Direct drawing ***

    void fillRect(float x, float y, float width, float height);
    inline void fillRect(const QRectF &rect);
    void clearRect(float x, float y, float width, float height);
    inline void clearRect(const QRectF &rect);
    void strokeRect(float x, float y, float width, float height);
    inline void strokeRect(const QRectF &rect);

    // *** Shadows ***

    void drawBoxShadow(const QCBoxShadow &shadow);

    // *** Images ***

    void drawImage(const QCImage &image, float x, float y);
    void drawImage(const QCImage &image, float x, float y, float width, float height);
    inline void drawImage(const QCImage &image, const QRectF &destinationRect);
    void drawImage(const QCImage &image, const QRectF &sourceRect, const QRectF &destinationRect);

    // *** Text ***

    void setFont(const QFont &font);
    void setTextAlign(QCPainter::TextAlign align);
    void setTextBaseline(QCPainter::TextBaseline baseline);
    void setTextDirection(QCPainter::TextDirection direction);
    void setTextWrapMode(QCPainter::WrapMode wrapMode);
    void setTextLineHeight(float height);
    void setTextAntialias(float antialias);

    void fillText(const QString &text, float x, float y, float maxWidth = -1);
    inline void fillText(const QString &text, QPointF point, float maxWidth = -1);
    void fillText(const QString &text, const QRectF &rect);

    QRectF textBoundingBox(const QString &text, float x, float y, float maxWidth = -1);
    inline QRectF textBoundingBox(const QString &text, QPointF point, float maxWidth = -1);
    QRectF textBoundingBox(const QString &text, const QRectF &rect);

    // *** Other ***

    void setAntialias(float antialias);
    float devicePixelRatio() const;
    void setRenderHint(RenderHint hint, bool on = true);
    void setRenderHints(RenderHints hints, bool on = true);
    RenderHints renderHints() const;

    // *** Static methods ***

    static float mmToPx(float mm);
    static float ptToPx(float pt);

    // *** Image & path cache handling ***

    QCImage addImage(const QImage &image, QCPainter::ImageFlags flags = {});
    QCImage addImage(QRhiTexture *texture, QCPainter::ImageFlags flags = {});
    QCImage addImage(const QCOffscreenCanvas &canvas, QCPainter::ImageFlags flags = {});
    void removeImage(const QCImage &image);
    void cleanupResources();
    qsizetype activeImageMemoryUsage() const;
    qsizetype activeImageCount() const;
    void removePathGroup(int pathGroup);

    QCOffscreenCanvas createCanvas(QSize pixelSize, int sampleCount = 1, QCOffscreenCanvas::Flags flags = {});
    void destroyCanvas(QCOffscreenCanvas &canvas);
    void grabCanvas(const QCOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

private:
    Q_DISABLE_COPY(QCPainter)
    Q_DECLARE_PRIVATE(QCPainter)
    std::unique_ptr<QCPainterPrivate> d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCPainter::ImageFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCPainter::RenderHints)

inline void QCPainter::translate(QPointF point)
{
    translate(float(point.x()), float(point.y()));
}

inline void QCPainter::setClipRect(const QRectF &rect)
{
    setClipRect(float(rect.x()),
                float(rect.y()),
                float(rect.width()),
                float(rect.height()));
}

inline void QCPainter::moveTo(QPointF point)
{
    moveTo(float(point.x()), float(point.y()));
}

inline void QCPainter::lineTo(QPointF point)
{
    lineTo(float(point.x()), float(point.y()));
}

inline void QCPainter::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(float(controlPoint1.x()),
                  float(controlPoint1.y()),
                  float(controlPoint2.x()),
                  float(controlPoint2.y()),
                  float(endPoint.x()),
                  float(endPoint.y()));
}

inline void QCPainter::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(float(controlPoint.x()),
                     float(controlPoint.y()),
                     float(endPoint.x()),
                     float(endPoint.y()));
}

inline void QCPainter::arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius)
{
    arcTo(float(controlPoint1.x()),
          float(controlPoint1.y()),
          float(controlPoint2.x()),
          float(controlPoint2.y()),
          radius);
}

inline void QCPainter::arc(QPointF centerPoint, float radius, float a0, float a1, PathWinding direction, bool isConnected)
{
    arc(float(centerPoint.x()),
        float(centerPoint.y()),
        radius, a0, a1, direction,
        isConnected);
}

inline void QCPainter::rect(const QRectF &rect)
{
    this->rect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

inline void QCPainter::roundRect(const QRectF &rect, float radius)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radius);
}

inline void QCPainter::roundRect(const QRectF &rect, float radiusTopLeft, float radiusTopRight, float radiusBottomRight, float radiusBottomLeft)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

inline void QCPainter::ellipse(QPointF centerPoint, float radiusX, float radiusY)
{
    ellipse(float(centerPoint.x()),
            float(centerPoint.y()),
            radiusX,
            radiusY);
}

inline void QCPainter::ellipse(const QRectF &rect)
{
    ellipse(float(rect.x() + rect.width() * 0.5),
            float(rect.y() + rect.height() * 0.5),
            float(rect.width() * 0.5),
            float(rect.height() * 0.5));
}

inline void QCPainter::circle(QPointF centerPoint, float radius)
{
    circle(float(centerPoint.x()),
           float(centerPoint.y()), radius);
}

inline void QCPainter::fillRect(const QRectF &rect)
{
    fillRect(float(rect.x()),
             float(rect.y()),
             float(rect.width()),
             float(rect.height()));
}

inline void QCPainter::clearRect(const QRectF &rect)
{
    clearRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()));
}

inline void QCPainter::strokeRect(const QRectF &rect)
{
    strokeRect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

inline void QCPainter::drawImage(const QCImage &image, const QRectF &destinationRect)
{
    drawImage(image,
              float(destinationRect.x()),
              float(destinationRect.y()),
              float(destinationRect.width()),
              float(destinationRect.height()));
}

inline void QCPainter::fillText(const QString &text, QPointF point, float maxWidth)
{
    fillText(text, float(point.x()), float(point.y()), maxWidth);
}

inline QRectF QCPainter::textBoundingBox(const QString &text, QPointF point, float maxWidth)
{
    return textBoundingBox(text, float(point.x()), float(point.y()), maxWidth);
}

QT_END_NAMESPACE

#endif // QCPAINTER_H
