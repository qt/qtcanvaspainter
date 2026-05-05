// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dcontext_p.h"
#include "qcanvas2dcommandbuffer_p.h"
#include "qcanvas2ditem_p.h"
#include "qcanvas2dutils_p.h"

#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCanvasPainter/qcanvaslineargradient.h>
#include <QtCanvasPainter/qcanvasradialgradient.h>
#include <QtCanvasPainter/qcanvasconicalgradient.h>
#include <QtCanvasPainter/qcanvasboxgradient.h>
#include <QtCanvasPainter/qcanvasboxshadow.h>
#include <QtCanvasPainter/qcanvasimagepattern.h>
#include <QtCanvasPainter/qcanvasgridpattern.h>
#include <QtCanvasPainter/qcanvascustombrush.h>
#include <QtCanvasPainter/qcanvaspath.h>

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/qsgrendererinterface.h>
#include <QtQuick/private/qsgcontext_p.h>
#include <QtQuick/private/qquickimage_p_p.h>
#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/private/qsgdefaultrendercontext_p.h>

#include <QtQml/qqmlinfo.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/private/qv4domerrors_p.h>
#include <QtQml/private/qv4engine_p.h>
#include <QtQml/private/qv4object_p.h>
#include <QtQml/private/qv4qobjectwrapper_p.h>
#include <QtQml/private/qv4value_p.h>
#include <QtQml/private/qv4functionobject_p.h>
#include <QtQml/private/qv4objectproto_p.h>
#include <QtQml/private/qv4scopedvalue_p.h>

#include <QtCore/qmath.h>
#include <QtCore/qlist.h>
#include <QtCore/qnumeric.h>
#include <QtCore/qpointer.h>

#include <QtGui/qguiapplication.h>

#include <cmath>
#if defined(Q_OS_QNX) || defined(Q_OS_ANDROID)
#include <ctype.h>
#endif

QT_BEGIN_NAMESPACE
/*!
    \qmltype Canvas2DContext
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides 2D context for drawing on a Canvas2D item.

    The Canvas2DContext object can be created by \c Canvas item's \c getContext()
    method:
    \code
    Canvas2D {
      id: canvas
      onPaint: {
         var ctx = canvas.getContext('2d');
         //...
      }
    }
    \endcode
    The Context2D API implements the same \l
    {http://www.w3.org/TR/2dcontext}{W3C Canvas 2D Context API standard} with
    some enhanced features.

    The Context2D API provides the rendering \b{context} which defines the
    methods and attributes needed to draw on the \c Canvas item. The following
    assigns the canvas rendering context to a \c{context} variable:
    \code
    var context = mycanvas.getContext("2d")
    \endcode

    The Context2D API renders the canvas as a coordinate system whose origin
    (0,0) is at the top left corner, as shown in the figure below. Coordinates
    increase along the \c{x} axis from left to right and along the \c{y} axis
    from top to bottom of the canvas.
*/


#define CHECK_CONTEXT(r)     if (!r || !r->d()->context() || !r->d()->context()->bufferValid()) \
THROW_GENERIC_ERROR("Not a Context2D object");

#define CHECK_PATH_OBJECT(r)     if (!r || !r->d()->path) \
THROW_GENERIC_ERROR("Not a Path2D object");

class QCanvas2DContextEngineData : public QV4::ExecutionEngine::Deletable
{
public:
    QCanvas2DContextEngineData(QV4::ExecutionEngine *engine);
    ~QCanvas2DContextEngineData();

    QV4::PersistentValue contextPrototype;
    QV4::PersistentValue gradientPrototype;
    QV4::PersistentValue pathPrototype;
};

V4_DEFINE_EXTENSION(QCanvas2DContextEngineData, engineData)

namespace QV4 {
namespace Heap {

struct QCanvasJSContext2D : Object {
    void init()
    {
        Object::init();
        m_context = nullptr;
    }

    void destroy()
    {
        delete m_context;
        Object::destroy();
    }

    QCanvas2DContext *context() { return m_context ? *m_context : nullptr; }
    void setContext(QCanvas2DContext *context)
    {
        if (m_context)
            *m_context = context;
        else
            m_context = new QPointer<QCanvas2DContext>(context);
    }

private:
    QPointer<QCanvas2DContext>* m_context;
};

struct QCanvasJSContext2DPrototype : Object {
    void init() { Object::init(); }
};

struct QCanvas2DGradientObject : Object {
    void init()
    {
        brush = nullptr;
        patternRepeatX = false;
        patternRepeatY = false;
    }
    void destroy() {
        delete brush;
        Object::destroy();
    }

    QCanvasBrush *brush;
    bool patternRepeatX:1;
    bool patternRepeatY:1;
};

struct QCanvas2DShadowObject : Object {
    void init()
    {
        brush = nullptr;
    }
    void destroy() {
        delete brush;
        Object::destroy();
    }

    QCanvasBrush *brush;
};

struct QCanvas2DGridObject : Object {
    void init()
    {
        brush = nullptr;
    }
    void destroy() {
        delete brush;
        Object::destroy();
    }

    QCanvasBrush *brush;
};

struct QCanvas2DPath2DObject : Object {
    void init()
    {
        path = nullptr;
    }
    void destroy() {
        delete path;
        Object::destroy();
    }

    QCanvasPath *path;
};

} // Heap
} // QV4


struct QCanvasJSContext2D : public QV4::Object
{
    V4_OBJECT2(QCanvasJSContext2D, QV4::Object)
    V4_NEEDS_DESTROY

