// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCanvasPainter/private/qcanvaspainteritem_p.h>
#include "qcanvas2dcontext_p.h"
#include "qcanvas2ditem_p.h"
#include "qcanvas2ditemrenderer_p.h"

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qsgadaptationlayer_p.h>
#include <QtQuick/qsgtextureprovider.h>
#include <QtQuick/private/qquickpixmap_p.h>

#include <QtCore/QBuffer>
#include <QtCore/qdatetime.h>
#include <QtCore/qcoreapplication.h>

#include <QtQml/qqmlinfo.h>
#include <QtQml/private/qqmlengine_p.h>
#include <QtQml/private/qv4value_p.h>
#include <QtQml/private/qv4functionobject_p.h>
#include <QtQml/private/qv4scopedvalue_p.h>
#include <QtQml/private/qv4jscall_p.h>
#include <QtQml/private/qv4qobjectwrapper_p.h>
#include <QtQml/private/qjsvalue_p.h>

QT_BEGIN_NAMESPACE

QCanvas2DPixmap::QCanvas2DPixmap(const QImage& image)
    : m_pixmap(nullptr)
    , m_image(image)
{

}

QCanvas2DPixmap::QCanvas2DPixmap(QQuickPixmap *pixmap)
    : m_pixmap(pixmap)
{

}

QCanvas2DPixmap::~QCanvas2DPixmap()
{
    delete m_pixmap;
}

qreal QCanvas2DPixmap::width() const
{
    if (m_pixmap)
        return m_pixmap->width();

    return m_image.width();
}

qreal QCanvas2DPixmap::height() const
{
    if (m_pixmap)
        return m_pixmap->height();

    return m_image.height();
}

bool QCanvas2DPixmap::isValid() const
{
    if (m_pixmap)
        return m_pixmap->isReady();
    return !m_image.isNull();
}

QImage QCanvas2DPixmap::image()
{
    if (m_image.isNull() && m_pixmap)
        m_image = m_pixmap->image();

    return m_image;
}

class QCanvas2DItemPrivate : public QCanvasPainterItemPrivate
{
public:
    QCanvas2DItemPrivate();
    ~QCanvas2DItemPrivate();
    QCanvas2DContext *context = nullptr;
    QString contextType;
    QHash<QUrl, QQmlRefPointer<QCanvas2DPixmap> > pixmaps;
    QUrl baseUrl;
    QMap<int, QV4::PersistentValue> animationCallbacks;
    QCanvas2DCommandBuffer *ccb = nullptr;
    bool available = false;
};

QCanvas2DItemPrivate::QCanvas2DItemPrivate()
    : QCanvasPainterItemPrivate()
{
    implicitAntialiasing = true;
}

QCanvas2DItemPrivate::~QCanvas2DItemPrivate()
{
    pixmaps.clear();
}


/*!
    \qmltype Canvas2D
    \nativetype QCanvas2DItem
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides a 2D canvas item enabling drawing via JavaScript.

    The Canvas2D item allows drawing of straight and curved lines, simple and
    complex shapes, graphs, and referenced graphic images. It can also add
    text, colors, gradients, and patterns, and do low level pixel operations.

    Rendering to the Canvas is done using a Canvas2DContext object, usually as a
    result of the \l paint signal.

    To define a drawing area in the Canvas2D item set the \c width and \c height
    properties.  For example, the following code creates a Canvas2D item which
    has a drawing area with a height of 100 pixels and width of 200 pixels:
    \qml
    import QtCanvas2D

    Canvas2D {
        id: mycanvas
        width: 100
        height: 200
        onPaint: {
            var ctx = getContext("2d");
            ctx.fillStyle = Qt.rgba(1, 0, 0, 1);
            ctx.fillRect(0, 0, width, height);
        }
    }
    \endqml

    \section1 Tips for Porting Existing HTML5 Canvas Applications

    Although the Canvas item provides an HTML5-like API, HTML5 canvas
    applications need to be modified to run in the Canvas item:
    \list
    \li Replace all DOM API calls with QML property bindings or Canvas item methods.
    \li Replace all HTML event handlers with the MouseArea item.
    \li Change setInterval/setTimeout function calls with the \l FrameAnimation item or
       the use of requestAnimationFrame().
    \li Place painting code into the \c onPaint handler and trigger
       painting by calling the markDirty() or requestPaint() methods.
    \li To draw images, load them by calling the Canvas's loadImage() method and then request to paint
       them in the \c onImageLoaded handler.
    \endlist

    \sa Canvas2DContext, QCanvasPainterItem, QCanvasPainter
*/

