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

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasCustomBrushPrivate)

class QCanvasCustomBrush
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasCustomBrush();
    Q_CANVASPAINTER_EXPORT QCanvasCustomBrush(const QString &fragmentShader,
                                              const QString &vertexShader = {});
    Q_CANVASPAINTER_EXPORT QCanvasCustomBrush(const QCanvasCustomBrush &);
    Q_CANVASPAINTER_EXPORT QCanvasCustomBrush &operator=(const QCanvasCustomBrush &);
    QCanvasCustomBrush(QCanvasCustomBrush &&) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasCustomBrush)
    Q_CANVASPAINTER_EXPORT ~QCanvasCustomBrush();

    Q_CANVASPAINTER_EXPORT operator QCanvasBrush() const;
    Q_CANVASPAINTER_EXPORT operator QVariant() const;

    Q_CANVASPAINTER_EXPORT void setFragmentShader(const QString &fragmentShader);
    Q_CANVASPAINTER_EXPORT void setFragmentShader(const QShader &fragmentShader);
    Q_CANVASPAINTER_EXPORT void setVertexShader(const QString &vertexShader);
    Q_CANVASPAINTER_EXPORT void setVertexShader(const QShader &vertexShader);

    Q_CANVASPAINTER_EXPORT bool timeRunning() const;
    Q_CANVASPAINTER_EXPORT void setTimeRunning(bool running);

    Q_CANVASPAINTER_EXPORT void setData1(const QVector4D &data);
    Q_CANVASPAINTER_EXPORT void setData2(const QVector4D &data);
    Q_CANVASPAINTER_EXPORT void setData3(const QVector4D &data);
    Q_CANVASPAINTER_EXPORT void setData4(const QVector4D &data);
    void swap(QCanvasCustomBrush &other) noexcept { d.swap(other.d); }

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasCustomBrush)

    void detach();
    explicit QCanvasCustomBrush(QCanvasCustomBrushPrivate *p);
    friend class QCanvasCustomBrushPrivate;
    QExplicitlySharedDataPointer<QCanvasCustomBrushPrivate> d;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasCustomBrush &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasCustomBrush &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasCustomBrush &);
#endif
};

template<> Q_CANVASPAINTER_EXPORT QCanvasCustomBrush QCanvasBrush::as<QCanvasCustomBrush>() const;

QT_END_NAMESPACE

#endif // QCANVASCUSTOMBRUSH_H