    static QV4::ReturnedValue method_get_globalAlpha(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_globalAlpha(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_globalBrightness(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_globalBrightness(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_globalContrast(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_globalContrast(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_globalSaturate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_globalSaturate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_globalCompositeOperation(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_globalCompositeOperation(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_fillStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_fillStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_fillRule(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_fillRule(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_strokeStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_strokeStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);

    static QV4::ReturnedValue method_get_lineCap(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_lineCap(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_lineJoin(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_lineJoin(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_lineWidth(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_lineWidth(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_miterLimit(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_miterLimit(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_lineDashOffset(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_lineDashOffset(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_antialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_antialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_font(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_font(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_textAlign(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_textAlign(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_textBaseline(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_textBaseline(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
};

DEFINE_OBJECT_VTABLE(QCanvasJSContext2D);


struct QCanvasJSContext2DPrototype : public QV4::Object
{
    V4_OBJECT2(QCanvasJSContext2DPrototype, QV4::Object)
public:
    static QV4::Heap::QCanvasJSContext2DPrototype *create(QV4::ExecutionEngine *engine)
    {
        QV4::Scope scope(engine);
        QV4::Scoped<QCanvasJSContext2DPrototype> o(scope, engine->memoryManager->allocate<QCanvasJSContext2DPrototype>());
        // Start: Path methods. These should match with Path2D & QCanvasPath
        o->defineDefaultProperty(QStringLiteral("closePath"), method_closePath, 0);
        o->defineDefaultProperty(QStringLiteral("moveTo"), method_moveTo, 0);
        o->defineDefaultProperty(QStringLiteral("lineTo"), method_lineTo, 0);
        o->defineDefaultProperty(QStringLiteral("bezierCurveTo"), method_bezierCurveTo, 0);
        o->defineDefaultProperty(QStringLiteral("quadraticCurveTo"), method_quadraticCurveTo, 0);
        o->defineDefaultProperty(QStringLiteral("arcTo"), method_arcTo, 0);
        o->defineDefaultProperty(QStringLiteral("arc"), method_arc, 0);
        o->defineDefaultProperty(QStringLiteral("rect"), method_rect, 0);
        o->defineDefaultProperty(QStringLiteral("roundedRect"), method_roundedRect, 0);
        o->defineDefaultProperty(QStringLiteral("roundRect"), method_roundRect, 0);
        o->defineDefaultProperty(QStringLiteral("ellipse"), method_ellipse, 0);
        o->defineDefaultProperty(QStringLiteral("ellipseRect"), method_ellipseRect, 0);
        o->defineDefaultProperty(QStringLiteral("circle"), method_circle, 0);
        o->defineDefaultProperty(QStringLiteral("beginSolidSubPath"), method_beginSolidSubPath, 0);
        o->defineDefaultProperty(QStringLiteral("beginHoleSubPath"), method_beginHoleSubPath, 0);
        // TODO: Missing compared to QCanvasPath: setPathWinding(), addPath()
        // End: Path Methods.
        o->defineDefaultProperty(QStringLiteral("restore"), method_restore, 0);
        o->defineDefaultProperty(QStringLiteral("caretBlinkRate"), method_caretBlinkRate, 0);
        o->defineDefaultProperty(QStringLiteral("clip"), method_clip, 0);
        o->defineDefaultProperty(QStringLiteral("clipRect"), method_clipRect, 0);
        o->defineDefaultProperty(QStringLiteral("resetClipping"), method_resetClipping, 0);
        o->defineDefaultProperty(QStringLiteral("setTransform"), method_setTransform, 0);
        o->defineDefaultProperty(QStringLiteral("getTransform"), method_getTransform, 0);
        o->defineDefaultProperty(QStringLiteral("createPattern"), method_createPattern, 0);
        o->defineDefaultProperty(QStringLiteral("stroke"), method_stroke, 0);
        o->defineDefaultProperty(QStringLiteral("measureText"), method_measureText, 0);
        o->defineDefaultProperty(QStringLiteral("fill"), method_fill, 0);
        o->defineDefaultProperty(QStringLiteral("save"), method_save, 0);
        o->defineDefaultProperty(QStringLiteral("scale"), method_scale, 0);
        o->defineDefaultProperty(QStringLiteral("drawImage"), method_drawImage, 0);
        o->defineDefaultProperty(QStringLiteral("transform"), method_transform, 0);
        o->defineDefaultProperty(QStringLiteral("fillText"), method_fillText, 0);
        o->defineDefaultProperty(QStringLiteral("strokeText"), method_strokeText, 0);
        o->defineDefaultProperty(QStringLiteral("translate"), method_translate, 0);
        o->defineDefaultProperty(QStringLiteral("createRadialGradient"), method_createRadialGradient, 0);
        o->defineDefaultProperty(QStringLiteral("createBoxGradient"), method_createBoxGradient, 0);
        o->defineDefaultProperty(QStringLiteral("createBoxShadow"), method_createBoxShadow, 0);
        o->defineDefaultProperty(QStringLiteral("createGridPattern"), method_createGridPattern, 0);
        o->defineDefaultProperty(QStringLiteral("createPath2D"), method_createPath2D, 0);
        o->defineDefaultProperty(QStringLiteral("createTransform2D"), method_createTransform2D, 0);
        o->defineDefaultProperty(QStringLiteral("drawBoxShadow"), method_drawBoxShadow, 0);
        // Note: Canvas Painter uses "skew" while Quick Canvas uses "shear", so support both.
        o->defineDefaultProperty(QStringLiteral("skew"), method_shear, 0);
        o->defineDefaultProperty(QStringLiteral("shear"), method_shear, 0);
        o->defineDefaultProperty(QStringLiteral("isPointInPath"), method_isPointInPath, 0);
        o->defineDefaultProperty(QStringLiteral("resetTransform"), method_resetTransform, 0);
        o->defineDefaultProperty(QStringLiteral("fillRect"), method_fillRect, 0);
        o->defineDefaultProperty(QStringLiteral("createConicalGradient"), method_createConicalGradient, 0);
        o->defineDefaultProperty(QStringLiteral("createConicGradient"), method_createConicGradient, 0);
        o->defineDefaultProperty(QStringLiteral("drawFocusRing"), method_drawFocusRing, 0);
        o->defineDefaultProperty(QStringLiteral("beginPath"), method_beginPath, 0);
        o->defineDefaultProperty(QStringLiteral("clearRect"), method_clearRect, 0);
        o->defineDefaultProperty(QStringLiteral("reset"), method_reset, 0);
        o->defineDefaultProperty(QStringLiteral("rotate"), method_rotate, 0);
        o->defineDefaultProperty(QStringLiteral("setCaretSelectionRect"), method_setCaretSelectionRect, 0);
        o->defineDefaultProperty(QStringLiteral("createImageData"), method_createImageData, 0);
        o->defineDefaultProperty(QStringLiteral("putImageData"), method_putImageData, 0);
        o->defineDefaultProperty(QStringLiteral("getImageData"), method_getImageData, 0);
        o->defineDefaultProperty(QStringLiteral("createLinearGradient"), method_createLinearGradient, 0);
        o->defineDefaultProperty(QStringLiteral("strokeRect"), method_strokeRect, 0);
        o->defineDefaultProperty(QStringLiteral("setLineDash"), method_setLineDash, 0);
        o->defineDefaultProperty(QStringLiteral("getLineDash"), method_getLineDash, 0);
        o->defineAccessorProperty(QStringLiteral("canvas"), QCanvasJSContext2DPrototype::method_get_canvas, nullptr);

        return o->d();
    }

    // Start: Path methods. These should match with Path2D & QCanvasPath
    static QV4::ReturnedValue method_closePath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_moveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_lineTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_bezierCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_quadraticCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_arcTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_arc(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_rect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_roundedRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_roundRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_ellipse(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_ellipseRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_circle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_beginSolidSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_beginHoleSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    // End: Path Methods.
    static QV4::ReturnedValue method_get_canvas(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_restore(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_reset(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_save(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_rotate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_scale(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_translate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_setTransform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_getTransform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_transform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_resetTransform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_shear(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createLinearGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createRadialGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createConicalGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createConicGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createBoxGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createBoxShadow(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createGridPattern(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createPath2D(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createTransform2D(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_drawBoxShadow(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createPattern(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_clearRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_fillRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_strokeRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_beginPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_clip(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_clipRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_resetClipping(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_fill(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_stroke(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_isPointInPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_drawFocusRing(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_setCaretSelectionRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_caretBlinkRate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_fillText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_strokeText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_measureText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_drawImage(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_createImageData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_getImageData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_putImageData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_setLineDash(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_getLineDash(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);

};

DEFINE_OBJECT_VTABLE(QCanvasJSContext2DPrototype);


struct QCanvas2DGradientObject : public QV4::Object
{
    V4_OBJECT2(QCanvas2DGradientObject, QV4::Object)
    V4_NEEDS_DESTROY

    static QV4::ReturnedValue gradient_proto_addColorStop(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
};

DEFINE_OBJECT_VTABLE(QCanvas2DGradientObject);

struct QCanvas2DShadowObject : public QV4::Object
{
    V4_OBJECT2(QCanvas2DShadowObject, QV4::Object)
    V4_NEEDS_DESTROY
};

DEFINE_OBJECT_VTABLE(QCanvas2DShadowObject);

struct QCanvas2DGridObject : public QV4::Object
{
    V4_OBJECT2(QCanvas2DGridObject, QV4::Object)
    V4_NEEDS_DESTROY
};

DEFINE_OBJECT_VTABLE(QCanvas2DGridObject);

struct QCanvas2DPath2DObject : public QV4::Object
{
    V4_OBJECT2(QCanvas2DPath2DObject, QV4::Object)
    V4_NEEDS_DESTROY

    static QV4::ReturnedValue path_proto_addPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_closePath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_moveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_lineTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_bezierCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_quadraticCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_arcTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_arc(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_rect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_roundedRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_roundRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_ellipse(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_ellipseRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_circle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_beginSolidSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_beginHoleSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_isEmpty(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue path_proto_clear(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
};

DEFINE_OBJECT_VTABLE(QCanvas2DPath2DObject);

static QCanvasPainter::CompositeOperation qcanvas_composite_mode_from_string(const QString &compositeOperator)
{
    if (compositeOperator == QStringLiteral("source-over")) {
        return QCanvasPainter::CompositeOperation::SourceOver;
    } else if (compositeOperator == QStringLiteral("source-atop")) {
        return QCanvasPainter::CompositeOperation::SourceAtop;
    } else if (compositeOperator == QStringLiteral("destination-out")) {
        return QCanvasPainter::CompositeOperation::DestinationOut;
    }
    return QCanvasPainter::CompositeOperation::SourceOver;
}

static QString qcanvas_composite_mode_to_string(QCanvasPainter::CompositeOperation op)
{
    switch (op) {
    case QCanvasPainter::CompositeOperation::SourceOver:
        return QStringLiteral("source-over");
    case QCanvasPainter::CompositeOperation::SourceAtop:
        return QStringLiteral("source-atop");
    case QCanvasPainter::CompositeOperation::DestinationOut:
        return QStringLiteral("destination-out");
    default:
        break;
    }
    return QString();
}

//static script functions

// ******************** Start: Path methods. ********************

/*!
  \qmlmethod object Canvas2DContext::closePath()
   Closes the current subpath by drawing a line to the beginning of the subpath, automatically starting a new path.
   The current point of the new path is the previous subpath's first point.

   \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-closepath}{W3C 2d context standard for closePath}
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_closePath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    r->d()->context()->buffer()->closePath();

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::moveTo(real x, real y)

   Creates a new subpath with a point at (\a x, \a y).
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_moveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 2) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();
        r->d()->context()->buffer()->moveTo(x, y);
    }

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::lineTo(real x, real y)

   Draws a line from the current position to the point at (\a x, \a y).
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_lineTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 2) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->lineTo(x, y);
    }

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::bezierCurveTo(real cp1x, real cp1y, real cp2x, real cp2y, real x, real y)

  Adds a cubic bezier curve between the current position and the given endPoint using the control points specified by (\a {cp1x}, \a {cp1y}),
  and (\a {cp2x}, \a {cp2y}).
  After the curve is added, the current position is updated to be at the end point (\a {x}, \a {y}) of the curve.
  The following code produces the path shown below:

  \code
  ctx.strokeStyle = Qt.rgba(0, 0, 0, 1);
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(20, 0); //start point
  ctx.bezierCurveTo(-10, 90, 210, 90, 180, 0);
  ctx.stroke();
  \endcode

  \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-beziercurveto}{W3C 2d context standard for bezierCurveTo}
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_bezierCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 6) {
        qreal cp1x = argv[0].toNumber();
        qreal cp1y = argv[1].toNumber();
        qreal cp2x = argv[2].toNumber();
        qreal cp2y = argv[3].toNumber();
        qreal x = argv[4].toNumber();
        qreal y = argv[5].toNumber();

        if (!qt_is_finite(cp1x) || !qt_is_finite(cp1y) || !qt_is_finite(cp2x) || !qt_is_finite(cp2y) || !qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
    }
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::quadraticCurveTo(real cpx, real cpy, real x, real y)

    Adds a quadratic bezier curve between the current point and the endpoint
    (\a x, \a y) with the control point specified by (\a cpx, \a cpy).

    \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-quadraticcurveto}{W3C 2d context standard for quadraticCurveTo}
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_quadraticCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal cpx = argv[0].toNumber();
        qreal cpy = argv[1].toNumber();
        qreal x = argv[2].toNumber();
        qreal y = argv[3].toNumber();

        if (!qt_is_finite(cpx) || !qt_is_finite(cpy) || !qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->quadraticCurveTo(cpx, cpy, x, y);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::arcTo(real x1, real y1, real x2,
        real y2, real radius)

    Adds an arc with the given control points and radius to the current subpath,
    connected to the previous point by a straight line. To draw an arc, you
    begin with the same steps you followed to create a line:

    \list
    \li Call the beginPath() method to set a new path.
    \li Call the moveTo(\c x, \c y) method to set your starting position on the
        canvas at the point (\c x, \c y).
    \li To draw an arc or circle, call the arcTo(\a x1, \a y1, \a x2, \a y2,
        \a radius) method. This adds an arc with starting point (\a x1, \a y1),
        ending point (\a x2, \a y2), and \a radius to the current subpath and
        connects it to the previous subpath by a straight line.
    \endlist

    \sa arc, {http://www.w3.org/TR/2dcontext/#dom-context-2d-arcto}{W3C's 2D
    Context Standard for arcTo()}
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_arcTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 5) {
        qreal c1x = argv[0].toNumber();
        qreal c1y = argv[1].toNumber();
        qreal c2x = argv[2].toNumber();
        qreal c2y = argv[3].toNumber();
        qreal radius = argv[4].toNumber();

        if (!qt_is_finite(c1x) || !qt_is_finite(c1y) ||
            !qt_is_finite(c2x) || !qt_is_finite(c2y)) {
            RETURN_UNDEFINED();
        }

        if (qt_is_finite(radius) && radius < 0)
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "Incorrect argument radius");

        r->d()->context()->buffer()->arcTo(c1x,
                                        c1y,
                                        c2x,
                                        c2y,
                                        radius);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::arc(real x, real y, real radius,
        real startAngle, real endAngle, bool anticlockwise)

    Adds an arc to the current subpath that lies on the circumference of the
    circle whose center is at the point (\a x, \a y) and whose radius is
    \a radius.

    Both \a startAngle and \a endAngle are measured from the x-axis in radians.

    The default curve direction is clockwise. To change direction to opposite,
    set \a anticlockwise to true.

    \sa arcTo, {http://www.w3.org/TR/2dcontext/#dom-context-2d-arc}{W3C's 2D
    Context Standard for arc()}
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_arc(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 5) {
        qreal centerX = argv[0].toNumber();
        qreal centerY = argv[1].toNumber();
        qreal radius = argv[2].toNumber();
        qreal a0 = argv[3].toNumber();
        qreal a1 = argv[4].toNumber();
        bool antiClockwise = (argc >= 6) ? argv[5].toBoolean() : false;

        if (!qt_is_finite(centerX) || !qt_is_finite(centerY) ||
            !qt_is_finite(a0) || !qt_is_finite(a1)) {
            RETURN_UNDEFINED();
        }

        if (qt_is_finite(radius) && radius < 0)
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "Incorrect argument radius");

        r->d()->context()->buffer()->arc(centerX,
                                      centerY,
                                      radius,
                                      a0,
                                      a1,
                                      antiClockwise);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::rect(real x, real y, real w, real h)

    Adds a rectangle at position (\a x, \a y), with the given width \a w and
    height \a h, as a closed subpath.
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_rect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->rect(x, y, w, h);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::roundedRect(real x, real y, real w, real h, real xRadius, real yRadius)

    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a w, \a h), to the path.
    The \a xRadius and \a yRadius arguments specify the radius of the
    ellipses defining the corners of the rounded rectangle.
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_roundedRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 6) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal yr = argv[4].toNumber();
        qreal xr = argv[5].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h) ||
            !qt_is_finite(yr) || !qt_is_finite(xr)) {
            RETURN_UNDEFINED();
        }

        r->d()->context()->buffer()->roundRect(x, y, w, h, yr, yr, xr, xr);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::roundRect(real x, real y, real w, real h, real radius)

    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a w, \a h), to the path.
    The \a radius argument specify the radius of the
    ellipses defining the corners of the rounded rectangle.
 */

/*!
    \qmlmethod object Canvas2DContext::roundRect(real x, real y, real w, real h,
                   real radiusTopLeft, real radiusTopRight,
                   real radiusBottomRight, real radiusBottomLeft)
    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a w, \a h), to the path.
    The \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight and \a radiusBottomLeft
    arguments specify the radius of the ellipses defining the corners of the rounded rectangle.
 */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_roundRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 8) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal rtl = argv[4].toNumber();
        qreal rtr = argv[5].toNumber();
        qreal rbr = argv[6].toNumber();
        qreal rbl = argv[7].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h) ||
            !qt_is_finite(rtl) || !qt_is_finite(rtr) || !qt_is_finite(rbr) || !qt_is_finite(rbl)) {
            RETURN_UNDEFINED();
        }

        r->d()->context()->buffer()->roundRect(x, y, w, h,
                                            rtl, rtr,
                                            rbr, rbl);
    } else if (argc >= 5) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal rad = argv[4].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h) ||
            !qt_is_finite(rad)) {
            RETURN_UNDEFINED();
        }
        r->d()->context()->buffer()->roundRect(x, y, w, h, rad, rad, rad, rad);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::ellipse(real centerX, real centerY, real radiusX, real radiusY)

    Creates new ellipse shaped sub-path into ( \a centerX, \a centerY) with
    \a radiusX and \a radiusY.

    The ellipse is composed of a clockwise curve, starting and finishing at
    zero degrees (the 3 o'clock position).
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_ellipse(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal cx = argv[0].toNumber();
        qreal cy = argv[1].toNumber();
        qreal rx = argv[2].toNumber();
        qreal ry = argv[3].toNumber();

        if (!qt_is_finite(cx) || !qt_is_finite(cy) ||
            !qt_is_finite(rx) || !qt_is_finite(ry)) {
            RETURN_UNDEFINED();
        }

        r->d()->context()->buffer()->ellipse(cx, cy, rx, ry);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::ellipseRect(real x, real y, real w, real h)

    Creates an ellipse within the bounding rectangle defined by its top-left
    corner at (\a x, \a y), width \a w and height \a h, and adds it to the
    path as a closed subpath.

    The ellipse is composed of a clockwise curve, starting and finishing at
    zero degrees (the 3 o'clock position).

    \note This method matches to \l QtQuick::Context2D::ellipse()
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_ellipseRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->ellipseRect(x, y, w, h);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::circle(real centerX, real centerY, real radius)

    Creates a circle defined by its center (\a centerX, \a centerY), and
    radius \a radius, and adds it to the path as a closed subpath.

    \note Compared to arc(), this method does not add a straight line from
    the last point in the subpath to the start point of the circle.
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_circle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 3) {
        qreal cx = argv[0].toNumber();
        qreal cy = argv[1].toNumber();
        qreal rad = argv[2].toNumber();

        if (!qt_is_finite(cx) || !qt_is_finite(cy) || !qt_is_finite(rad))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->circle(cx, cy, rad);
    }

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::beginSolidSubPath()

  Start a solid subpath.
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_beginSolidSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    r->d()->context()->buffer()->beginSolidSubPath();

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::beginHoleSubPath()

  Start a hole subpath.
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_beginHoleSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    r->d()->context()->buffer()->beginHoleSubPath();

    RETURN_RESULT(*thisObject);
}

// ******************** End: Path methods. ********************

/*!
    \qmlproperty Canvas2D Canvas2DContext::canvas
     Holds the canvas item that the context paints on.

     This property is read only.
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_get_canvas(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::QObjectWrapper::wrap(scope.engine, r->d()->context()->canvas()));
}

// ***** state handling *****

/*!
    \qmlmethod object Canvas2DContext::restore()
    Pops the top state on the stack, restoring the context to that state.

    \sa save()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_restore(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    r->d()->context()->popState();
    RETURN_RESULT(thisObject->asReturnedValue());
}

/*!
    \qmlmethod object Canvas2DContext::reset()
    Resets the context state and properties to the default values.
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_reset(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    r->d()->context()->reset();

    RETURN_RESULT(thisObject->asReturnedValue());
}

/*!
    \qmlmethod object Canvas2DContext::save()
    Pushes the current state onto the state stack.

    Before changing any state attributes, you should save the current state
    for future reference. The context maintains a stack of drawing states.
    Each state consists of the current transformation matrix, clipping region,
    and values of the following attributes:
    \list
    \li strokeStyle
    \li fillStyle
    \li fillRule
    \li globalAlpha
    \li lineWidth
    \li lineCap
    \li lineJoin
    \li miterLimit
    \li globalCompositeOperation
    \li \l font
    \li textAlign
    \li textBaseline
    \endlist

    The current path is NOT part of the drawing state. The path can be reset by
    invoking the beginPath() method.
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_save(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    r->d()->context()->pushState();

    RETURN_RESULT(*thisObject);
}

// ***** transformations *****

/*!
    \qmlmethod object Canvas2DContext::rotate(real angle)
    Rotate the canvas around the current origin by \a angle radians in clockwise direction.

    \code
    ctx.rotate(Math.PI/2);
    \endcode
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_rotate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 1)
        r->d()->context()->rotate(argv[0].toNumber());
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::scale(real sxy)

    Increases or decreases the size of each unit in the canvas grid by multiplying the
    scale factors to the current tranform matrix. Scales both the horizontal direction
    the vertical direction with the same \a sxy amount.

    The following code doubles the size of an object drawn:

    \code
    ctx.scale(2.0);
    \endcode
*/

/*!
    \qmlmethod object Canvas2DContext::scale(real sx, real sy)

    Increases or decreases the size of each unit in the canvas grid by multiplying the
    scale factors to the current tranform matrix. \a sx is the scale factor in the
    horizontal direction and \a sy is the scale factor in the vertical direction.

    The following code doubles the horizontal size of an object drawn on the canvas and halves its
    vertical size:

    \code
    ctx.scale(2.0, 0.5);
    \endcode
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_scale(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 2)
        r->d()->context()->scale(argv[0].toNumber(), argv[1].toNumber());
    else if (argc == 1)
        r->d()->context()->scale(argv[0].toNumber(), argv[0].toNumber());

    RETURN_RESULT(*thisObject);
}


/*!
    \qmlmethod object Canvas2DContext::skew(real sh, real sv)

    Skews (shears) the transformation matrix by \a sh in the horizontal direction and
    \a sv in the vertical direction. The default value of \a sv is \c 0 when
    only a single parameter is provided.
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_shear(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 1) {
        qreal h = argv[0].toNumber();
        qreal v = argc >= 2 ? argv[1].toNumber() : 0;
        r->d()->context()->shear(h, v);
    }
    RETURN_RESULT(*thisObject);
}


/*!
    \qmlmethod object Canvas2DContext::translate(real x, real y)

    Translates the origin of the canvas by a horizontal distance of \a x,
    and a vertical distance of \a y, in coordinate space units.

    Translating the origin enables you to draw patterns of different objects on the canvas
    without having to measure the coordinates manually for each shape.
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_translate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 2)
        r->d()->context()->translate(argv[0].toNumber(), argv[1].toNumber());
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::transform(real a, real b, real c, real d, real e, real f)

    This method is very similar to setTransform(), but instead of replacing
    the old transform matrix, this method applies the given tranform matrix
    to the current matrix by multiplying to it.

    The setTransform(\a a, \a b, \a c, \a d, \a e, \a f) method actually
    resets the current transform to the identity matrix, and then invokes
    the transform(\a a, \a b, \a c, \a d, \a e, \a f) method with the same
    arguments.

    \sa setTransform()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_transform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 6)
        r->d()->context()->transform( argv[0].toNumber()
                                     , argv[1].toNumber()
                                     , argv[2].toNumber()
                                     , argv[3].toNumber()
                                     , argv[4].toNumber()
                                     , argv[5].toNumber());

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::setTransform(transform2d transform)

    Changes the transformation matrix to the \a transform.
    \sa getTransform()
*/
/*!
    \qmlmethod object Canvas2DContext::setTransform(real a, real b, real c, real d, real e, real f)

    Changes the transformation matrix to the matrix given by the arguments as described below.

    Modifying the transformation matrix directly enables you to perform scaling,
    rotating, and translating transformations in a single step.

    Each point on the canvas is multiplied by the matrix before anything is
    drawn. The \l{http://www.w3.org/TR/2dcontext/#transformations}{HTML Canvas 2D Context specification}
    defines the transformation matrix where:
    \list
    \li \a{a} is the scale factor in the horizontal (x) direction
    \li \a{c} is the skew factor in the x direction
    \li \a{e} is the translation in the x direction
    \li \a{b} is the skew factor in the y (vertical) direction
    \li \a{d} is the scale factor in the y direction
    \li \a{f} is the translation in the y direction
    \li the last row remains constant
    \endlist

    The scale factors and skew factors are multiples; \a{e} and \a{f} are
    coordinate space units, just like the units in the translate(x,y)
    method.

    \sa transform()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_setTransform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)


    if (argc >= 6) {
        r->d()->context()->setTransform( argv[0].toNumber()
                                        , argv[1].toNumber()
                                        , argv[2].toNumber()
                                        , argv[3].toNumber()
                                        , argv[4].toNumber()
                                        , argv[5].toNumber());
    } else if (argc >= 1) {
        QV4::ScopedValue value(scope, argv[0]);
        if (value->as<Object>()) {
            QTransform t = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QTransform>()).value<QTransform>();
            r->d()->context()->setTransform(t.m11(), t.m12(), t.m21(), t.m22(), t.m31(), t.m32());
        }
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod transform2d Canvas2DContext::getTransform()

    Returns the current transformation matrix.

    \sa setTransform()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_getTransform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(scope.engine->fromVariant(r->d()->context()->state.transform));
}

/*!
    \qmlmethod object Canvas2DContext::resetTransform()

    Reset the transformation matrix to the default value (equivalent to calling
    setTransform(\c 1, \c 0, \c 0, \c 1, \c 0, \c 0)).

    \sa transform(), setTransform(), reset()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_resetTransform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    r->d()->context()->setTransform(1, 0, 0, 1, 0, 0);

    RETURN_RESULT(*thisObject);
}

// ***** compositing and color effects *****

/*!
    \qmlproperty real Canvas2DContext::globalAlpha

    Holds the current alpha value applied to rendering operations.
    The value must be in the range from \c 0.0 (fully transparent) to \c 1.0 (fully opaque).
    The default value is \c 1.0.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_globalAlpha(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.globalAlpha));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_globalAlpha(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    double globalAlpha = argc ? argv[0].toNumber() : qt_qnan();


    if (!qt_is_finite(globalAlpha))
        RETURN_UNDEFINED();

    if (globalAlpha >= 0.0 && globalAlpha <= 1.0 && r->d()->context()->state.globalAlpha != globalAlpha) {
        r->d()->context()->state.globalAlpha = globalAlpha;
        r->d()->context()->buffer()->setGlobalAlpha(globalAlpha);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::globalBrightness

    Holds the current brightness value applied to rendering operations.
    A value of 0 will cause painting to be completely black.
    Value can also be bigger than 1.0, to increase the brightness.
    The default value is \c 1.0.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_globalBrightness(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.globalBrightness));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_globalBrightness(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    double globalBrightness = argc ? argv[0].toNumber() : qt_qnan();


    if (!qt_is_finite(globalBrightness))
        RETURN_UNDEFINED();

    if (globalBrightness >= 0.0 && r->d()->context()->state.globalBrightness != globalBrightness) {
        r->d()->context()->state.globalBrightness = globalBrightness;
        r->d()->context()->buffer()->setGlobalBrightness(globalBrightness);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::globalContrast

    Holds the current contrast value applied to rendering operations.
    A value of 0 will cause painting to be completely gray (0.5, 0.5, 0.5).
    Value can also be bigger than 1.0, to increase the contrast.
    The default value is \c 1.0.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_globalContrast(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.globalContrast));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_globalContrast(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    double globalContrast = argc ? argv[0].toNumber() : qt_qnan();


    if (!qt_is_finite(globalContrast))
        RETURN_UNDEFINED();

    if (globalContrast >= 0.0 && r->d()->context()->state.globalContrast != globalContrast) {
        r->d()->context()->state.globalContrast = globalContrast;
        r->d()->context()->buffer()->setGlobalContrast(globalContrast);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::globalSaturate

    Holds the current saturate value applied to rendering operations.
    A value of 0 will disable saturation and cause painting to be completely grayscale.
    Value can also be bigger than 1.0, to increase the saturation.
    The default value is \c 1.0.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_globalSaturate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.globalSaturate));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_globalSaturate(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    double globalSaturate = argc ? argv[0].toNumber() : qt_qnan();


    if (!qt_is_finite(globalSaturate))
        RETURN_UNDEFINED();

    if (globalSaturate >= 0.0 && r->d()->context()->state.globalSaturate != globalSaturate) {
        r->d()->context()->state.globalSaturate = globalSaturate;
        r->d()->context()->buffer()->setGlobalSaturate(globalSaturate);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty string Canvas2DContext::globalCompositeOperation
    Holds the current the current composition operation. Allowed operations are:

    \value "source-atop"
        QCanvasPainter::CompositeOperation::SourceAtop
        A atop B. Display the source image wherever both images are opaque.
        Display the destination image wherever the destination image is opaque
        but the source image is transparent. Display transparency elsewhere.
    \value "source-over"
        QCanvasPainter::CompositeOperation::SourceOver (default)
        A over B. Display the source image wherever the source image is opaque.
        Display the destination image elsewhere.
    \value "destination-out"
        QCanvasPainter::CompositeOperation::DestinationOut
        B out A. Display the destination image wherever the destination image
        is opaque and the source image is transparent. Display transparency
        elsewhere.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_globalCompositeOperation(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(scope.engine->newString(qcanvas_composite_mode_to_string(r->d()->context()->state.globalCompositeOperation)));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_globalCompositeOperation(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (!argc)
        THROW_TYPE_ERROR();

    QString mode = argv[0].toQString();
    QCanvasPainter::CompositeOperation cm = qcanvas_composite_mode_from_string(mode);
    if (cm == QCanvasPainter::CompositeOperation::SourceOver && mode != QStringLiteral("source-over"))
        RETURN_UNDEFINED();

    if (cm != r->d()->context()->state.globalCompositeOperation) {
        r->d()->context()->state.globalCompositeOperation = cm;
        r->d()->context()->buffer()->setGlobalCompositeOperation(cm);
    }

    RETURN_UNDEFINED();
}

// ***** colors and styles *****

/*!
    \qmlproperty variant Canvas2DContext::fillStyle
     Holds the current style used for filling shapes.
     The style can be either a string containing a CSS color, a Canvas2DGradient or CanvasPattern object. Invalid values are ignored.
     This property accepts several color syntaxes:
     \list
     \li 'rgb(red, green, blue)' - for example: 'rgb(255, 100, 55)' or 'rgb(100%, 70%, 30%)'
     \li 'rgba(red, green, blue, alpha)' - for example: 'rgb(255, 100, 55, 1.0)' or 'rgb(100%, 70%, 30%, 0.5)'
     \li 'hsl(hue, saturation, lightness)'
     \li 'hsla(hue, saturation, lightness, alpha)'
     \li '#RRGGBB' - for example: '#00FFCC'
     \li Qt.rgba(red, green, blue, alpha) - for example: Qt.rgba(0.3, 0.7, 1, 1.0)
     \endlist
     If the \c fillStyle or \l strokeStyle is assigned many times in a loop, the last Qt.rgba() syntax should be chosen, as it has the
     best performance, because it's already a valid QColor value, does not need to be parsed everytime.

     The default value is  '#000000'.
     \sa createLinearGradient()
     \sa createRadialGradient()
     \sa createPattern()
     \sa strokeStyle
 */
QV4::ReturnedValue QCanvasJSContext2D::method_get_fillStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    const QColor color = r->d()->context()->state.fillColor.toRgb();
    if (color.isValid()) {
        if (color.alpha() == 255)
            RETURN_RESULT(scope.engine->newString(color.name()));
        QString alphaString = QString::number(color.alphaF(), 'f');
        while (alphaString.endsWith(QLatin1Char('0')))
            alphaString.chop(1);
        if (alphaString.endsWith(QLatin1Char('.')))
            alphaString += QLatin1Char('0');
        QString str = QString::fromLatin1("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alphaString);
        RETURN_RESULT(scope.engine->newString(str));
    }
    RETURN_RESULT(r->d()->context()->m_fillStyle.value());
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_fillStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());

    if (value->as<Object>()) {
        QColor color = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QColor>()).value<QColor>();
        if (color.isValid()) {
            r->d()->context()->state.fillColor = color;
            r->d()->context()->buffer()->setFillColor(color);
            r->d()->context()->m_fillStyle.set(scope.engine, value);
        } else {
            QV4::Scoped<QCanvas2DGradientObject> style(scope, value->as<QCanvas2DGradientObject>());
            if (style && style->d()->brush != r->d()->context()->state.fillStyle) {
                // Gradient
                r->d()->context()->state.fillStyle = style->d()->brush;
                r->d()->context()->buffer()->setFillStyle(style->d()->brush, style->d()->patternRepeatX, style->d()->patternRepeatY);
                r->d()->context()->m_fillStyle.set(scope.engine, value);
                r->d()->context()->state.fillPatternRepeatX = style->d()->patternRepeatX;
                r->d()->context()->state.fillPatternRepeatY = style->d()->patternRepeatY;
            } else {
                QV4::Scoped<QCanvas2DGridObject> style(scope, value->as<QCanvas2DGridObject>());
                if (style && style->d()->brush != r->d()->context()->state.fillStyle) {
                    // Grid pattern
                    r->d()->context()->state.fillStyle = style->d()->brush;
                    r->d()->context()->buffer()->setFillStyle(style->d()->brush);
                    r->d()->context()->m_fillStyle.set(scope.engine, value);
                }
            }
        }
    } else if (value->isString()) {
        QColor color = QCanvas2DUtils::qColorFromString(value);
        if (color.isValid()) {
            r->d()->context()->state.fillColor = color;
            r->d()->context()->buffer()->setFillColor(color);
            r->d()->context()->m_fillStyle.set(scope.engine, value);
        }
    }
    RETURN_UNDEFINED();
}

// TODO: Document if this gets supported.
QV4::ReturnedValue QCanvasJSContext2D::method_get_fillRule(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    RETURN_RESULT(scope.engine->fromVariant(r->d()->context()->state.fillRule));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_fillRule(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());

    if ((value->isString() && value->toQString() == QStringLiteral("WindingFill"))
        || (value->isInt32() && value->integerValue() == Qt::WindingFill)) {
        r->d()->context()->state.fillRule = Qt::WindingFill;
    } else if ((value->isString() && value->toQStringNoThrow() == QStringLiteral("OddEvenFill"))
               || (value->isInt32() && value->integerValue() == Qt::OddEvenFill)) {
        r->d()->context()->state.fillRule = Qt::OddEvenFill;
    } else {
        //error
    }
    // TODO: Enable when there is OddEven fillrule support.
    //r->d()->context()->m_path.setFillRule(r->d()->context()->state.fillRule);
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty variant Canvas2DContext::strokeStyle
     Holds the current color or style to use for the lines around shapes,
     The style can be either a string containing a CSS color, a Canvas2DGradient or CanvasPattern object.
     Invalid values are ignored.

     The default value is  '#000000'.

     \sa createLinearGradient()
     \sa createRadialGradient()
     \sa createPattern()
     \sa fillStyle
 */
QV4::ReturnedValue QCanvasJSContext2D::method_get_strokeStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    const QColor color = r->d()->context()->state.strokeColor.toRgb();
    if (color.isValid()) {
        if (color.alpha() == 255)
            RETURN_RESULT(scope.engine->newString(color.name()));
        QString alphaString = QString::number(color.alphaF(), 'f');
        while (alphaString.endsWith(QLatin1Char('0')))
            alphaString.chop(1);
        if (alphaString.endsWith(QLatin1Char('.')))
            alphaString += QLatin1Char('0');
        QString str = QString::fromLatin1("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alphaString);
        RETURN_RESULT(scope.engine->newString(str));
    }
    RETURN_RESULT(r->d()->context()->m_strokeStyle.value());
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_strokeStyle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());

    if (value->as<Object>()) {
        QColor color = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QColor>()).value<QColor>();
        if (color.isValid()) {
            r->d()->context()->state.strokeColor = color;
            r->d()->context()->buffer()->setStrokeColor(color);
            r->d()->context()->m_strokeStyle.set(scope.engine, value);
        } else {
            QV4::Scoped<QCanvas2DGradientObject> style(scope, value->as<QCanvas2DGradientObject>());
            if (style && style->d()->brush != r->d()->context()->state.strokeStyle) {
                // Gradient
                r->d()->context()->state.strokeStyle = style->d()->brush;
                r->d()->context()->buffer()->setStrokeStyle(style->d()->brush, style->d()->patternRepeatX, style->d()->patternRepeatY);
                r->d()->context()->m_strokeStyle.set(scope.engine, value);
                r->d()->context()->state.strokePatternRepeatX = style->d()->patternRepeatX;
                r->d()->context()->state.strokePatternRepeatY = style->d()->patternRepeatY;
            } else {
                QV4::Scoped<QCanvas2DGridObject> style(scope, value->as<QCanvas2DGridObject>());
                if (style && style->d()->brush != r->d()->context()->state.strokeStyle) {
                    // Grid pattern
                    r->d()->context()->state.strokeStyle = style->d()->brush;
                    r->d()->context()->buffer()->setStrokeStyle(style->d()->brush);
                    r->d()->context()->m_strokeStyle.set(scope.engine, value);
                }
                if (!style && !r->d()->context()->state.strokeStyle) {
                    // If there is no style object, then ensure that the strokeStyle is at least
                    // QColor in case it was previously set
                    r->d()->context()->state.strokeColor = color;
                    r->d()->context()->buffer()->setStrokeColor(color);
                    r->d()->context()->m_strokeStyle.set(scope.engine, value);
                }
            }
        }
    } else if (value->isString()) {
        QColor color = QCanvas2DUtils::qColorFromString(value);
        if (color.isValid()) {
            r->d()->context()->state.strokeColor = color;
            r->d()->context()->buffer()->setStrokeColor(color);
            r->d()->context()->m_strokeStyle.set(scope.engine, value);
        }
    }
    RETURN_UNDEFINED();
}

/*!
  \qmlmethod object Canvas2DContext::createLinearGradient(real x0, real y0, real x1, real y1)
   Returns a Canvas2DGradient object that represents a linear gradient that transitions the color along a line between
   the start point (\a x0, \a y0) and the end point (\a x1, \a y1).

   A gradient is a smooth transition between colors. There are two types of gradients: linear and radial.
   Gradients must have two or more color stops, representing color shifts positioned from 0 to 1 between
   to the gradient's starting and end points or circles.

    \sa Canvas2DGradient::addColorStop()
    \sa createRadialGradient()
    \sa createConicalGradient()
    \sa createPattern()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createLinearGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal x0 = argv[0].toNumber();
        qreal y0 = argv[1].toNumber();
        qreal x1 = argv[2].toNumber();
        qreal y1 = argv[3].toNumber();

        if (!qt_is_finite(x0)
            || !qt_is_finite(y0)
            || !qt_is_finite(x1)
            || !qt_is_finite(y1)) {
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createLinearGradient(): Incorrect arguments")
        }
        QCanvas2DContextEngineData *ed = engineData(scope.engine);

        QV4::Scoped<QCanvas2DGradientObject> gradient(scope, scope.engine->memoryManager->allocate<QCanvas2DGradientObject>());
        QV4::ScopedObject p(scope, ed->gradientPrototype.value());
        gradient->setPrototypeOf(p);
        gradient->d()->brush = new QCanvasLinearGradient(x0, y0, x1, y1);
        RETURN_RESULT(*gradient);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createRadialGradient(real x0, real y0, real r0, real x1, real y1, real r1)

    Returns a Canvas2DGradient object that represents a radial gradient that
    paints along the cone given by the start circle with origin (\a x0, \a y0)
    and radius \a r0, and the end circle with origin (\a x1, \a y1) and radius
    \a r1.

    \sa Canvas2DGradient::addColorStop()
    \sa createLinearGradient()
    \sa createConicalGradient()
    \sa createPattern()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createRadialGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 3) {
        qreal icx, icy, iRad, ocx, ocy, oRad;
        icx = argv[0].toNumber();
        icy = argv[1].toNumber();
        const bool extended = (argc >= 6);

        if (extended) {
            // With 6 parameters, order is: (icx, icy, iRad, ocx, ocy, oRad).
            iRad = argv[2].toNumber();
            ocx = argv[3].toNumber();
            ocy = argv[4].toNumber();
            // With 6 parameters (2d context API),
            // 3th is the inner radius and 6th is the outer radius.
            oRad = argv[5].toNumber();
        } else {
            // With 3-4 parameters, order is: (cx, cy, oRad, iRad).
            oRad = argv[2].toNumber();
            iRad = (argc >= 4) ? argv[3].toNumber() : 0;
            // Inner and outer centers are the same.
            ocx = icx;
            ocy = icy;
        }

        if (!qt_is_finite(icx)
            || !qt_is_finite(icy)
            || !qt_is_finite(iRad)
            || !qt_is_finite(ocx)
            || !qt_is_finite(ocy)
            || !qt_is_finite(oRad)) {
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createRadialGradient(): Incorrect arguments")
        }

        if (iRad < 0 || oRad < 0)
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "createRadialGradient(): Incorrect arguments")

        QCanvas2DContextEngineData *ed = engineData(scope.engine);

        QV4::Scoped<QCanvas2DGradientObject> gradient(scope, scope.engine->memoryManager->allocate<QCanvas2DGradientObject>());
        QV4::ScopedObject p(scope, ed->gradientPrototype.value());
        gradient->setPrototypeOf(p);

        if (extended)
            gradient->d()->brush = new QCanvasRadialGradient(icx, icy, iRad, ocx, ocy, oRad);
        else
            gradient->d()->brush = new QCanvasRadialGradient(icx, icy, oRad, iRad);

        RETURN_RESULT(*gradient);
    }

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::createConicalGradient(real x, real y, real angle)

   Returns a Canvas2DGradient object that represents a conical gradient that
   interpolates colors counter-clockwise around a center point (\a x, \a y)
   with a start angle \a angle in units of radians.

    \sa Canvas2DGradient::addColorStop()
    \sa createLinearGradient()
    \sa createRadialGradient()
    \sa createPattern()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createConicalGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 3) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal angle = argv[2].toNumber();
        if (!qt_is_finite(x) || !qt_is_finite(y)) {
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createConicalGradient(): Incorrect arguments");
        }

        if (!qt_is_finite(angle)) {
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "createConicalGradient(): Incorrect arguments");
        }

        QCanvas2DContextEngineData *ed = engineData(scope.engine);

        QV4::Scoped<QCanvas2DGradientObject> gradient(scope, scope.engine->memoryManager->allocate<QCanvas2DGradientObject>());
        QV4::ScopedObject p(scope, ed->gradientPrototype.value());
        gradient->setPrototypeOf(p);
        gradient->d()->brush = new QCanvasConicalGradient(x, y, angle);
        RETURN_RESULT(*gradient);
    }

    RETURN_RESULT(*thisObject);
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createConicGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);
    Q_UNUSED(argv);
    Q_UNUSED(argc);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createConicGradient(): Please use createConicalGradient(x, y, angle) instead.");

    RETURN_RESULT(QV4::Encode::null());
}

/*!
    \qmlmethod object Canvas2DContext::createBoxGradient(real x, real y, real width, real height, real feather, real radius)

    Returns a Canvas2DGradient object that represents a box gradient that
    covers rectangle area (\a x, \a y, \a width, \a height) with feather
    (smoothing) \a feather and corner radius \a radius.

    \sa Canvas2DGradient::addColorStop()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createBoxGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 5) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal feather = argv[4].toNumber();
        qreal radius = 0;
        if (argc >= 6)
            radius = argv[5].toNumber();

        if (!qt_is_finite(x)
            || !qt_is_finite(y)
            || !qt_is_finite(w)
            || !qt_is_finite(h)
            || !qt_is_finite(feather)
            || !qt_is_finite(radius)) {
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createBoxGradient(): Incorrect arguments")
        }

        QCanvas2DContextEngineData *ed = engineData(scope.engine);

        QV4::Scoped<QCanvas2DGradientObject> gradient(scope, scope.engine->memoryManager->allocate<QCanvas2DGradientObject>());
        QV4::ScopedObject p(scope, ed->gradientPrototype.value());
        gradient->setPrototypeOf(p);

        gradient->d()->brush = new QCanvasBoxGradient(x, y, w, h, feather, radius);
        RETURN_RESULT(*gradient);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createBoxShadow(real x, real y, real width, real height,
                                                       real blur, string color, real radius)

    Returns a Canvas2DShadow object with color \a color that represents
    a box shadow that covers rectangle area (\a x, \a y, \a width, \a height)
    with blur \a blur and corner radius \a radius.

    \sa drawBoxShadow()
  */

/*!
    \qmlmethod object Canvas2DContext::createBoxShadow(real x, real y, real width, real height,
                                                       real blur, string color,
                                                       real radiusTopLeft, real radiusTopRight,
                                                       real radiusBottomRight, real radiusBottomLeft)

    Returns a Canvas2DShadow object with color \a color that represents
    a box shadow that covers rectangle area (\a x, \a y, \a width, \a height)
    with blur \a blur and corner radius (\a radiusTopLeft, \a radiusTopRight,
    \a radiusBottomRight, \a radiusBottomLeft).

    \sa drawBoxShadow()
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createBoxShadow(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal blur = 0;
        if (argc >= 5)
            blur = argv[4].toNumber();

        QColor color = QColorConstants::Black;
        if (argc >= 6) {
            if (argv[5].as<Object>()) {
                color = QV4::ExecutionEngine::toVariant(
                                argv[5], QMetaType::fromType<QColor>()).value<QColor>();
            } else {
                color = QCanvas2DUtils::qColorFromString(argv[5]);
            }
        }

        qreal radiusTL = 0;
        qreal radiusTR = 0;
        qreal radiusBR = 0;
        qreal radiusBL = 0;
        if (argc == 7) {
            // Single radius for all corners
            radiusTL = argv[6].toNumber();
            radiusTR = radiusTL;
            radiusBR = radiusTL;
            radiusBL = radiusTL;
        } else if (argc >= 8) {
            // Separate radius values
            radiusTL = argv[6].toNumber();
            radiusTR = argv[7].toNumber();
            if (argc >= 9)
                radiusBR = argv[8].toNumber();
            if (argc >= 10)
                radiusBL = argv[9].toNumber();
        }

        if (!qt_is_finite(x)
            || !qt_is_finite(y)
            || !qt_is_finite(w)
            || !qt_is_finite(h)
            || !qt_is_finite(blur)
            || !qt_is_finite(radiusTL)
            || !qt_is_finite(radiusTR)
            || !qt_is_finite(radiusBR)
            || !qt_is_finite(radiusBL)) {
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createBoxShadow(): Incorrect arguments")
        }

        // TODO: Consider adding methods for QCanvas2DShadowObject
        QV4::Scoped<QCanvas2DShadowObject> shadow(scope, scope.engine->memoryManager->allocate<QCanvas2DShadowObject>());

        auto b = new QCanvasBoxShadow(x, y, w, h, 0, blur, color);
        b->setTopLeftRadius(radiusTL);
        b->setTopRightRadius(radiusTR);
        b->setBottomRightRadius(radiusBR);
        b->setBottomLeftRadius(radiusBL);
        shadow->d()->brush = b;
        RETURN_RESULT(*shadow);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createGridPattern(real x, real y, real width, real height,
                                                       string lineColor, string backgroundColor,
                                                       real lineWidth, real feather, real angle)

    Returns a Canvas2DGrid object that covers rectangle area (\a x, \a y, \a width, \a height).
    The grid uses \a lineColor for lines and \a backgroundColor for the background.
    The line width is \a lineWidth, line feather \a feather and rotation angle \a angle in radians.
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createGridPattern(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 4) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();

        QColor lineColor = QColorConstants::White;
        QColor backgroundColor = QColorConstants::Black;
        if (argc >= 5) {
            if (argv[4].as<Object>()) {
                lineColor = QV4::ExecutionEngine::toVariant(
                                argv[4], QMetaType::fromType<QColor>()).value<QColor>();
            } else {
                lineColor = QCanvas2DUtils::qColorFromString(argv[4]);
            }
        }
        if (argc >= 6) {
            if (argv[5].as<Object>()) {
                backgroundColor = QV4::ExecutionEngine::toVariant(
                                    argv[5], QMetaType::fromType<QColor>()).value<QColor>();
            } else {
                backgroundColor = QCanvas2DUtils::qColorFromString(argv[5]);
            }
        }

        qreal lineWidth = (argc >= 7) ? argv[6].toNumber() : 1;
        qreal feather = (argc >= 8) ? argv[7].toNumber() : 1;
        qreal angle = (argc >= 9) ? argv[8].toNumber() : 0;

        if (!qt_is_finite(x)
            || !qt_is_finite(y)
            || !qt_is_finite(w)
            || !qt_is_finite(h)
            || !qt_is_finite(lineWidth)
            || !qt_is_finite(feather)
            || !qt_is_finite(angle)) {
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createGridPattern(): Incorrect arguments")
        }

        // TODO: Consider adding methods for QCanvas2DGridObject
        QV4::Scoped<QCanvas2DGridObject> grid(scope, scope.engine->memoryManager->allocate<QCanvas2DGridObject>());

        auto b = new QCanvasGridPattern(x, y, w, h, lineColor, backgroundColor, lineWidth, feather, angle);
        grid->d()->brush = b;
        RETURN_RESULT(*grid);
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createPath2D()

    Returns a new Path2D object. Calling this is equal to HTML canvas "new Path2D()" command.
    \sa fill(), stroke()
  */
/*!
    \qmlmethod object Canvas2DContext::createPath2D(Path2D path)

    Returns a new Path2D object, with the copy of \a path.
    Calling this is equal to HTML canvas "new Path2D(path)" command.
    \sa fill(), stroke()
  */
/*!
    \qmlmethod object Canvas2DContext::createPath2D(string svgPath)

    Returns a new Path2D object, with the content of \a svgPath.
    Calling this is equal to HTML canvas "new Path2D(d)" command.
    \sa fill(), stroke()
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createPath2D(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QCanvas2DContextEngineData *ed = engineData(scope.engine);
    QV4::Scoped<QCanvas2DPath2DObject> path(scope, scope.engine->memoryManager->allocate<QCanvas2DPath2DObject>());
    QV4::ScopedObject p(scope, ed->pathPrototype.value());
    path->setPrototypeOf(p);

    if (argc > 0) {
        QV4::ScopedValue arg1(scope, argv[0]);
        if (arg1->isString()) {
            // svg string as an parameter.
            QString svgPath = arg1->toQString();
            QTransform transform;
            if (argc >= 2) {
                QV4::ScopedValue transformValue(scope, argv[1]);
                if (transformValue->as<Object>())
                    transform = QV4::ExecutionEngine::toVariant(transformValue, QMetaType::fromType<QTransform>()).value<QTransform>();
            }
            auto p = new QCanvasPath();
            p->addPath(svgPath, transform);
            path->d()->path = p;
        } else if (arg1->isObject()) {
            // Path2D as an parameter.
            QV4::Scoped<QCanvas2DPath2DObject> sourcePath(scope, arg1);
            if (!!sourcePath)
                path->d()->path = new QCanvasPath(*sourcePath->d()->path);
        }
    }

    if (!path->d()->path)
        path->d()->path = new QCanvasPath();

    RETURN_RESULT(*path);
}

/*!
    \qmlmethod object Canvas2DContext::createTransform2D()

    Returns a new transform2d object, initialized to identity matrix.

    \sa getTransform()
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createTransform2D(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QTransform t;
    RETURN_RESULT(scope.engine->fromVariant(t));
}

/*!
    \qmlmethod object Canvas2DContext::drawBoxShadow(shadow)

    Draws a given box \a shadow. The shadow will be painted with the
    position, size, color, blur etc. set in the \a shadow.
    Calling beginPath() before this method is not required.

    \sa createBoxShadow()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_drawBoxShadow(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{

    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());
    QV4::Scoped<QCanvas2DShadowObject> shadow(scope, value);
    if (!!shadow) {
        auto b = shadow->d()->brush;
        if (b->type() == QCanvasBrush::BrushType::BoxShadow) {
            auto s = static_cast<QCanvasBoxShadow *>(b);
            r->d()->context()->drawBoxShadow(s);
        }
    }
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createPattern(Image image, string repetition)
    Returns a CanvasPattern object that uses the given image and repeats in the
    direction(s) given by the repetition argument.

    The \a image parameter must be a valid Image item
    or loaded image url. If there is no image data, this function throws an
    INVALID_STATE_ERR exception.

    The allowed values for \a repetition are:

    \value "repeat"    both directions
    \value "repeat-x   horizontal only
    \value "repeat-y"  vertical only
    \value "no-repeat" neither

    If the repetition argument is empty or null, the value "repeat" is used.

    \sa strokeStyle
    \sa fillStyle
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createPattern(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)
    if (argc >= 2) {
        // TODO: Consider adding own object type for image patterns than QCanvas2DGradientObject,
        // so that these don't have addColorStop methods.
        QV4::Scoped<QCanvas2DGradientObject> pattern(scope, scope.engine->memoryManager->allocate<QCanvas2DGradientObject>());
        QImage patternTexture;
        patternTexture = r->d()->context()->createPixmap(QUrl(argv[0].toQStringNoThrow()))->image();

        if (!patternTexture.isNull()) {
            r->d()->context()->buffer()->addImage(patternTexture);
            pattern->d()->brush = new QCanvasImagePattern();

            QString repetition = argv[1].toQStringNoThrow();
            if (repetition == QStringLiteral("repeat") || repetition.isEmpty()) {
                pattern->d()->patternRepeatX = true;
                pattern->d()->patternRepeatY = true;
            } else if (repetition == QStringLiteral("repeat-x")) {
                pattern->d()->patternRepeatX = true;
                pattern->d()->patternRepeatY = false;
            } else if (repetition == QStringLiteral("repeat-y")) {
                pattern->d()->patternRepeatX = false;
                pattern->d()->patternRepeatY = true;
            } else if (repetition == QStringLiteral("no-repeat")) {
                pattern->d()->patternRepeatX = false;
                pattern->d()->patternRepeatY = false;
            } else {
                //TODO: exception: SYNTAX_ERR
            }
        }

        RETURN_RESULT(*pattern);
    }
    RETURN_UNDEFINED();
}

// ***** line styles *****

/*!
    \qmlproperty string Canvas2DContext::lineCap
    Holds the current line cap style.
    The possible line cap styles are:

    \value "butt"
        (default) QCanvasPainter::LineCap::Butt the end of each line has a flat edge
        perpendicular to the direction of the line.
    \value "round"
        QCanvasPainter::LineCap::Round a semi-circle with the diameter equal to the width of the
        line is added on to the end of the line.
    \value "square"
        QCanvasPainter::LineCap::Square a rectangle with the length of the line width and the
        width of half the line width, placed flat against the edge
        perpendicular to the direction of the line.

    Other values are ignored.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_lineCap(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    switch (r->d()->context()->state.lineCap) {
    case QCanvasPainter::LineCap::Round:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("round")));
    case QCanvasPainter::LineCap::Square:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("square")));
    case QCanvasPainter::LineCap::Butt:
    default:
        break;
    }
    RETURN_RESULT(scope.engine->newString(QStringLiteral("butt")));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_lineCap(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    if (!argc)
        return QV4::Encode::undefined();

    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    QString lineCap = argv[0].toQString();
    QCanvasPainter::LineCap cap;
    if (lineCap == QStringLiteral("round"))
        cap = QCanvasPainter::LineCap::Round;
    else if (lineCap == QStringLiteral("butt"))
        cap = QCanvasPainter::LineCap::Butt;
    else if (lineCap == QStringLiteral("square"))
        cap = QCanvasPainter::LineCap::Square;
    else
        RETURN_UNDEFINED();

    // Always set the cap
    r->d()->context()->state.lineCap = cap;
    r->d()->context()->buffer()->setLineCap(cap);

    RETURN_UNDEFINED();
}

/*!
    \qmlproperty string Canvas2DContext::lineJoin
    Holds the current line join style. A join exists at any point in a subpath
    shared by two consecutive lines. When a subpath is closed, then a join also
    exists at its first point (equivalent to its last point) connecting the
    first and last lines in the subpath.

    The possible line join styles are:

    \value "bevel"  QCanvasPainter::LineJoin::Bevel The triangular notch between the two lines is filled.
    \value "round"  QCanvasPainter::LineJoin::Round A circular arc between the two lines is filled.
    \value "miter"  (default) QCanvasPainter::LineJoin::Miter The outer edges of the lines are extended to
                    meet at an angle, and this area is filled.

    Other values are ignored.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_lineJoin(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    switch (r->d()->context()->state.lineJoin) {
    case QCanvasPainter::LineJoin::Round:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("round")));
    case QCanvasPainter::LineJoin::Bevel:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("bevel")));
    case QCanvasPainter::LineJoin::Miter:
    default:
        break;
    }
    RETURN_RESULT(scope.engine->newString(QStringLiteral("miter")));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_lineJoin(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (!argc)
        THROW_TYPE_ERROR();

    QString lineJoin = argv[0].toQString();
    QCanvasPainter::LineJoin join;
    if (lineJoin == QStringLiteral("round"))
        join = QCanvasPainter::LineJoin::Round;
    else if (lineJoin == QStringLiteral("bevel"))
        join = QCanvasPainter::LineJoin::Bevel;
    else if (lineJoin == QStringLiteral("miter"))
        join = QCanvasPainter::LineJoin::Miter;
    else
        RETURN_UNDEFINED();

    // Always set the join
    r->d()->context()->state.lineJoin = join;
    r->d()->context()->buffer()->setLineJoin(join);

    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::lineWidth
     Holds the current line width. Values that are not finite values greater than zero are ignored.
 */
QV4::ReturnedValue QCanvasJSContext2D::method_get_lineWidth(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.lineWidth));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_lineWidth(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    qreal w = argc ? argv[0].toNumber() : -1;

    if (w > 0 && qt_is_finite(w) && w != r->d()->context()->state.lineWidth) {
        r->d()->context()->state.lineWidth = w;
        r->d()->context()->buffer()->setLineWidth(w);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::miterLimit
     Holds the current miter limit ratio.
     The default miter limit value is 10.0.
 */
QV4::ReturnedValue QCanvasJSContext2D::method_get_miterLimit(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.miterLimit));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_miterLimit(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    qreal ml = argc ? argv[0].toNumber() : -1;

    if (ml > 0 && qt_is_finite(ml) && ml != r->d()->context()->state.miterLimit) {
        r->d()->context()->state.miterLimit = ml;
        r->d()->context()->buffer()->setMiterLimit(ml);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::antialias
    Holds the current antialias amount. Values that are not finite values greater than zero are ignored.
    The default antialias value is 1.0.
 */
QV4::ReturnedValue QCanvasJSContext2D::method_get_antialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.antialias));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_antialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    qreal w = argc ? argv[0].toNumber() : -1;

    if (w > 0 && qt_is_finite(w) && w != r->d()->context()->state.antialias) {
        r->d()->context()->state.antialias = w;
        r->d()->context()->buffer()->setAntialias(w);
    }
    RETURN_UNDEFINED();
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_getLineDash(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "getLineDash(): Method not supported");

    RETURN_RESULT(QV4::Encode::null());
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_setLineDash(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);
    Q_UNUSED(argv);
    Q_UNUSED(argc);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "setLineDash(): Method not supported");

    RETURN_UNDEFINED();
}

QV4::ReturnedValue QCanvasJSContext2D::method_get_lineDashOffset(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "getLineDashOffset(): Method not supported");

    RETURN_RESULT(QV4::Encode::null());
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_lineDashOffset(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);
    Q_UNUSED(argv);
    Q_UNUSED(argc);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "setLineDashOffset(): Method not supported");

    RETURN_UNDEFINED();
}

// ***** direct rect methods *****

/*!
    \qmlmethod object Canvas2DContext::clearRect(real x, real y, real w, real h)

    Clears all pixels on the canvas in the rectangle specified by
    (\a x, \a y, \a w, \a h) to transparent black.
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_clearRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)


    if (argc >= 4)
        r->d()->context()->clearRect(argv[0].toNumber(),
                                     argv[1].toNumber(),
                                     argv[2].toNumber(),
                                     argv[3].toNumber());

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::fillRect(real x, real y, real w, real h)

    Paints a rectangular area specified by (\a x, \a y, \a w, \a h) using fillStyle.

   \sa fillStyle
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_fillRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4)
        r->d()->context()->fillRect(argv[0].toNumber(), argv[1].toNumber(), argv[2].toNumber(), argv[3].toNumber());
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::strokeRect(real x, real y, real w, real h)

    Strokes the path of the rectangle specified by (\a x, \a y, \a w, \a h) using
    strokeStyle, lineWidth, lineJoin, and (if appropriate) miterLimit attributes.

   \sa strokeStyle, lineWidth, lineJoin, miterLimit
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_strokeRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4)
        r->d()->context()->strokeRect(argv[0].toNumber(), argv[1].toNumber(), argv[2].toNumber(), argv[3].toNumber());

    RETURN_RESULT(*thisObject);
}

// ***** path handling *****

/*!
  \qmlmethod object Canvas2DContext::beginPath()

   Resets the current path to a new path.
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_beginPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    r->d()->context()->beginPath();

    RETURN_RESULT(*thisObject);
}

// TODO: Document if supported
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_clip(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "clip(): Method not supported");
    r->d()->context()->clip();
    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::clipRect(real x, real y, real width, real height)

   Creates the clipping region from the rect \a x, \a y, \a width, \a height.
   Any parts of the shape outside the clipping path are not displayed.
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_clipRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 4)
        r->d()->context()->clipRect(argv[0].toNumber(), argv[1].toNumber(), argv[2].toNumber(), argv[3].toNumber());

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::resetClipping()

  Resets and disables clipping.
  \sa clipRect
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_resetClipping(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    r->d()->context()->resetClipping();

    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::fill()

   Fills the subpaths with the current fill style.

   \sa fillStyle, {http://www.w3.org/TR/2dcontext/#dom-context-2d-fill}{W3C 2d context standard for fill}
  */
/*!
  \qmlmethod object Canvas2DContext::fill(Canvas2DPath2D path)

   Fills the \a path with the current fill style.

   \sa fillStyle, Canvas2DPath2D, {http://www.w3.org/TR/2dcontext/#dom-context-2d-fill}{W3C 2d context standard for fill}
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_fill(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r);
    if (argc == 0) {
        r->d()->context()->fill();
    } else {
        // fill a path
        QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());
        QV4::Scoped<QCanvas2DPath2DObject> path(scope, value);
        if (!!path) {
            const QCanvasPath &p = *path->d()->path;
            if (argc >= 2) {
                int pathGroup = argv[1].toInteger();
                r->d()->context()->fillPath(p, pathGroup);
            } else {
                r->d()->context()->fillPath(p);
            }
        }
    }
    RETURN_RESULT(*thisObject);
}

/*!
  \qmlmethod object Canvas2DContext::stroke()

   Strokes the subpaths with the current stroke style.

   \sa strokeStyle, {http://www.w3.org/TR/2dcontext/#dom-context-2d-stroke}{W3C 2d context standard for stroke}
 */
/*!
  \qmlmethod object Canvas2DContext::stroke(Canvas2DPath2D path)

   Strokes the \a path with the current stroke style.

   \sa strokeStyle, Canvas2DPath2D, {http://www.w3.org/TR/2dcontext/#dom-context-2d-stroke}{W3C 2d context standard for stroke}
 */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_stroke(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)
    if (argc == 0) {
        r->d()->context()->stroke();
    } else {
        // stroke a path
        QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());
        QV4::Scoped<QCanvas2DPath2DObject> path(scope, value);
        if (!!path) {
            const QCanvasPath &p = *path->d()->path;
            if (argc >= 2) {
                int pathGroup = argv[1].toInteger();
                r->d()->context()->strokePath(p, pathGroup);
            } else {
                r->d()->context()->strokePath(p);
            }
        }
    }
    RETURN_RESULT(*thisObject);
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_isPointInPath(const QV4::FunctionObject *b, const QV4::Value *, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "Canvas2DContext::isPointInPath is not supported");
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_drawFocusRing(const QV4::FunctionObject *b, const QV4::Value *, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "Canvas2DContext::drawFocusRing is not supported");
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_setCaretSelectionRect(const QV4::FunctionObject *b, const QV4::Value *, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "Context2D::setCaretSelectionRect is not supported");
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_caretBlinkRate(const QV4::FunctionObject *b, const QV4::Value *, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "Context2D::caretBlinkRate is not supported");
}

/*!
    \qmlproperty string Canvas2DContext::font
    Holds the current font settings.

    A subset of the
    \l {http://www.w3.org/TR/2dcontext/#dom-context-2d-font}{w3C 2d context standard for font}
    is supported:

    \list
        \li font-style (optional):
        normal | italic | oblique
        \li font-variant (optional): normal | small-caps
        \li font-weight (optional): normal | bold | 1 ... 1000
        \li font-size: Npx | Npt (where N is a positive number)
        \li font-family: See \l {http://www.w3.org/TR/CSS2/fonts.html#propdef-font-family}
    \endlist

    \note The font-size and font-family properties are mandatory and must be in
    the order they are shown in above. In addition, a font family with spaces in
    its name must be quoted.

    The default font value is "10px sans-serif".
  */
QV4::ReturnedValue QCanvasJSContext2D::method_get_font(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    RETURN_RESULT(scope.engine->newString(r->d()->context()->state.font.toString()));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_font(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    QV4::ScopedString s(scope, argc ? argv[0] : QV4::Value::undefinedValue(), QV4::ScopedString::Convert);
    if (scope.hasException())
        RETURN_UNDEFINED();
    QFont font = QCanvas2DUtils::qFontFromString(s->toQString(), r->d()->context()->state.font);
    if (font != r->d()->context()->state.font) {
        r->d()->context()->state.font = font;
        r->d()->context()->buffer()->setFont(font);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty string Canvas2DContext::textAlign

    Holds the current text alignment settings. The possible values are:

    \value "start"  (default) QCanvasPainter::TextAlign::Start Align to the start edge of the text (left side in
                    left-to-right text, right side in right-to-left text).
    \value "end"    QCanvasPainter::TextAlign::End Align to the end edge of the text (right side in left-to-right
                    text, left side in right-to-left text).
    \value "left"   QCanvasPainter::TextAlign::Left
    \value "right"  QCanvasPainter::TextAlign::Right
    \value "center" QCanvasPainter::TextAlign::Center

    Other values are ignored.
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_textAlign(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    switch (r->d()->context()->state.textAlign) {
    case QCanvasPainter::TextAlign::End:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("end")));
    case QCanvasPainter::TextAlign::Left:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("left")));
    case QCanvasPainter::TextAlign::Right:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("right")));
    case QCanvasPainter::TextAlign::Center:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("center")));
    case QCanvasPainter::TextAlign::Start:
    default:
        break;
    }
    RETURN_RESULT(scope.engine->newString(QStringLiteral("start")));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_textAlign(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    QV4::ScopedString s(scope, argc ? argv[0] : QV4::Value::undefinedValue(), QV4::ScopedString::Convert);
    if (scope.hasException())
        RETURN_UNDEFINED();
    QString textAlign = s->toQString();

    QCanvasPainter::TextAlign ta;
    if (textAlign == QStringLiteral("start"))
        ta = QCanvasPainter::TextAlign::Start;
    else if (textAlign == QStringLiteral("end"))
        ta = QCanvasPainter::TextAlign::End;
    else if (textAlign == QStringLiteral("left"))
        ta = QCanvasPainter::TextAlign::Left;
    else if (textAlign == QStringLiteral("right"))
        ta = QCanvasPainter::TextAlign::Right;
    else if (textAlign == QStringLiteral("center"))
        ta = QCanvasPainter::TextAlign::Center;
    else
        RETURN_UNDEFINED();

    r->d()->context()->buffer()->setTextAlign(ta);

    if (ta != r->d()->context()->state.textAlign)
        r->d()->context()->state.textAlign = ta;

    RETURN_UNDEFINED();
}

/*!
    \qmlproperty string Canvas2DContext::textBaseline

    Holds the current baseline alignment settings. The possible values are:

    \value "top"            QCanvasPainter::TextBaseline::Top The top of the em square
    \value "hanging"        QCanvasPainter::TextBaseline::Hanging The hanging baseline
    \value "middle"         QCanvasPainter::TextBaseline::Middle The middle of the em square
    \value "alphabetic"     (default) QCanvasPainter::TextBaseline::Alphabetic The alphabetic baseline
    \value "bottom"         QCanvasPainter::TextBaseline::Bottom The bottom of the em square

    Other values are ignored. The default value is "alphabetic".
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_textBaseline(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    switch (r->d()->context()->state.textBaseline) {
    case QCanvasPainter::TextBaseline::Hanging:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("hanging")));
    case QCanvasPainter::TextBaseline::Top:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("top")));
    case QCanvasPainter::TextBaseline::Bottom:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("bottom")));
    case QCanvasPainter::TextBaseline::Middle:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("middle")));
    case QCanvasPainter::TextBaseline::Alphabetic:
    default:
        break;
    }
    RETURN_RESULT(scope.engine->newString(QStringLiteral("alphabetic")));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_textBaseline(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)
    QV4::ScopedString s(scope, argc ? argv[0] : QV4::Value::undefinedValue(), QV4::ScopedString::Convert);
    if (scope.hasException())
        RETURN_UNDEFINED();
    QString textBaseline = s->toQString();

    QCanvasPainter::TextBaseline tb;
    if (textBaseline == QStringLiteral("alphabetic"))
        tb = QCanvasPainter::TextBaseline::Alphabetic;
    else if (textBaseline == QStringLiteral("hanging"))
        tb = QCanvasPainter::TextBaseline::Hanging;
    else if (textBaseline == QStringLiteral("top"))
        tb = QCanvasPainter::TextBaseline::Top;
    else if (textBaseline == QStringLiteral("bottom"))
        tb = QCanvasPainter::TextBaseline::Bottom;
    else if (textBaseline == QStringLiteral("middle"))
        tb = QCanvasPainter::TextBaseline::Middle;
    else
        RETURN_UNDEFINED();

    r->d()->context()->buffer()->setTextBaseline(tb);

    if (tb != r->d()->context()->state.textBaseline)
        r->d()->context()->state.textBaseline = tb;

    RETURN_UNDEFINED();
}

/*!
  \qmlmethod object Canvas2DContext::fillText(text, x, y)

  Fills the specified \a text at the given position (\a x, \a y).

  \sa font
  \sa textAlign
  \sa textBaseline
  \sa strokeText
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_fillText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 3) {
        qreal x = argv[1].toNumber();
        qreal y = argv[2].toNumber();
        if (!qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        // Draw texts with native methods rather than
        // using the common path filling.
        r->d()->context()->buffer()->drawText(argv[0].toQStringNoThrow(), x, y);

        // TODO: Do we want to have alternative to use filled
        // path for glyphs? Currently used only for stroking.
        //QPainterPath textPath = r->d()->context()->createTextGlyphs(x, y, argv[0].toQStringNoThrow());
        //r->d()->context()->buffer()->fill(textPath);
    }

    RETURN_RESULT(*thisObject);
}
/*!
    \qmlmethod object Canvas2DContext::strokeText(text, x, y)

    Strokes the given \a text at a position specified by (\a x, \a y).

    \sa font
    \sa textAlign
    \sa textBaseline
    \sa fillText
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_strokeText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 3)
        r->d()->context()->drawText(argv[0].toQStringNoThrow(), argv[1].toNumber(), argv[2].toNumber(), false);

    RETURN_RESULT(*thisObject);
}

// TODO: Should this be extended to cover the HTML canvas measureText(),
// so return full TextMetrics object and not just width?

/*!
  \qmlmethod object Canvas2DContext::measureText(text)

  Returns an object with a \c width property, whose value is equivalent to
  calling QFontMetrics::horizontalAdvance() with the given \a text in the
  current font.
  */
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_measureText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 1) {
        QFontMetrics fm(r->d()->context()->state.font);
        uint width = fm.horizontalAdvance(argv[0].toQStringNoThrow());
        QV4::ScopedObject tm(scope, scope.engine->newObject());
        tm->put(QV4::ScopedString(scope, scope.engine->newIdentifier(QStringLiteral("width"))).getPointer(),
                QV4::ScopedValue(scope, QV4::Value::fromDouble(width)));
        RETURN_RESULT(*tm);
    }
    RETURN_UNDEFINED();
}

// ***** drawing images *****

/*!
  \qmlmethod object Canvas2DContext::drawImage(variant image, real dx, real dy)
  Draws the given \a image on the canvas at position (\a dx, \a dy).
  Note:
  The \a image type can be an Image item or an image url.
  When given as Image item, if the image isn't fully loaded, this method draws nothing.
  When given as url string, the image should be loaded by calling Canvas item's Canvas2D::loadImage() method first.
  This image been drawing is subject to the current context clip path.

  \sa Image
  \sa Canvas2D::loadImage
  \sa Canvas2D::isImageLoaded
  \sa Canvas2D::imageLoaded

  \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-drawimage}{W3C 2d context standard for drawImage}
  */

/*!
  \qmlmethod object Canvas2DContext::drawImage(variant image, real dx, real dy, real dw, real dh)
  This is an overloaded function.
  Draws the given item as \a image onto the canvas at point (\a dx, \a dy) and with width \a dw,
  height \a dh.

  Note:
  The \a image type can be an Image item or an image url.
  When given as Image item, if the image isn't fully loaded, this method draws nothing.
  When given as url string, the image should be loaded by calling Canvas item's Canvas2D::loadImage() method first.
  This image been drawing is subject to the current context clip path.

  \sa Image
  \sa Canvas2D::loadImage()
  \sa Canvas2D::isImageLoaded
  \sa Canvas2D::imageLoaded

  \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-drawimage}{W3C 2d context standard for drawImage}
  */
/*!
  \qmlmethod object Canvas2DContext::drawImage(variant image, real sx, real sy, real sw, real sh, real dx, real dy, real dw, real dh)
  This is an overloaded function.
  Draws the given item as \a image from source point (\a sx, \a sy) and source width \a sw, source height \a sh
  onto the canvas at point (\a dx, \a dy) and with width \a dw, height \a dh.

  Note:
  The \a image type can be an Image or an image url.
  When given as Image item, if the image isn't fully loaded, this method draws nothing.
  When given as url string, the image should be loaded by calling Canvas item's Canvas2D::loadImage() method first.
  This image been drawing is subject to the current context clip path.

  \sa Image
  \sa Canvas2D::loadImage()
  \sa Canvas2D::isImageLoaded
  \sa Canvas2D::imageLoaded

  \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-drawimage}{W3C 2d context standard for drawImage}
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_drawImage(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    qreal sx, sy, sw, sh, dx, dy, dw, dh;

    if (!argc)
        RETURN_UNDEFINED();

    QQmlRefPointer<QCanvas2DPixmap> pixmap;

    QV4::ScopedValue arg(scope, argv[0]);
    QString filename;
    if (arg->isString()) {
        filename = arg->toQString();
        QUrl url(filename);
        if (!url.isValid())
            THROW_DOM(DOMEXCEPTION_TYPE_MISMATCH_ERR, "drawImage(), type mismatch");

        pixmap = r->d()->context()->createPixmap(url);
    } else if (arg->isObject()) {
        QV4::Scoped<QV4::QObjectWrapper> qobjectWrapper(scope, arg);
        if (!!qobjectWrapper) {
            if (QQuickImage *imageItem = qobject_cast<QQuickImage*>(qobjectWrapper->object())) {
                pixmap = r->d()->context()->createPixmap(imageItem->source());
            } else if (QCanvas2DItem *canvas = qobject_cast<QCanvas2DItem*>(qobjectWrapper->object())) {
                Q_UNUSED(canvas);
                THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "drawImage(): Canvas2D not supported");
            } else {
                THROW_DOM(DOMEXCEPTION_TYPE_MISMATCH_ERR, "drawImage(), type mismatch");
            }
        } else {
            QUrl url(arg->toQStringNoThrow());
            if (url.isValid())
                pixmap = r->d()->context()->createPixmap(url);
            else
                THROW_DOM(DOMEXCEPTION_TYPE_MISMATCH_ERR, "drawImage(), type mismatch");
        }
    } else {
        THROW_DOM(DOMEXCEPTION_TYPE_MISMATCH_ERR, "drawImage(), type mismatch");
    }

    if (pixmap.isNull() || !pixmap->isValid())
        RETURN_UNDEFINED();

    if (argc >= 9) {
        sx = argv[1].toNumber();
        sy = argv[2].toNumber();
        sw = argv[3].toNumber();
        sh = argv[4].toNumber();
        dx = argv[5].toNumber();
        dy = argv[6].toNumber();
        dw = argv[7].toNumber();
        dh = argv[8].toNumber();
    } else if (argc >= 5) {
        sx = 0;
        sy = 0;
        sw = pixmap->width();
        sh = pixmap->height();
        dx = argv[1].toNumber();
        dy = argv[2].toNumber();
        dw = argv[3].toNumber();
        dh = argv[4].toNumber();
    } else if (argc >= 3) {
        dx = argv[1].toNumber();
        dy = argv[2].toNumber();
        sx = 0;
        sy = 0;
        sw = pixmap->width();
        sh = pixmap->height();
        dw = sw;
        dh = sh;
    } else {
        RETURN_UNDEFINED();
    }

    if (!qt_is_finite(sx)
        || !qt_is_finite(sy)
        || !qt_is_finite(sw)
        || !qt_is_finite(sh)
        || !qt_is_finite(dx)
        || !qt_is_finite(dy)
        || !qt_is_finite(dw)
        || !qt_is_finite(dh))
        RETURN_UNDEFINED();

    if (sx < 0
        || sy < 0
        || sw == 0
        || sh == 0
        || sx + sw > pixmap->width()
        || sy + sh > pixmap->height()
        || sx + sw < 0 || sy + sh < 0) {
        THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "drawImage(), index size error");
    }

    r->d()->context()->buffer()->drawPixmap(pixmap, filename, QRectF(sx, sy, sw, sh), QRectF(dx, dy, dw, dh));

    RETURN_RESULT(*thisObject);
}

// ***** pixel manipulation *****

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createImageData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);
    Q_UNUSED(argv);
    Q_UNUSED(argc);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "createImageData(): Method not supported");

    RETURN_UNDEFINED();
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_getImageData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);
    Q_UNUSED(argv);
    Q_UNUSED(argc);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "getImageData(): Method not supported");

    RETURN_RESULT(QV4::Encode::null());
}

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_putImageData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);
    Q_UNUSED(argv);
    Q_UNUSED(argc);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "putImageData(): Method not supported");

    RETURN_UNDEFINED();
}

/*!
    \qmltype Canvas2DGradient
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides an opaque Canvas2DGradient interface.
  */

/*!
    \qmlmethod object Canvas2DGradient::addColorStop(real offset, string color)

    Adds a color stop with the given \a color to the gradient at the given \a offset.
    0.0 is the offset at one end of the gradient, 1.0 is the offset at the other end.

    For example:

    \code
    const gradient = ctx.createLinearGradient(0, 0, 100, 100);
    gradient.addColorStop(0.0, Qt.rgba(1, 0, 0, 1));
    gradient.addColorStop(1.0, 'rgba(0, 255, 255, 1)');
    \endcode
  */
QV4::ReturnedValue QCanvas2DGradientObject::gradient_proto_addColorStop(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DGradientObject> style(scope, thisObject->as<QCanvas2DGradientObject>());
    if (!style)
        THROW_GENERIC_ERROR("Not a Canvas2DGradient object");

    if (argc == 2) {

        auto type = style->d()->brush->type();
        if (type != QCanvasBrush::BrushType::LinearGradient &&
            type != QCanvasBrush::BrushType::RadialGradient &&
            type != QCanvasBrush::BrushType::ConicalGradient &&
            type != QCanvasBrush::BrushType::BoxGradient) {
            THROW_GENERIC_ERROR("Not a valid Canvas2DGradient object, can't get the gradient information");
        }
        QCanvasGradient *gradient = static_cast<QCanvasGradient *>(style->d()->brush);
        qreal pos = argv[0].toNumber();
        QColor color;

        if (argv[1].as<Object>()) {
            color = QV4::ExecutionEngine::toVariant(
                            argv[1], QMetaType::fromType<QColor>()).value<QColor>();
        } else {
            color = QCanvas2DUtils::qColorFromString(argv[1]);
        }
        if (pos < 0.0 || pos > 1.0 || !qt_is_finite(pos)) {
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "Canvas2DGradient: parameter offset out of range");
        }

        if (color.isValid()) {
            gradient->setColorAt(pos, color);
        } else {
            THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Canvas2DGradient: parameter color is not a valid color string");
        }
    }

    return thisObject->asReturnedValue();
}

// ***** Path2D *****

/*!
    \qmltype Canvas2DPath2D
    \inqmlmodule QtCanvas2D
    \since 6.12
    \brief Provides Path2D object, matching to QCanvasPath.
  */

/*!
    \qmlmethod object Canvas2DPath2D::addPath(var path)

    Adds a \a path into this path.
  */
/*!
    \qmlmethod object Canvas2DPath2D::addPath(var path, transform matrix)

    Adds a \a path into this path, using \a matrix as a transformation matrix.
  */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_addPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)
    if (argc >= 1) {
        // Path2D as an parameter.
        QV4::ScopedValue value(scope, argv[0]);
        QV4::Scoped<QCanvas2DPath2DObject> sourcePath(scope, value);
        if (!!sourcePath) {
            QTransform transform;
            if (argc >= 2) {
                QV4::ScopedValue transformValue(scope, argv[1]);
                if (transformValue->as<Object>()) {
                    transform = QV4::ExecutionEngine::toVariant(transformValue,
                                                                QMetaType::fromType<QTransform>()).value<QTransform>();
                }
            }
            r->d()->path->addPath(*sourcePath->d()->path, transform);
        }
    }
    return thisObject->asReturnedValue();
}

/*!
  \qmlmethod object Canvas2DPath2D::closePath()
   Closes the current subpath by drawing a line to the beginning of the subpath, automatically starting a new path.
   The current point of the new path is the previous subpath's first point.
  */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_closePath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    r->d()->path->closePath();
    return thisObject->asReturnedValue();
}

/*!
  \qmlmethod object Canvas2DPath2D::moveTo(real x, real y)

   Creates a new subpath with a point at (\a x, \a y).
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_moveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 2) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        r->d()->path->moveTo(x, y);
    }
    return thisObject->asReturnedValue();
}
/*!
  \qmlmethod object Canvas2DPath2D::lineTo(real x, real y)

   Draws a line from the current position to the point at (\a x, \a y).
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_lineTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 2) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        r->d()->path->lineTo(x, y);
    }

    return thisObject->asReturnedValue();
}

/*!
  \qmlmethod object Canvas2DPath2D::bezierCurveTo(real cp1x, real cp1y, real cp2x, real cp2y, real x, real y)

  Adds a cubic bezier curve between the current position and the given endPoint using the control points specified by (\a {cp1x}, \a {cp1y}),
  and (\a {cp2x}, \a {cp2y}).
  After the curve is added, the current position is updated to be at the end point (\a {x}, \a {y}) of the curve.
  */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_bezierCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 6) {
        qreal cp1x = argv[0].toNumber();
        qreal cp1y = argv[1].toNumber();
        qreal cp2x = argv[2].toNumber();
        qreal cp2y = argv[3].toNumber();
        qreal x = argv[4].toNumber();
        qreal y = argv[5].toNumber();

        if (!qt_is_finite(cp1x) || !qt_is_finite(cp1y) ||
            !qt_is_finite(cp2x) || !qt_is_finite(cp2y) ||
            !qt_is_finite(x) || !qt_is_finite(y)) {
            RETURN_UNDEFINED();
        }

        r->d()->path->bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::quadraticCurveTo(real cpx, real cpy, real x, real y)

    Adds a quadratic bezier curve between the current point and the endpoint
    (\a x, \a y) with the control point specified by (\a cpx, \a cpy).
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_quadraticCurveTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 4) {
        qreal cpx = argv[0].toNumber();
        qreal cpy = argv[1].toNumber();
        qreal x = argv[2].toNumber();
        qreal y = argv[3].toNumber();

        if (!qt_is_finite(cpx) || !qt_is_finite(cpy) || !qt_is_finite(x) || !qt_is_finite(y))
            RETURN_UNDEFINED();

        r->d()->path->quadraticCurveTo(cpx, cpy, x, y);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::arcTo(real x1, real y1, real x2,
        real y2, real radius)

    Adds an arc with starting point (\a x1, \a y1), ending point (\a x2, \a y2),
    and \a radius to the current subpath and connects it to the previous subpath
    by a straight line.

 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_arcTo(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 5) {
        qreal c1x = argv[0].toNumber();
        qreal c1y = argv[1].toNumber();
        qreal c2x = argv[2].toNumber();
        qreal c2y = argv[3].toNumber();
        qreal radius = argv[4].toNumber();

        if (!qt_is_finite(c1x) || !qt_is_finite(c1y) ||
            !qt_is_finite(c2x) || !qt_is_finite(c2y)) {
            RETURN_UNDEFINED();
        }

        if (qt_is_finite(radius) && radius < 0)
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "Incorrect argument radius");

        r->d()->path->arcTo(c1x, c1y, c2x, c2y, radius);
    }

    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::arc(real x, real y, real radius,
        real startAngle, real endAngle, bool anticlockwise)

    Adds an arc to the current subpath that lies on the circumference of the
    circle whose center is at the point (\a x, \a y) and whose radius is
    \a radius.

    Both \a startAngle and \a endAngle are measured from the x-axis in radians.

    The default curve direction is clockwise. To change direction to opposite,
    set \a anticlockwise to true.
*/
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_arc(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 5) {
        qreal centerX = argv[0].toNumber();
        qreal centerY = argv[1].toNumber();
        qreal radius = argv[2].toNumber();
        qreal a0 = argv[3].toNumber();
        qreal a1 = argv[4].toNumber();
        bool antiClockwise = (argc >= 6) ? argv[5].toBoolean() : false;

        if (!qt_is_finite(centerX) || !qt_is_finite(centerY) ||
            !qt_is_finite(a0) || !qt_is_finite(a1)) {
            RETURN_UNDEFINED();
        }

        if (qt_is_finite(radius) && radius < 0)
            THROW_DOM(DOMEXCEPTION_INDEX_SIZE_ERR, "Incorrect argument radius");

        auto direction = antiClockwise ? QCanvasPainter::PathWinding::CounterClockWise :
                QCanvasPainter::PathWinding::ClockWise;
        r->d()->path->arc(centerX, centerY, radius, a0, a1, direction);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::rect(real x, real y, real w, real h)

    Adds a rectangle at position (\a x, \a y), with the given width \a w and
    height \a h, as a closed subpath.
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_rect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 4) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
            RETURN_UNDEFINED();

        r->d()->path->rect(x, y, w, h);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::roundedRect(real x, real y, real w, real h, real xRadius, real yRadius)

    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a w, \a h), to the path.
    The \a xRadius and \a yRadius arguments specify the radius of the
    ellipses defining the corners of the rounded rectangle.
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_roundedRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 6) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal yr = argv[4].toNumber();
        qreal xr = argv[5].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h) ||
            !qt_is_finite(yr) || !qt_is_finite(xr)) {
            RETURN_UNDEFINED();
        }

        r->d()->path->roundRect(x, y, w, h, yr, yr, xr, xr);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::roundRect(real x, real y, real w, real h, real radius)

    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a w, \a h), to the path.
    The \a radius argument specify the radius of the
    ellipses defining the corners of the rounded rectangle.
 */

/*!
    \qmlmethod object Canvas2DPath2D::roundRect(real x, real y, real w, real h,
                   real radiusTopLeft, real radiusTopRight,
                   real radiusBottomRight, real radiusBottomLeft)
    Adds a rounded-corner rectangle, specified by (\a x, \a y, \a w, \a h), to the path.
    The \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight and \a radiusBottomLeft
    arguments specify the radius of the ellipses defining the corners of the rounded rectangle.
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_roundRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 8) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal rtl = argv[4].toNumber();
        qreal rtr = argv[5].toNumber();
        qreal rbr = argv[6].toNumber();
        qreal rbl = argv[7].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h) ||
            !qt_is_finite(rtl) || !qt_is_finite(rtr) || !qt_is_finite(rbr) || !qt_is_finite(rbl)) {
            RETURN_UNDEFINED();
        }

        r->d()->path->roundRect(x, y, w, h,
                                rtl, rtr,
                                rbr, rbl);
    } else if (argc >= 5) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();
        qreal rad = argv[4].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h) ||
            !qt_is_finite(rad)) {
            RETURN_UNDEFINED();
        }
        r->d()->path->roundRect(x, y, w, h, rad);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::ellipse(real centerX, real centerY, real radiusX, real radiusY)

    Creates new ellipse shaped sub-path into ( \a centerX, \a centerY) with
    \a radiusX and \a radiusY.

    The ellipse is composed of a clockwise curve, starting and finishing at
    zero degrees (the 3 o'clock position).
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_ellipse(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 4) {
        qreal cx = argv[0].toNumber();
        qreal cy = argv[1].toNumber();
        qreal rx = argv[2].toNumber();
        qreal ry = argv[3].toNumber();

        if (!qt_is_finite(cx) || !qt_is_finite(cy) ||
            !qt_is_finite(rx) || !qt_is_finite(ry)) {
            RETURN_UNDEFINED();
        }

        r->d()->path->ellipse(cx, cy, rx, ry);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::ellipseRect(real x, real y, real w, real h)

    Creates an ellipse within the bounding rectangle defined by its top-left
    corner at (\a x, \a y), width \a w and height \a h, and adds it to the
    path as a closed subpath.

    The ellipse is composed of a clockwise curve, starting and finishing at
    zero degrees (the 3 o'clock position).

    \note This method matches to \l QtQuick::Context2D::ellipse()
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_ellipseRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 4) {
        qreal x = argv[0].toNumber();
        qreal y = argv[1].toNumber();
        qreal w = argv[2].toNumber();
        qreal h = argv[3].toNumber();

        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
            RETURN_UNDEFINED();

        QRectF rect(x, y, w, h);
        r->d()->path->ellipse(rect);
    }
    return thisObject->asReturnedValue();
}

/*!
    \qmlmethod object Canvas2DPath2D::circle(real centerX, real centerY, real radius)

    Creates a circle defined by its center (\a centerX, \a centerY), and
    radius \a radius, and adds it to the path as a closed subpath.

    \note Compared to arc(), this method does not add a straight line from
    the last point in the subpath to the start point of the circle.
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_circle(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    if (argc >= 3) {
        qreal cx = argv[0].toNumber();
        qreal cy = argv[1].toNumber();
        qreal rad = argv[2].toNumber();

        if (!qt_is_finite(cx) || !qt_is_finite(cy) || !qt_is_finite(rad))
            RETURN_UNDEFINED();

        r->d()->path->circle(cx, cy, rad);
    }
    return thisObject->asReturnedValue();
}

/*!
  \qmlmethod object Canvas2DPath2D::beginSolidSubPath()

  Start a solid subpath.
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_beginSolidSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    r->d()->path->beginSolidSubPath();
    return thisObject->asReturnedValue();
}

/*!
  \qmlmethod object Canvas2DPath2D::beginHoleSubPath()

  Start a hole subpath.
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_beginHoleSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    r->d()->path->beginHoleSubPath();
    return thisObject->asReturnedValue();
}

/*!
  \qmlmethod bool Canvas2DPath2D::isEmpty()

  Returns true when the path does not contain any painting commands.
  \sa clear()
 */

QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_isEmpty(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    bool isEmpty = r->d()->path->isEmpty();
    RETURN_RESULT(QV4::Encode(isEmpty));
}

/*!
  \qmlmethod object Canvas2DPath2D::clear()

  Clears the path from all the painting commands.
  \sa isEmpty()
 */
QV4::ReturnedValue QCanvas2DPath2DObject::path_proto_clear(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvas2DPath2DObject> r(scope, thisObject->as<QCanvas2DPath2DObject>());
    CHECK_PATH_OBJECT(r)

    r->d()->path->clear();
    return thisObject->asReturnedValue();
}

// ***** transformations *****

void QCanvas2DContext::rotate(qreal angle)
{
    if (!qt_is_finite(angle))
        return;

    state.transform.rotateRadians(angle);
    buffer()->rotate(angle);
}

void QCanvas2DContext::scale(qreal x,  qreal y)
{
    if (!qt_is_finite(x) || !qt_is_finite(y))
        return;

    state.transform.scale(x, y);
    buffer()->scale(x, y);
}

void QCanvas2DContext::shear(qreal h, qreal v)
{
    if (!qt_is_finite(h) || !qt_is_finite(v))
        return ;

    state.transform.shear(h, v);
    buffer()->shear(h, v);
}

void QCanvas2DContext::translate(qreal x, qreal y)
{
    if (!qt_is_finite(x) || !qt_is_finite(y))
        return ;

    state.transform.translate(x, y);
    buffer()->translate(x, y);
}

void QCanvas2DContext::transform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
    if (!qt_is_finite(a) || !qt_is_finite(b) || !qt_is_finite(c) || !qt_is_finite(d) || !qt_is_finite(e) || !qt_is_finite(f))
        return;

    QTransform transform(a, b, c, d, e, f);
    state.transform *= transform;
    buffer()->transform(transform);
}

void QCanvas2DContext::setTransform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
    if (!qt_is_finite(a) || !qt_is_finite(b) || !qt_is_finite(c) || !qt_is_finite(d) || !qt_is_finite(e) || !qt_is_finite(f))
        return;

    QTransform transform(a, b, c, d, e, f);
    state.transform = transform;
    buffer()->setTransform(transform);
}

// ***** direct rect methods *****

void QCanvas2DContext::clearRect(qreal x, qreal y, qreal w, qreal h)
{
    if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
        return;

    buffer()->clearRect(QRectF(x, y, w, h));
}

void QCanvas2DContext::fillRect(qreal x, qreal y, qreal w, qreal h)
{
    if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
        return;

    buffer()->fillRect(QRectF(x, y, w, h));
}

void QCanvas2DContext::strokeRect(qreal x, qreal y, qreal w, qreal h)
{
    if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
        return;

    buffer()->strokeRect(QRectF(x, y, w, h));
}

// ***** path handling *****

void QCanvas2DContext::beginPath()
{
    buffer()->beginPath();
}

void QCanvas2DContext::clip()
{
    // TODO: Clipping to current shape not implemented
}

void QCanvas2DContext::clipRect(qreal x, qreal y, qreal w, qreal h)
{
    if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(w) || !qt_is_finite(h))
        return;

    buffer()->clipRect(QRectF(x, y, w, h));
}

void QCanvas2DContext::resetClipping()
{
    buffer()->resetClipping();
}

void QCanvas2DContext::fill()
{
    buffer()->fill();
}

void QCanvas2DContext::stroke()
{
    buffer()->stroke();
}

void QCanvas2DContext::fillPath(const QCanvasPath &path, int pathGroup)
{
    buffer()->fillPath(path, pathGroup);
}

void QCanvas2DContext::strokePath(const QCanvasPath &path, int pathGroup)
{
    buffer()->strokePath(path, pathGroup);
}

// ***** other *****

// Note: This is never called with fill=true, so text is always stroked
// TODO: Rename to strokeText()?
void QCanvas2DContext::drawText(const QString& text, qreal x, qreal y, bool fill)
{
    if (!qt_is_finite(x) || !qt_is_finite(y))
        return;

    QPainterPath textPath = createTextGlyphs(x, y, text);
    if (fill)
        buffer()->fillPath(textPath);
    else
        buffer()->strokePath(textPath);
}

void QCanvas2DContext::drawBoxShadow(QCanvasBoxShadow *shadow)
{
    if (!shadow)
        return;

    buffer()->drawBoxShadow(shadow);
}

int baseLineOffset(QCanvasPainter::TextBaseline value, const QFontMetrics &metrics)
{
    int offset = 0;
    switch (value) {
    case QCanvasPainter::TextBaseline::Top:
        break;
    case QCanvasPainter::TextBaseline::Hanging:
        offset = metrics.height() - metrics.ascent();
        break;
    case QCanvasPainter::TextBaseline::Middle:
        // TODO: Adjusted to match canvas painter middle, so fill and stroke are
        // positioned similarly. Check what is the optimal one.
        // If we will support aligning into middle of rect, this
        // method needs additional parameters.
        //offset = (metrics.ascent() >> 1) + metrics.height() - metrics.ascent();
        offset = metrics.height() - 0.625 * metrics.ascent();
        break;
    case QCanvasPainter::TextBaseline::Alphabetic:
        offset = metrics.ascent();
        break;
    case QCanvasPainter::TextBaseline::Bottom:
        offset = metrics.height();
        break;
    }
    return offset;
}

static int textAlignOffset(QCanvasPainter::TextAlign value, const QFontMetrics &metrics, const QString &text)
{
    int offset = 0;
    if (value == QCanvasPainter::TextAlign::Start)
        value = QGuiApplication::layoutDirection() == Qt::LeftToRight ? QCanvasPainter::TextAlign::Left : QCanvasPainter::TextAlign::Right;
    else if (value == QCanvasPainter::TextAlign::End)
        value = QGuiApplication::layoutDirection() == Qt::LeftToRight ? QCanvasPainter::TextAlign::Right: QCanvasPainter::TextAlign::Left;
    switch (value) {
    case QCanvasPainter::TextAlign::Center:
        offset = metrics.horizontalAdvance(text) / 2;
        break;
    case QCanvasPainter::TextAlign::Right:
        offset = metrics.horizontalAdvance(text);
        break;
    case QCanvasPainter::TextAlign::Left:
    default:
        break;
    }
    return offset;
}

void QCanvas2DContext::setGrabbedImage(const QImage& grab)
{
    m_grabbedImage = grab;
    m_grabbed = true;
}

QQmlRefPointer<QCanvas2DPixmap> QCanvas2DContext::createPixmap(const QUrl& url, QSizeF sourceSize)
{
    return m_canvas->loadedPixmap(url, sourceSize);
}

QPainterPath QCanvas2DContext::createTextGlyphs(qreal x, qreal y, const QString& text)
{
    const QFontMetrics metrics(state.font);
    int yoffset = baseLineOffset(static_cast<QCanvasPainter::TextBaseline>(state.textBaseline), metrics);
    int xoffset = textAlignOffset(static_cast<QCanvasPainter::TextAlign>(state.textAlign), metrics, text);

    QPainterPath textPath;

    textPath.addText(x - xoffset, y - yoffset+metrics.ascent(), state.font, text);
    return textPath;
}

QCanvas2DContext::QCanvas2DContext(QObject *parent)
    : QObject(parent)
      , m_buffer(new QCanvas2DCommandBuffer)
      , m_v4engine(nullptr)
      , m_grabbed(false)
{
}

QCanvas2DContext::~QCanvas2DContext()
{
    delete m_buffer;
}

QV4::ReturnedValue QCanvas2DContext::v4value() const
{
    return m_v4value.value();
}

QStringList QCanvas2DContext::contextNames() const
{
    return QStringList() << QStringLiteral("2d");
}

void QCanvas2DContext::init(QCanvas2DItem *canvasItem, const QVariantMap &args)
{
    Q_UNUSED(args);

    m_canvas = canvasItem;

    reset();
}

void QCanvas2DContext::flush()
{
    // TODO: Should we reset the state on every frame?
    // Currently required for state handling to work correctly.
    reset();

    if (m_buffer)
        m_canvas->setCcb(m_buffer);
}

QCanvas2DContextEngineData::QCanvas2DContextEngineData(QV4::ExecutionEngine *v4)
{
    QV4::Scope scope(v4);

    QV4::ScopedObject proto(scope, QCanvasJSContext2DPrototype::create(v4));
    proto->defineAccessorProperty(QStringLiteral("strokeStyle"), QCanvasJSContext2D::method_get_strokeStyle, QCanvasJSContext2D::method_set_strokeStyle);
    proto->defineAccessorProperty(QStringLiteral("font"), QCanvasJSContext2D::method_get_font, QCanvasJSContext2D::method_set_font);
    proto->defineAccessorProperty(QStringLiteral("fillRule"), QCanvasJSContext2D::method_get_fillRule, QCanvasJSContext2D::method_set_fillRule);
    proto->defineAccessorProperty(QStringLiteral("globalAlpha"), QCanvasJSContext2D::method_get_globalAlpha, QCanvasJSContext2D::method_set_globalAlpha);
    proto->defineAccessorProperty(QStringLiteral("globalBrightness"), QCanvasJSContext2D::method_get_globalBrightness, QCanvasJSContext2D::method_set_globalBrightness);
    proto->defineAccessorProperty(QStringLiteral("globalContrast"), QCanvasJSContext2D::method_get_globalContrast, QCanvasJSContext2D::method_set_globalContrast);
    proto->defineAccessorProperty(QStringLiteral("globalSaturate"), QCanvasJSContext2D::method_get_globalSaturate, QCanvasJSContext2D::method_set_globalSaturate);
    proto->defineAccessorProperty(QStringLiteral("lineCap"), QCanvasJSContext2D::method_get_lineCap, QCanvasJSContext2D::method_set_lineCap);
    proto->defineAccessorProperty(QStringLiteral("globalCompositeOperation"), QCanvasJSContext2D::method_get_globalCompositeOperation, QCanvasJSContext2D::method_set_globalCompositeOperation);
    proto->defineAccessorProperty(QStringLiteral("miterLimit"), QCanvasJSContext2D::method_get_miterLimit, QCanvasJSContext2D::method_set_miterLimit);
    proto->defineAccessorProperty(QStringLiteral("fillStyle"), QCanvasJSContext2D::method_get_fillStyle, QCanvasJSContext2D::method_set_fillStyle);
    proto->defineAccessorProperty(QStringLiteral("textBaseline"), QCanvasJSContext2D::method_get_textBaseline, QCanvasJSContext2D::method_set_textBaseline);
    proto->defineAccessorProperty(QStringLiteral("lineJoin"), QCanvasJSContext2D::method_get_lineJoin, QCanvasJSContext2D::method_set_lineJoin);
    proto->defineAccessorProperty(QStringLiteral("lineWidth"), QCanvasJSContext2D::method_get_lineWidth, QCanvasJSContext2D::method_set_lineWidth);
    proto->defineAccessorProperty(QStringLiteral("textAlign"), QCanvasJSContext2D::method_get_textAlign, QCanvasJSContext2D::method_set_textAlign);
    proto->defineAccessorProperty(QStringLiteral("lineDashOffset"), QCanvasJSContext2D::method_get_lineDashOffset, QCanvasJSContext2D::method_set_lineDashOffset);
    proto->defineAccessorProperty(QStringLiteral("antialias"), QCanvasJSContext2D::method_get_antialias, QCanvasJSContext2D::method_set_antialias);
    contextPrototype = proto;

    proto = scope.engine->newObject();
    proto->defineDefaultProperty(QStringLiteral("addColorStop"), QCanvas2DGradientObject::gradient_proto_addColorStop, 0);
    gradientPrototype = proto;

    proto = scope.engine->newObject();
    proto->defineDefaultProperty(QStringLiteral("addPath"), QCanvas2DPath2DObject::path_proto_addPath, 0);
    proto->defineDefaultProperty(QStringLiteral("closePath"), QCanvas2DPath2DObject::path_proto_closePath, 0);
    proto->defineDefaultProperty(QStringLiteral("moveTo"), QCanvas2DPath2DObject::path_proto_moveTo, 0);
    proto->defineDefaultProperty(QStringLiteral("lineTo"), QCanvas2DPath2DObject::path_proto_lineTo, 0);
    proto->defineDefaultProperty(QStringLiteral("bezierCurveTo"), QCanvas2DPath2DObject::path_proto_bezierCurveTo, 0);
    proto->defineDefaultProperty(QStringLiteral("quadraticCurveTo"), QCanvas2DPath2DObject::path_proto_quadraticCurveTo, 0);
    proto->defineDefaultProperty(QStringLiteral("arcTo"), QCanvas2DPath2DObject::path_proto_arcTo, 0);
    proto->defineDefaultProperty(QStringLiteral("arc"), QCanvas2DPath2DObject::path_proto_arc, 0);
    proto->defineDefaultProperty(QStringLiteral("rect"), QCanvas2DPath2DObject::path_proto_rect, 0);
    proto->defineDefaultProperty(QStringLiteral("roundedRect"), QCanvas2DPath2DObject::path_proto_roundedRect, 0);
    proto->defineDefaultProperty(QStringLiteral("roundRect"), QCanvas2DPath2DObject::path_proto_roundRect, 0);
    proto->defineDefaultProperty(QStringLiteral("ellipse"), QCanvas2DPath2DObject::path_proto_ellipse, 0);
    proto->defineDefaultProperty(QStringLiteral("ellipseRect"), QCanvas2DPath2DObject::path_proto_ellipseRect, 0);
    proto->defineDefaultProperty(QStringLiteral("circle"), QCanvas2DPath2DObject::path_proto_circle, 0);
    proto->defineDefaultProperty(QStringLiteral("beginSolidSubPath"), QCanvas2DPath2DObject::path_proto_beginSolidSubPath, 0);
    proto->defineDefaultProperty(QStringLiteral("beginHoleSubPath"), QCanvas2DPath2DObject::path_proto_beginHoleSubPath, 0);
    // Memory and size management
    proto->defineDefaultProperty(QStringLiteral("isEmpty"), QCanvas2DPath2DObject::path_proto_isEmpty, 0);
    proto->defineDefaultProperty(QStringLiteral("clear"), QCanvas2DPath2DObject::path_proto_clear, 0);
    pathPrototype = proto;
}

QCanvas2DContextEngineData::~QCanvas2DContextEngineData()
{
}

void QCanvas2DContext::popState()
{
    if (m_stateStack.isEmpty())
        return;

    QCanvas2DContext::State newState = m_stateStack.pop();

    if (newState.globalAlpha != state.globalAlpha)
        buffer()->setGlobalAlpha(newState.globalAlpha);

    if (newState.globalBrightness != state.globalBrightness)
        buffer()->setGlobalBrightness(newState.globalBrightness);

    if (newState.globalContrast != state.globalContrast)
        buffer()->setGlobalContrast(newState.globalContrast);

    if (newState.globalSaturate != state.globalSaturate)
        buffer()->setGlobalSaturate(newState.globalSaturate);

    if (newState.globalCompositeOperation != state.globalCompositeOperation)
        buffer()->setGlobalCompositeOperation(newState.globalCompositeOperation);

    if (newState.fillStyle && newState.fillStyle != state.fillStyle)
        buffer()->setFillStyle(newState.fillStyle);

    if (newState.strokeStyle && newState.strokeStyle != state.strokeStyle)
        buffer()->setStrokeStyle(newState.strokeStyle);

    if (newState.fillColor != state.fillColor)
        buffer()->setFillColor(newState.fillColor);

    if (newState.strokeColor != state.strokeColor)
        buffer()->setStrokeColor(newState.strokeColor);

    if (newState.lineWidth != state.lineWidth)
        buffer()->setLineWidth(newState.lineWidth);

    if (newState.antialias != state.antialias)
        buffer()->setAntialias(newState.antialias);

    if (newState.lineCap != state.lineCap)
        buffer()->setLineCap(newState.lineCap);

    if (newState.lineJoin != state.lineJoin)
        buffer()->setLineJoin(newState.lineJoin);

    if (newState.miterLimit != state.miterLimit)
        buffer()->setMiterLimit(newState.miterLimit);

    if (newState.clip != state.clip || newState.clipPath != state.clipPath)
        buffer()->clip(newState.clip, newState.clipPath);

    //if (newState.lineDash != state.lineDash)
    //    buffer()->setLineDash(newState.lineDash);

    state = newState;
    buffer()->restore();
}

void QCanvas2DContext::pushState()
{
    m_stateStack.push(state);
    buffer()->save();
}

void QCanvas2DContext::reset()
{
    QCanvas2DContext::State newState;
    m_stateStack.clear();
    m_stateStack.push(newState);
    popState();

    // Note: CanvasPainter reset doesn't clear the view,
    // should it be done here or not?
    //m_buffer->clearRect(QRectF(0, 0, m_canvas->width(), m_canvas->height()));
    buffer()->reset();
}

QV4::ExecutionEngine *QCanvas2DContext::v4Engine() const
{
    return m_v4engine;
}

void QCanvas2DContext::setV4Engine(QV4::ExecutionEngine *engine)
{
    if (m_v4engine != engine) {
        m_v4engine = engine;

        if (m_v4engine == nullptr)
            return;

        QCanvas2DContextEngineData *ed = engineData(engine);
        QV4::Scope scope(engine);
        QV4::Scoped<QCanvasJSContext2D> wrapper(scope, engine->memoryManager->allocate<QCanvasJSContext2D>());
        QV4::ScopedObject p(scope, ed->contextPrototype.value());
        wrapper->setPrototypeOf(p);
        wrapper->d()->setContext(this);
        m_v4value = wrapper;
    }
}

QT_END_NAMESPACE

#include "moc_qcanvas2dcontext_p.cpp"
