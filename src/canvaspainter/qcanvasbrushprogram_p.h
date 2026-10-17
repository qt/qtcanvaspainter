// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBRUSHPROGRAM_P_H
#define QCANVASBRUSHPROGRAM_P_H

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

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qstring.h>
#include <rhi/qshader.h>
#include <memory>

QT_BEGIN_NAMESPACE

// The shaders of a custom brush. Programs are immutable and shared between
// all the brushes (and their copies) using the same shaders, see get(). The
// renderer identifies the shaders by the program id, instead of comparing
// the shaders.
class Q_CANVASPAINTER_EXPORT QCanvasBrushProgram
{
public:
    // Returns the shared program for the shader pair, creating it when needed.
    // Returns null when both shaders are invalid.
    static std::shared_ptr<const QCanvasBrushProgram> get(const QShader &fragmentShader,
                                                          const QShader &vertexShader);
    // Loads a .qsb file, caching the result of files in the resource system.
    static QShader loadShader(const QString &fileName);

    quint32 id() const { return m_id; }
    const QShader &fragmentShader() const { return m_fragmentShader; }
    const QShader &vertexShader() const { return m_vertexShader; }

    QCanvasBrushProgram(const QShader &fragmentShader, const QShader &vertexShader);

private:
    QShader m_fragmentShader;
    QShader m_vertexShader;
    quint32 m_id = 0;
};

QT_END_NAMESPACE

#endif // QCANVASBRUSHPROGRAM_P_H
