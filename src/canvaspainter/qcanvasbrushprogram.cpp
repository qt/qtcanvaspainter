// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasbrushprogram_p.h"
#include <QtCore/qatomic.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

namespace {

struct QCanvasBrushProgramCache
{
    QMutex mutex;
    // One program per distinct shader pair, kept alive for the lifetime of the
    // process. The program id is part of the pipeline cache keys, so recreating
    // a brush with the same shaders must get the same program (and id) to reuse
    // the existing pipelines instead of building and caching new ones.
    QHash<std::pair<QShader, QShader>, std::shared_ptr<const QCanvasBrushProgram>> programs;
    // Shaders loaded from the resource system never change, cache those.
    QHash<QString, QShader> resourceShaders;
};

Q_GLOBAL_STATIC(QCanvasBrushProgramCache, programCache)

} // namespace

std::shared_ptr<const QCanvasBrushProgram> QCanvasBrushProgram::get(const QShader &fragmentShader,
                                                                    const QShader &vertexShader)
{
    if (!fragmentShader.isValid() && !vertexShader.isValid())
        return {};

    QCanvasBrushProgramCache *cache = programCache();
    QMutexLocker locker(&cache->mutex);
    const auto key = std::make_pair(fragmentShader, vertexShader);
    auto it = cache->programs.constFind(key);
    if (it != cache->programs.cend())
        return *it;

    auto program = std::make_shared<const QCanvasBrushProgram>(fragmentShader, vertexShader);
    cache->programs.insert(key, program);
    return program;
}

QShader QCanvasBrushProgram::loadShader(const QString &fileName)
{
    if (fileName.isEmpty())
        return QShader();

    const bool isResource = fileName.startsWith(u':') || fileName.startsWith(u"qrc:");
    QCanvasBrushProgramCache *cache = programCache();
    if (isResource) {
        QMutexLocker locker(&cache->mutex);
        auto it = cache->resourceShaders.constFind(fileName);
        if (it != cache->resourceShaders.cend())
            return *it;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to create custom brush with shader:" << fileName;
        return QShader();
    }
    const QShader shader = QShader::fromSerialized(f.readAll());

    if (isResource) {
        QMutexLocker locker(&cache->mutex);
        cache->resourceShaders.insert(fileName, shader);
    }
    return shader;
}

QCanvasBrushProgram::QCanvasBrushProgram(const QShader &fragmentShader, const QShader &vertexShader)
    : m_fragmentShader(fragmentShader)
    , m_vertexShader(vertexShader)
{
    static QAtomicInteger<quint32> idGenerator = 0;
    m_id = ++idGenerator;
}

QT_END_NAMESPACE