QCanvas2DItem::QCanvas2DItem(QQuickItem *parent)
    : QCanvasPainterItem(*(new QCanvas2DItemPrivate), parent)
{
    // TODO: Should we default to performance like QCanvasPainterItem
    // and remove these, or compatibity with HTLM/Quick Canvas and
    // enable these to be the default?
    //setAlphaBlending(true);
    //setFillColor(Qt::transparent);
}

QCanvas2DItem::~QCanvas2DItem()
{
    Q_D(QCanvas2DItem);
    delete d->context;
}

/*!
    \qmlproperty bool Canvas2D::available

    Indicates when Canvas is able to provide a drawing context to operate on.
*/

bool QCanvas2DItem::isAvailable() const
{
    return d_func()->available;
}

/*!
    \qmlproperty string Canvas2D::contextType
    The type of drawing context to use.

    This property is set to the name of the active context type.

    If set explicitly the canvas will attempt to create a context of the
    named type after becoming available.

    The type name is the same as used in the getContext() call, for the 2d
    canvas the value will be "2d".

    \sa getContext(), available
*/

QString QCanvas2DItem::contextType() const
{
    return d_func()->contextType;
}

void QCanvas2DItem::setContextType(const QString &contextType)
{
    Q_D(QCanvas2DItem);

    if (contextType.compare(d->contextType, Qt::CaseInsensitive) == 0)
        return;

    if (d->context) {
        qmlWarning(this) << "Canvas already initialized with a different context type";
        return;
    }

    d->contextType = contextType;

    if (d->available)
        createContext(contextType);

    emit contextTypeChanged();
}

/*!
    \qmlproperty object Canvas2D::context
    Holds the active drawing context.

    If the canvas is ready and there has been a successful call to getContext()
    or the contextType property has been set with a supported context type,
    this property will contain the current drawing context, otherwise null.
*/

QJSValue QCanvas2DItem::context() const
{
    Q_D(const QCanvas2DItem);
    return d->context ? QJSValuePrivate::fromReturnedValue(d->context->v4value()) : QJSValue();
}

bool QCanvas2DItem::isPaintConnected()
{
    IS_SIGNAL_CONNECTED(this, QCanvas2DItem, paint, ());
}

void QCanvas2DItem::sceneGraphInitialized()
{
    Q_D(QCanvas2DItem);

    d->available = true;
    connect(this, SIGNAL(visibleChanged()), SLOT(checkAnimationCallbacks()));
    QMetaObject::invokeMethod(this, "availableChanged", Qt::QueuedConnection);

    if (!d->contextType.isNull())
        QMetaObject::invokeMethod(this, "delayedCreate", Qt::QueuedConnection);
    else if (isPaintConnected())
        QMetaObject::invokeMethod(this, "requestPaint", Qt::QueuedConnection);
}

void QCanvas2DItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QCanvas2DItem);

    QCanvasPainterItem::geometryChange(newGeometry, oldGeometry);

    // Due to indirect recursion, newGeometry may be outdated
    // after this call, so we use width and height instead.
    QSizeF newSize = QSizeF(width(), height());
    if (d->available && newSize != oldGeometry.size()) {
        if (isVisible() || (d->extra.isAllocated() && d->extra->effectRefCount > 0))
            requestPaint();
    }
}

void QCanvas2DItem::releaseResources()
{
    Q_D(QCanvas2DItem);

    if (d->context) {
        delete d->context;
        d->context = nullptr;
    }
}

bool QCanvas2DItem::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::PolishRequest:
        polish();
        return true;
    default:
        return QCanvasPainterItem::event(event);
    }
}

void QCanvas2DItem::invalidateSceneGraph()
{
    Q_D(QCanvas2DItem);
    if (d->context)
        d->context->deleteLater();
    d->context = nullptr;

    // As we can expect(/hope) that the SG will be "good again", we can requestPaint
    // Otherwise this Canvas will be "blank" when SG comes back
    requestPaint();
}

