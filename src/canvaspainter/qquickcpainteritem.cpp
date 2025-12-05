// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qquickcpainteritem_p.h"
#include "qquickcpainterrenderer.h"
#include "qquickcpainterrenderer_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QQuickCPainterItem
    \brief The QQuickCPainterItem class provides a way to use the Qt Canvas Painter API in the
    QML Scene Graph.
    \inmodule QtCanvasPainter

    To write your own painted item, you first create a subclass of
    QQuickCPainterItem, and then start by implementing its only pure virtual
    public function: createItemRenderer(), which returns an object that performs
    the actual painting.

    The below code snippet shows the typical structure of a QQuickCPainterItem
    subclass. See QQuickCPainterRenderer for an example of the \c MyRenderer
    class.

    \snippet item-ex-1.cpp 0

    \sa QQuickCPainterRenderer
*/

/*!
    \property QQuickCPainterItem::backendName
    \brief Get name of used QCPainter RHI backend.
*/

/*!
    \property QQuickCPainterItem::fillColor
    \brief The color to use for filling the item ie. the item background.

    The default color is black.
*/

/*!
    \property QQuickCPainterItem::debug
    \internal
*/

/*!
    Constructs a QQuickCPainterItem with the given \a parent item.
*/

QQuickCPainterItem::QQuickCPainterItem(QQuickItem *parent)
    : QQuickRhiItem(*new QQuickCPainterItemPrivate, parent)
{
    Q_D(QQuickCPainterItem);
    setFlag(ItemHasContents, true);
    connect(this, SIGNAL(scaleChanged()), this, SLOT(update()));
    // Default to antialiased
    setAntialiasing(true);

    static bool collectDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_COLLECT");
    if (collectDebug) {
        const int DEBUG_UPDATE_FREQUENCY_MS = 1000;
        d->m_debugUpdateTimer.setInterval(DEBUG_UPDATE_FREQUENCY_MS);
        QObjectPrivate::connect(&d->m_debugUpdateTimer,
                                &QTimer::timeout,
                                d,
                                &QQuickCPainterItemPrivate::updateDebug);
        d->m_debugUpdateTimer.start();
    }
    // Initial debug values
    QCDrawDebug emptyDebug;
    d->updateDebugData(emptyDebug);
}

/*!
    Destroys the QQuickCPainterItem.
*/

QQuickCPainterItem::~QQuickCPainterItem()
{
}

/*!
    \fn QQuickCPainterRenderer* QQuickCPainterItem::createItemRenderer() const

    Implement this method to (re)create a painter for this item. The
    painter class should be inherited from QQuickCPainterRenderer.
    QQuickCPainterItem takes the ownership of the created object and
    deletes it when needed.

    Example code:

    QQuickCPainterRenderer* MyItem::createItemRenderer() const
    {
        return new MyItemPainter();
    }

*/

/*!
    \fn QString QQuickCPainterItem::backendName() const

    Returns used RHI rendering backend name. See \l QRhi::backendName()

    To affect which backend is used, configure the RHI
    backend of the Qt application e.g. with QSG_RHI_BACKEND.
*/
QString QQuickCPainterItem::backendName() const
{
    Q_D(const QQuickCPainterItem);
    return d->m_backendName;
}

/*!
    \fn QColor QQuickCPainterItem::fillColor() const

    Returns the current fill color.

    \sa setFillColor()
*/

QColor QQuickCPainterItem::fillColor() const
{
    Q_D(const QQuickCPainterItem);
    return d->m_fillColor;
}

/*!
    \fn void QQuickCPainterItem::setFillColor (const QColor &color)

    Set the fill color to \a color. This color will be used to draw
    the background of the item. The default color is black.

    \note When setting the fill color to not fully opaque (alpha channel
    less than 255), remember to set also \l {QQuickRhiItem::}{alphaBlending}
    to \c true.

    \sa fillColor(), QQuickRhiItem::alphaBlending
*/

void QQuickCPainterItem::setFillColor(const QColor &color)
{
    Q_D(QQuickCPainterItem);
    if (d->m_fillColor == color)
        return;
    d->m_fillColor = color;

    Q_EMIT fillColorChanged();
    update();
}

/*!
   \internal
 */

QVariantMap QQuickCPainterItem::debug()
{
    Q_D(QQuickCPainterItem);
    if (d->m_debug.isEmpty()) {
        // Initialize debug data
        d->m_debug.insert(QLatin1String("drawCallCount"), 0);
        d->m_debug.insert(QLatin1String("strokeTriCount"), 0);
        d->m_debug.insert(QLatin1String("fillTriCount"), 0);
        d->m_debug.insert(QLatin1String("textTriCount"), 0);
    }
    return d->m_debug;
}

/*!
   \internal
*/

void QQuickCPainterItemPrivate::updateDebug()
{
    Q_Q(QQuickCPainterItem);
    if (m_debugDataChanged) {
        Q_EMIT q->debugChanged();
        m_debugDataChanged = false;
        q->update();
    }
}

/*!
   \internal
*/

void QQuickCPainterItemPrivate::updateDebugData(QCDrawDebug drawDebug)
{
    m_debug.insert(QStringLiteral(u"fillDrawCallCount"), drawDebug.fillDrawCallCount);
    m_debug.insert(QStringLiteral(u"strokeDrawCallCount"), drawDebug.strokeDrawCallCount);
    m_debug.insert(QStringLiteral(u"textDrawCallCount"), drawDebug.textDrawCallCount);
    m_debug.insert(QStringLiteral(u"fillTriangleCount"), drawDebug.fillTriangleCount);
    m_debug.insert(QStringLiteral(u"strokeTriangleCount"), drawDebug.strokeTriangleCount);
    m_debug.insert(QStringLiteral(u"textTriangleCount"), drawDebug.textTriangleCount);
    m_debug.insert(QStringLiteral(u"drawCallCount"), drawDebug.drawCallCount);
    m_debug.insert(QStringLiteral(u"triangleCount"), drawDebug.triangleCount);
    m_debugDataChanged = true;
}

/*!
   \internal
*/

QQuickRhiItemRenderer *QQuickCPainterItem::createRenderer()
{
    Q_D(QQuickCPainterItem);
    d->m_renderer = createItemRenderer();
    return d->m_renderer;
}

/*!
   \internal
*/

void QQuickCPainterItemPrivate::setBackendName(const QString &name)
{
    Q_Q(QQuickCPainterItem);
    if (m_backendName != name) {
        m_backendName = name;
        Q_EMIT q->backendNameChanged();
    }
}

QT_END_NAMESPACE
