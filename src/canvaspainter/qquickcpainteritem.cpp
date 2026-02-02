// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquickcpainteritem_p.h"
#include "qquickcpainterrenderer.h"
#include "qquickcpainterrenderer_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QQuickCPainterItem
    \since 6.11
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
    \property QQuickCPainterItem::fillColor
    \brief The color to use for filling the item ie. the item background.

    The default color is black.
*/

/*!
    \property QQuickCPainterItem::debug

    Contains a key - value map of rendering statistics. The data is only
    collected when the environment variable \c{QCPAINTER_DEBUG_COLLECT} is set
    to a non-zero value.

    The data is updated periodically, not every time the item repaints. The
    interval is currently one second. Therefore, the data received will in many
    cases refer to a previous drawing of the item. This is not usually an issue
    when displaying the statistics interactively, but it is important to be
    aware of.

    To automatically show the data within the item in form of an overlay, set
    the environment variable \c{QCPAINTER_DEBUG_RENDER} instead. This provides a
    convenient shortcut when the intention is anyway to show the values
    on-screen.

    The list of keys is currently the following:
    \list
    \li fillDrawCallCount
    \li strokeDrawCallCount
    \li textDrawCallCount
    \li fillTriangleCount
    \li strokeTriangleCount
    \li textTriangleCount
    \li drawCallCount
    \li triangleCount
    \endlist
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
    QCDebugCounters zeroCounters;
    d->updateDebugData(zeroCounters);
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
    if (d->m_debugData.isEmpty()) {
        QCDebugCounters zeroCounters;
        d->updateDebugData(zeroCounters);
    }
    return d->m_debugData;
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

void QQuickCPainterItemPrivate::updateDebugData(const QCDebugCounters &debugCounters)
{
    QCPainterDebugCounterUtils::fillDebugCounters(&m_debugData, debugCounters);
    m_debugDataChanged = true;
}

/*!
   \reimp
*/
QQuickRhiItemRenderer *QQuickCPainterItem::createRenderer()
{
    Q_D(QQuickCPainterItem);
    d->m_renderer = createItemRenderer();
    return d->m_renderer;
}

QT_END_NAMESPACE