void QCanvas2DItem::schedulePolish()
{
    auto polishRequestEvent = new QEvent(QEvent::PolishRequest);
    QCoreApplication::postEvent(this, polishRequestEvent);
}

void QCanvas2DItem::componentComplete()
{
    QCanvasPainterItem::componentComplete();

    Q_D(QCanvas2DItem);
    d->baseUrl = qmlEngine(this)->contextForObject(this)->baseUrl();
}

void QCanvas2DItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    QCanvasPainterItem::itemChange(change, value);
    if (change != QQuickItem::ItemSceneChange)
        return;

    Q_D(QCanvas2DItem);
    if (d->available) {
        if (d->dirtyAttributes & QQuickItemPrivate::ContentUpdateMask)
            requestPaint();
        return;
    }

    if (value.window == nullptr)
        return;

    d->window = value.window;
    QSGRenderContext *context = QQuickWindowPrivate::get(d->window)->context;

    if (context) {
        // Defer the call. In some (arguably incorrect) cases we get here due
        // to ItemSceneChange with the user-supplied property values not yet
        // set. Work this around by a deferred invoke. (QTBUG-49692)
        QMetaObject::invokeMethod(this, "sceneGraphInitialized", Qt::QueuedConnection);
    } else {
        connect(d->window, SIGNAL(sceneGraphInitialized()), SLOT(sceneGraphInitialized()));
    }
}

void QCanvas2DItem::updatePolish()
{
    QCanvasPainterItem::updatePolish();

    Q_D(QCanvas2DItem);

    if (d->animationCallbacks.size() > 0 && isVisible()) {
        QMap<int, QV4::PersistentValue> animationCallbacks = d->animationCallbacks;
        d->animationCallbacks.clear();

        QV4::ExecutionEngine *v4 = qmlEngine(this)->handle();
        QV4::Scope scope(v4);
        QV4::ScopedFunctionObject function(scope);
        QV4::JSCallArguments jsCall(scope, 1);
        *jsCall.thisObject = QV4::QObjectWrapper::wrap(v4, this);

        for (auto it = animationCallbacks.cbegin(), end = animationCallbacks.cend(); it != end; ++it) {
            function = it.value().value();
            jsCall.args[0] = QV4::Value::fromUInt32(QDateTime::currentMSecsSinceEpoch());
            function->call(jsCall);
        }
    } else {
        // TODO: Always call paint()?
        emit paint();
    }

    if (d->context)
        d->context->flush();
}


QCanvas2DCommandBuffer *QCanvas2DItem::ccb() const
{
    Q_D(const QCanvas2DItem);
    return d->ccb;
}

void QCanvas2DItem::setCcb(QCanvas2DCommandBuffer *ccb)
{
    Q_D(QCanvas2DItem);
    d->ccb = ccb;
    update();
}

/*!
    \qmlmethod object Canvas2D::getContext(string contextId, ... args)

    Returns a drawing context, or \c null if no context is available.

    The \a contextId parameter names the required context. The Canvas2D item
    will return a context that implements the required drawing mode. After the
    first call to getContext, any subsequent call to getContext with the same
    contextId will return the same context object. Any additional arguments
    (\a args) are currently ignored.

    If the context type is not supported or the canvas has previously been
    requested to provide a different and incompatible context type, \c null
    will be returned.

    Canvas2D only supports a 2d context.

*/

void QCanvas2DItem::getContext(QQmlV4FunctionPtr args)
{
    Q_D(QCanvas2DItem);

    QV4::Scope scope(args->v4engine());
    QV4::ScopedString str(scope, (*args)[0]);
    if (!str) {
        qmlWarning(this) << "getContext should be called with a string naming the required context type";
        args->setReturnValue(QV4::Encode::null());
        return;
    }

    if (!d->available) {
        qmlWarning(this) << "Unable to use getContext() at this time, please wait for available: true";
        args->setReturnValue(QV4::Encode::null());
        return;
    }

    QString contextId = str->toQString();

    if (d->context != nullptr) {
        if (d->context->contextNames().contains(contextId, Qt::CaseInsensitive)) {
            args->setReturnValue(d->context->v4value());
            return;
        }

        qmlWarning(this) << "Canvas already initialized with a different context type";
        args->setReturnValue(QV4::Encode::null());
        return;
    }

    if (createContext(contextId))
        args->setReturnValue(d->context->v4value());
    else
        args->setReturnValue(QV4::Encode::null());
}

