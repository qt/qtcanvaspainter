// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASCUSTOMBRUSH_H
#define QCANVASCUSTOMBRUSH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qvectornd.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QCanvasCustomBrushPrivate;
class QCanvasCustomBrush;
class QShader;

// TODO: Should this have QDataStream support?

class Q_CANVASPAINTER_EXPORT QCanvasCustomBrush : public QCanvasBrush
{
public:
    QCanvasCustomBrush();
    QCanvasCustomBrush(const QString &fragmentShader,
                  const QString &vertexShader = {});
    ~QCanvasCustomBrush();

    operator QVariant() const;

    void setFragmentShader(const QString &fragmentShader);
    void setFragmentShader(const QShader &fragmentShader);
    void setVertexShader(const QString &vertexShader);
    void setVertexShader(const QShader &vertexShader);

    bool timeRunning() const;
    void setTimeRunning(bool running);

    void setData1(const QVector4D &data);
    void setData2(const QVector4D &data);
    void setData3(const QVector4D &data);
    void setData4(const QVector4D &data);

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasCustomBrush)

    friend class QCanvasCustomBrushPrivate;
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasCustomBrush &);
#endif
};

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasCustomBrush &);
#endif

QT_END_NAMESPACE

#endif // QCANVASCUSTOMBRUSH_H
