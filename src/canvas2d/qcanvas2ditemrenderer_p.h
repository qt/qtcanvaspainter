// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANVAS2DITEMRENDERER_H
#define QCANVAS2DITEMRENDERER_H

#include "qtcanvas2dglobal_p.h"
#include "qcanvas2dcontext_p.h"
#include "qcanvas2ditem_p.h"
#include <QtCanvasPainter/qcanvaspainteritemrenderer.h>
#include <QtGui/qcolor.h>
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class Q_CANVAS2D_EXPORT QCanvas2DItemRenderer : public QObject, public QCanvasPainterItemRenderer
{
    Q_OBJECT
public:
    explicit QCanvas2DItemRenderer();
    ~QCanvas2DItemRenderer();

protected:
    void synchronizeData(QCanvasPainterItem *item) override;
    void paint(QCanvasPainter *painter) override;

Q_SIGNALS:
    void painted();

private:
    void setPaintStyle(const QCanvasBrush &brush, bool fill);
    QCanvasImage getCachedImage(const QImage &image, const QString &url, QCanvasPainter::ImageFlags flags);
    void drawImage(const QImage &image, const QString &url, const QRectF &sr, const QRectF &dr);
    void copyBrushes(const QList<QCanvasBrush> &newBrushes);

    QCanvasPainter *m_painter = nullptr;
    QCanvas2DContext::State m_state;

    inline bool hasNext() const {return cmdIdx < commands.size(); }
    inline QCanvas2DContext::PaintCommand takeNextCommand() { return commands.at(cmdIdx++); }

    inline QCanvasPainter::LineCap takeLineCap() { return static_cast<QCanvasPainter::LineCap>(takeInt());}
    inline QCanvasPainter::LineJoin takeLineJoin() { return static_cast<QCanvasPainter::LineJoin>(takeInt());}
    inline QCanvasPainter::TextAlign takeAlign() { return static_cast<QCanvasPainter::TextAlign>(takeInt());}
    inline QCanvasPainter::TextBaseline takeBaseline() { return static_cast<QCanvasPainter::TextBaseline>(takeInt());}
    inline QCanvasPainter::WrapMode takeWrapMode() { return static_cast<QCanvasPainter::WrapMode>(takeInt());}
    inline QCanvasPainter::TextDirection takeDirection() { return static_cast<QCanvasPainter::TextDirection>(takeInt());}

    inline QTransform takeMatrix() { return matrixes.at(matrixIdx++); }

    inline QRectF takeRect() { return rects.at(rectIdx++); }

    inline QPainterPath takePath() { return paths.at(pathIdx++); }
    inline const QCanvasPath& takeCanvasPath() { return canvasPaths.at(canvasPathIdx++); }

    inline const QImage& takeImage() { return images.at(imageIdx++); }
    inline QQmlRefPointer<QCanvas2DPixmap> takePixmap() { return pixmaps.at(pixmapIdx++); }

    inline int takeInt() { return ints.at(intIdx++); }
    inline bool takeBool() {return bools.at(boolIdx++); }
    inline qreal takeReal() { return reals.at(realIdx++); }
    inline QColor takeColor() { return colors.at(colorIdx++); }
    inline const QCanvasBrush &takeBrush() { return brushes.at(brushIdx++); }
    inline QString takeString() { return strings.at(stringIdx++); }
    inline const QFont& takeFont() { return fonts.at(fontIdx++); }

    // Note: Matching with the buffer
    int cmdIdx = 0;
    int intIdx = 0;
    int boolIdx = 0;
    int realIdx = 0;
    int rectIdx = 0;
    int colorIdx = 0;
    int matrixIdx = 0;
    int brushIdx = 0;
    int pathIdx = 0;
    int canvasPathIdx = 0;
    int imageIdx = 0;
    int fontIdx = 0;
    int pixmapIdx = 0;
    int stringIdx = 0;
    void reset();

    QHash<QString, QCanvasImage> qcImages;
    QHash<uint, QCanvas2DItem::ImageData> imageData;

    QList<QCanvas2DContext::PaintCommand> commands;
    QList<int> ints;
    QList<bool> bools;
    QList<qreal> reals;
    QList<QRectF> rects;
    QList<QColor> colors;
    QList<QTransform> matrixes;
    QList<QCanvasBrush> brushes;
    QList<QPainterPath> paths;
    QList<QCanvasPath> canvasPaths;
    QList<QImage> images;
    QList<QFont> fonts;
    QList<QQmlRefPointer<QCanvas2DPixmap> > pixmaps;
    QList<QString> strings;
};

QT_END_NAMESPACE

#endif // QCANVAS2DITEMRENDERER_H