/*!
    \qmlmethod int Canvas2D::requestAnimationFrame(callback)

    This function schedules \a callback to be invoked before composing the Qt Quick
    scene.
*/

void QCanvas2DItem::requestAnimationFrame(QQmlV4FunctionPtr args)
{
    QV4::Scope scope(args->v4engine());
    QV4::ScopedFunctionObject f(scope, (*args)[0]);
    if (!f) {
        qmlWarning(this) << "requestAnimationFrame should be called with an animation callback function";
        args->setReturnValue(QV4::Encode::null());
        return;
    }

    Q_D(QCanvas2DItem);

    static int id = 0;

    d->animationCallbacks.insert(++id, QV4::PersistentValue(scope.engine, f->asReturnedValue()));

    // QTBUG-55778: Calling polish directly here can lead to a polish loop
    if (isVisible())
        schedulePolish();

    args->setReturnValue(QV4::Encode(id));
}

/*!
    \qmlmethod void Canvas2D::cancelRequestAnimationFrame(int handle)

    This function will cancel the animation callback referenced by \a handle.
*/

void QCanvas2DItem::cancelRequestAnimationFrame(QQmlV4FunctionPtr args)
{
    QV4::Scope scope(args->v4engine());
    QV4::ScopedValue v(scope, (*args)[0]);
    if (!v->isInteger()) {
        qmlWarning(this) << "cancelRequestAnimationFrame should be called with an animation callback id";
        args->setReturnValue(QV4::Encode::null());
        return;
    }

    d_func()->animationCallbacks.remove(v->integerValue());
}


/*!
    \qmlmethod void Canvas2D::requestPaint()

    Request the entire visible region be re-drawn.

    \sa markDirty()
*/

void QCanvas2DItem::requestPaint()
{
    markDirty();
}

/*!
    \qmlmethod void Canvas2D::markDirty()

    Marks the canvas as dirty. This will trigger the \c paint signal.

    \sa paint, requestPaint()
*/

void QCanvas2DItem::markDirty()
{
    Q_D(QCanvas2DItem);
    if (!d->available)
        return;

    polish();
}

void QCanvas2DItem::checkAnimationCallbacks()
{
    if (d_func()->animationCallbacks.size() > 0 && isVisible())
        polish();
}

QQmlRefPointer<QCanvas2DPixmap> QCanvas2DItem::loadedPixmap(const QUrl& url, QSizeF sourceSize)
{
    Q_D(QCanvas2DItem);
    QUrl fullPathUrl = d->baseUrl.resolved(url);
    if (!d->pixmaps.contains(fullPathUrl)) {
        loadImage(url, sourceSize);
    }
    return d->pixmaps.value(fullPathUrl);
}

/*!
    \qmlsignal Canvas2D::imageLoaded()

    This signal is emitted when an image has been loaded.

    \sa loadImage()
*/

/*!
    \qmlmethod void Canvas2D::loadImage(url image, size sourceSize = undefined)

    Loads the given \a image asynchronously.

    Once the image is ready, imageLoaded() signal will be emitted.
    The loaded image can be unloaded with the unloadImage() method.

    \note Only loaded images can be painted on the Canvas2D item.

    If \a sourceSize is specified, the image will be scaled to that size during loading. This is
    useful for loading scalable (vector) images (eg. SVGs) at their intended display size.

    \sa unloadImage(), imageLoaded(), isImageLoaded(),
        Context2D::createImageData(), Context2D::drawImage()
*/
void QCanvas2DItem::loadImage(const QUrl& url, QSizeF sourceSize)
{
    Q_D(QCanvas2DItem);
    QUrl fullPathUrl = d->baseUrl.resolved(url);
    if (!d->pixmaps.contains(fullPathUrl)) {
        QQuickPixmap* pix = new QQuickPixmap();
        QQmlRefPointer<QCanvas2DPixmap> canvasPix;
        canvasPix.adopt(new QCanvas2DPixmap(pix));
        d->pixmaps.insert(fullPathUrl, canvasPix);

        pix->load(qmlEngine(this)
                , fullPathUrl
                , QRect()
                , sourceSize.toSize()
                , QQuickPixmap::Cache | QQuickPixmap::Asynchronous);
        if (pix->isLoading())
            pix->connectFinished(this, SIGNAL(imageLoaded()));
    }
}
/*!
    \qmlmethod void Canvas2D::unloadImage(url image)

    Unloads the \a image.

    Once an image is unloaded, it cannot be painted by the canvas context
    unless it is loaded again.

    \sa loadImage(), imageLoaded(), isImageLoaded(),
        Context2D::createImageData(), Context2D::drawImage
*/
void QCanvas2DItem::unloadImage(const QUrl& url)
{
    Q_D(QCanvas2DItem);
    d->pixmaps.remove(d->baseUrl.resolved(url));
}

