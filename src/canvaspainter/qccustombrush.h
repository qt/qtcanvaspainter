// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCCUSTOMBRUSH_H
#define QCCUSTOMBRUSH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcbrush.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qvectornd.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QCCustomBrushPrivate;
class QCCustomBrush;
struct FragUniforms;

// TODO: Should this have QDataStream support?

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCCustomBrushPrivate)

class Q_CANVASPAINTER_EXPORT QCCustomBrush : public QCBrush
{
public:
    QCCustomBrush();
    QCCustomBrush(const QString &fragmentShader,
                  const QString &vertexShader);
    QCCustomBrush(const QCCustomBrush &brush) noexcept;
    ~QCCustomBrush();

    QCCustomBrush &operator=(const QCCustomBrush &brush) noexcept;
    QCCustomBrush(QCCustomBrush &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCCustomBrush)
    void swap(QCCustomBrush &other) noexcept { d.swap(other.d); }

    bool operator==(const QCCustomBrush &brush) const;
    inline bool operator!=(const QCCustomBrush &brush) const { return !(operator==(brush)); }
    operator QVariant() const;

    void detach();
    BrushType type() const override;

    void setFragmentShader(const QString &fragmentShader);
    void setVertexShader(const QString &vertexShader);

    bool timeRunning() const;
    void setTimeRunning(bool running);

    void setData1(const QVector4D &data);
    void setData2(const QVector4D &data);
    void setData3(const QVector4D &data);
    void setData4(const QVector4D &data);

private:
    friend class QCCustomBrushPrivate;
    QExplicitlySharedDataPointer<QCCustomBrushPrivate> d;
};

Q_DECLARE_SHARED(QCCustomBrush)

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCCustomBrush &);
#endif

QT_END_NAMESPACE

#endif // QCCUSTOMBRUSH_H