/*!
    \qmlmethod bool Canvas2D::isImageError(url image)

    Returns \c true if the \a image failed to load, \c false otherwise.

    \sa loadImage()
*/
bool QCanvas2DItem::isImageError(const QUrl& url) const
{
    Q_D(const QCanvas2DItem);
    QUrl fullPathUrl = d->baseUrl.resolved(url);
    return d->pixmaps.contains(fullPathUrl)
        && d->pixmaps.value(fullPathUrl)->pixmap()->isError();
}

/*!
  \qmlmethod bool Canvas2D::isImageLoading(url image)
  Returns true if the \a image is currently loading.

  \sa loadImage()
*/
bool QCanvas2DItem::isImageLoading(const QUrl& url) const
{
    Q_D(const QCanvas2DItem);
    QUrl fullPathUrl = d->baseUrl.resolved(url);
    return d->pixmaps.contains(fullPathUrl)
        && d->pixmaps.value(fullPathUrl)->pixmap()->isLoading();
}
/*!
  \qmlmethod bool Canvas2D::isImageLoaded(url image)
  Returns true if the \a image is successfully loaded and ready to use.

  \sa loadImage()
*/
bool QCanvas2DItem::isImageLoaded(const QUrl& url) const
{
    Q_D(const QCanvas2DItem);
    QUrl fullPathUrl = d->baseUrl.resolved(url);
    return d->pixmaps.contains(fullPathUrl)
        && d->pixmaps.value(fullPathUrl)->pixmap()->isReady();
}

void QCanvas2DItem::delayedCreate()
{
    Q_D(QCanvas2DItem);

    if (!d->context && !d->contextType.isNull())
        createContext(d->contextType);

    requestPaint();
}

bool QCanvas2DItem::createContext(const QString &contextType)
{
    Q_D(QCanvas2DItem);

    if (!window())
        return false;

    if (contextType == QStringLiteral("2d")) {
        if (d->contextType.compare(QStringLiteral("2d"), Qt::CaseInsensitive) != 0)  {
            d->contextType = QStringLiteral("2d");
            emit contextTypeChanged(); // XXX: can't be in setContextType()
        }
        initializeContext(new QCanvas2DContext(this));
        return true;
    }

    return false;
}

void QCanvas2DItem::initializeContext(QCanvas2DContext *context, const QVariantMap &args)
{
    Q_D(QCanvas2DItem);

    d->context = context;
    d->context->init(this, args);
    d->context->setV4Engine(qmlEngine(this)->handle());
    emit contextChanged();
}

QCanvasPainterItemRenderer* QCanvas2DItem::createItemRenderer() const
{
    auto *renderer = new QCanvas2DItemRenderer();
    QObject::connect(renderer, &QCanvas2DItemRenderer::painted,
                     this, &QCanvas2DItem::painted);
    return renderer;
}

/*!
    \qmlsignal Canvas2D::paint(rect region)

    This signal is emitted when the \a region needs to be rendered. If a context
    is active it can be referenced from the context property.

    This signal can be triggered by markDirty(), requestPaint() or by changing
    the current canvas window.
*/

/*!
    \qmlsignal Canvas2D::painted()

    This signal is emitted after all context painting commands are executed and
    the Canvas has been rendered.
*/

QT_END_NAMESPACE
