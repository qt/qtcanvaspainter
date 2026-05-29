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
    \brief Provides a 2D context for drawing on a Canvas2D item.

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

    Here is a simple example of using Canvas2D to create a round button.
    \table
    \row
    \li \inlineimage canvas2d-buttonexample.webp
    \li
    \code
    let offsetX = 2;
    let offsetY = 4;
    let shadow = ctx.createBoxShadow(40 + offsetX, 70 + offsetY,
                                     120, 60,
                                     30, "#60373F26", 15);
    ctx.drawBoxShadow(shadow);
    // Paint rounded rect
    ctx.beginPath();
    ctx.roundRect(40, 70, 120, 60, 30);
    ctx.fillStyle = "#DBEB00";
    ctx.fill();
    // Paint text
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.font = "24px 'Titillium Web'";
    ctx.fillStyle = "#373F26";
    ctx.fillText("CLICK!", 100, 100);
    \endcode
    \endtable

    Here is another example of painting a simple graph.
    \table
    \row
    \li \inlineimage canvas2d-graphexample.webp
    \li
    \code
    // Paint grid
    const grid = ctx.createGridPattern(0, 0, 10, 10,
                                       "#404040",
                                       "#202020");
    const w = 200;
    const h = 200;
    ctx.fillStyle = grid;
    ctx.fillRect(0, 0, w, h);
    // Paint axis
    ctx.fillStyle = "white";
    ctx.fillRect(0, 0.5 * h - 1, w, 2);
    ctx.fillRect(0.5 * w - 1, 0, 2, h);
    // Paint shadowed graph
    ctx.beginPath();
    ctx.moveTo(20, h * 0.8);
    ctx.bezierCurveTo(w * 0.2, h * 0.4,
                      w * 0.5, h * 0.8,
                      w - 20, h * 0.2);
    ctx.antialias = 10;
    ctx.lineWidth = 12;
    ctx.strokeStyle = "#D0000000";
    ctx.stroke();
    ctx.antialias = 1;
    ctx.lineWidth = 6;
    const lg = ctx.createLinearGradient(0, 0, 0, h);
    lg.addColorStop(0, "red");
    lg.addColorStop(1, "green");
    ctx.strokeStyle = lg;
    ctx.stroke();
    \endcode
    \endtable
*/


#define CHECK_CONTEXT(r)     if (!r || !r->d()->context() || !r->d()->context()->bufferValid()) \
THROW_GENERIC_ERROR("Not a Context2D object");

class QCanvas2DContextEngineData : public QV4::ExecutionEngine::Deletable
{
public:
    QCanvas2DContextEngineData(QV4::ExecutionEngine *engine);
    ~QCanvas2DContextEngineData();

    QV4::PersistentValue contextPrototype;
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
    static QV4::ReturnedValue method_get_textWrapMode(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_textWrapMode(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_textAntialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_textAntialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_get_textLineHeight(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_set_textLineHeight(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
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
        o->defineDefaultProperty(QStringLiteral("addPath"), method_addPath, 0);
        o->defineDefaultProperty(QStringLiteral("setPathWinding"), method_setPathWinding, 0);
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
    static QV4::ReturnedValue method_addPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue method_setPathWinding(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
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

static QCanvasPainter::FillRule qcanvas_fill_rule_from_string(const QString &fillRule)
{
    if (fillRule == QStringLiteral("evenodd") || fillRule == QStringLiteral("OddEvenFill"))
        return QCanvasPainter::FillRule::EvenOdd;
    return QCanvasPainter::FillRule::NonZero;
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
    \table
    \row
    \li \inlineimage canvas2d-line.webp
    \li
    \code
    ctx.beginPath();
    ctx.moveTo(20, 20);
    ctx.lineTo(140, 180);
    ctx.lineTo(180, 120);
    ctx.stroke();
    \endcode
    \endtable
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

    Adds a cubic bezier segment from last point in the path via two
    control points (\a cp1x, \a cp1y and \a cp2x, \a cp2y) to the specified point (\a x, \a y).
    \table
    \row
    \li \inlineimage canvas2d-beziercurve.webp
    \li
    \code
    ctx.beginPath();
    ctx.moveTo(20, 20);
    ctx.bezierCurveTo(150, 50, 50, 250, 180, 120);
    ctx.stroke();
    \endcode
    \endtable

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

    Adds a quadratic bezier segment from last point in the path via
    a control point (\a cpx, \a cpy) to the specified point (\a x, \a y).
    \table
    \row
    \li \inlineimage canvas2d-quadraticcurve.webp
    \li
    \code
    ctx.beginPath();
    ctx.moveTo(20, 20);
    ctx.quadraticCurveTo(150, 50, 180, 180);
    ctx.quadraticCurveTo(20, 220, 20, 20);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable

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

    Adds an arc segment at the corner defined by the last path point,
    and two specified points (\a x1, \a y1 and \a x2, \a y2) with \a radius.
    The arc is automatically connected to the path's latest point with
    a straight line if necessary.
    \table
    \row
    \li \inlineimage canvas2d-arcto.webp
    \li
    \code
    ctx.beginPath();
    ctx.moveTo(20, 20);
    ctx.arcTo(240, 20, 20, 220, 50);
    ctx.arcTo(20, 220, 20, 20, 30);
    ctx.stroke();
    \endcode
    \endtable

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

    Creates a new circle arc shaped sub-path. The arc center is at \a x, \a y,
    with \a radius, and the arc is drawn from angle \a startAngle to \a endAngle.
    The default curve direction is clockwise. To change direction to the opposite,
    set \a anticlockwise to true.
    Angles are specified in radians.
    \table
    \row
    \li \inlineimage canvas2d-arc.webp
    \li
    \code
    ctx.beginPath();
    ctx.moveTo(100, 100);
    ctx.arc(100, 100, 80, 0, 1.5 * Math.PI);
    ctx.closePath();
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable

    \note While HTML canvas 2D context uses arc() for painting circles, with
    Canvas2D it is recommended to use \l circle() or \l ellipse() for those.

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
    \qmlmethod object Canvas2DContext::rect(real x, real y, real width, real height)

    Creates a new rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height.
    \table
    \row
    \li \inlineimage canvas2d-rect.webp
    \li
    \code
    ctx.beginPath();
    ctx.rect(20, 20, 160, 160);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable
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

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_roundedRect(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    Q_UNUSED(thisObject);

    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "roundedRect(): Please use roundRect() instead.");

    RETURN_RESULT(QV4::Encode::null());
}

/*!
    \qmlmethod object Canvas2DContext::roundRect(real x, real y, real width, real height, real radius)

    Creates a new rounded rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height. Corners rounding will be \a radius.
    \table
    \row
    \li \inlineimage canvas2d-roundrect.webp
    \li
    \code
    ctx.beginPath();
    ctx.roundRect(20, 20, 160, 160, 30);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable
 */

/*!
    \qmlmethod object Canvas2DContext::roundRect(real x, real y, real width, real height,
                   real radiusTopLeft, real radiusTopRight,
                   real radiusBottomRight, real radiusBottomLeft)

    Creates a new rounded rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height. Corners rounding can be varying per-corner, with
    \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight, \a radiusBottomLeft.
    \table
    \row
    \li \inlineimage canvas2d-roundrect2.webp
    \li
    \code
    ctx.beginPath();
    ctx.roundRect(20, 20, 160, 160,
                  0, 40, 20, 80);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable
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

    Creates a new ellipse shaped sub-path centered at ( \a centerX, \a centerY) with \a radiusX and \a radiusY.
    \table
    \row
    \li \inlineimage canvas2d-ellipse.webp
    \li
    \code
    ctx.beginPath();
    ctx.ellipse(100, 100, 80, 60);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable
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
    \qmlmethod object Canvas2DContext::ellipseRect(real x, real y, real width, real height)

    Creates a new ellipse shaped sub-path into rect \a x, \a y, \a width, \a height.
    This ellipse will cover the rect area.
    \table
    \row
    \li \inlineimage canvas2d-ellipse2.webp
    \li
    \code
    ctx.fillRect(40, 20, 120, 160);
    ctx.beginPath();
    ctx.ellipseRect(40, 20, 120, 160);
    ctx.stroke();
    \endcode
    \endtable

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

    Creates a new circle shaped sub-path centered at ( \a centerX, \a centerY) with \a radius.
    \table
    \row
    \li \inlineimage canvas2d-circle.webp
    \li
    \code
    ctx.beginPath();
    ctx.circle(100, 100, 80);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable

    \note Compared to \l arc(), this method does not add a straight line from
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

    Start a solid subpath. This is equivalent to
    \c setPathWinding("counterclockwise")
    \sa beginHoleSubPath()
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

    Start a hole subpath. This is equivalent to
    \c setPathWinding("clockwise")
    \table
    \row
    \li \inlineimage canvas2d-beginhole.webp
    \li
    \code
    ctx.beginPath();
    ctx.circle(100, 100, 80);
    ctx.beginHoleSubPath();
    ctx.rect(60, 60, 80, 80);
    ctx.beginSolidSubPath();
    ctx.circle(100, 100, 20);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable

    \sa beginSolidSubPath()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_beginHoleSubPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    r->d()->context()->buffer()->beginHoleSubPath();

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::addPath(path2d path, transform2d transform)

    Adds \a path into the current path, optionally using \a transform to
    alter the path points. When \a transform is not provided (or it is
    identity matrix), this operation is very fast as it reuses the path data.
    \table
    \row
    \li \inlineimage canvas2d-addpath.webp
    \li
    \code
    // myPath is path2d property
    if (myPath.isEmpty())
        myPath.circle(60, 60, 40);
    ctx.beginPath();
    ctx.addPath(myPath);
    let t = ctx.createTransform2D();
    t.translate(80, 80);
    ctx.addPath(myPath, t);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable
*/

/*!
    \qmlmethod object Canvas2DContext::addPath(path2d path, int start, int count, transform2d transform)

    Adds \a path into the current path, starting from the command at \a start
    and including \a count amount of commands. Optionally using \a transform to
    alter the path points.
    The range of \a start and \a count is checked, so that commands are not
    accessed more than the path has commands.
    In case the path shouldn't continue from the current path position, call
    first \l moveTo().
    \table
    \row
    \li \inlineimage canvas2d-addpath2.webp
    \li
    \code
    // myPath is QCanvasPath
    if (myPath.isEmpty()) {
        myPath.moveTo(20, 60);
        for (let i = 1; i < 160; i++) {
            myPath.lineTo(20 + i,
                60 + 20 * Math.sin(0.1 * i));
        }
    }
    ctx.stroke(myPath);
    ctx.beginPath();
    let t = ctx.createTransform2D();
    t.translate(0, 80);
    ctx.addPath(myPath, 20, 100, t);
    ctx.stroke();
    \endcode
    \endtable
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_addPath(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 1) {
        QV4::ScopedValue arg1(scope, argv[0]);
        if (arg1->as<Object>()) {
            QCanvasPath path = QV4::ExecutionEngine::toVariant(arg1, QMetaType::fromType<QCanvasPath>()).value<QCanvasPath>();
            QTransform transform;
            if (argc >= 3) {
                int start = argv[1].toInteger();
                int count = argv[2].toInteger();
                if (argc >= 4) {
                    QV4::ScopedValue arg4(scope, argv[3]);
                    transform = QV4::ExecutionEngine::toVariant(arg4, QMetaType::fromType<QTransform>()).value<QTransform>();
                }
                r->d()->context()->buffer()->addPath(path, start, count, transform);
            } else if (argc >= 2) {
                QV4::ScopedValue arg2(scope, argv[1]);
                transform = QV4::ExecutionEngine::toVariant(arg2, QMetaType::fromType<QTransform>()).value<QTransform>();
                r->d()->context()->buffer()->addPath(path, transform);
            } else {
                r->d()->context()->buffer()->addPath(path, transform);
            }
        }
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::setPathWinding(string winding)

    Sets the current sub-path \a winding to either "counterclockwise" (default) or "clockwise".
    "counterclockwise" draws solid subpaths while "clockwise" draws holes.
    \table
    \row
    \li \inlineimage canvas2d-pathwinding.webp
    \li
    \code
    ctx.beginPath();
    ctx.roundRect(20, 20, 160, 160, 40);
    ctx.setPathWinding("clockwise");
    ctx.circle(140, 60, 20);
    ctx.rect(60, 120, 80, 30);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable

    \sa beginHoleSubPath(), beginSolidSubPath()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_setPathWinding(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 1) {
        QString windingString = argv[0].toQString();
        QCanvasPainter::PathWinding winding;
        if (windingString == QStringLiteral("clockwise") || windingString == QStringLiteral("ClockWise"))
            winding = QCanvasPainter::PathWinding::ClockWise;
        else if (windingString == QStringLiteral("counterclockwise") || windingString == QStringLiteral("CounterClockWise"))
            winding = QCanvasPainter::PathWinding::CounterClockWise;
        else
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "setPathWinding(): Incorrect arguments")

        r->d()->context()->state.pathWinding = winding;
        r->d()->context()->buffer()->setPathWinding(winding);
    }

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

    Pops and restores the current render state.
    So previously saved state will be restored.
    If save() has not been called and the state stack
    is empty, calling this does nothing.

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

    Resets the current painter state to default values.

    \note This method differs from the HTML canvas 2D context reset() method
    so that it doesn't visually clear the canvas buffers.
    \table
    \row
    \li \inlineimage canvas2d-reset.webp
    \li
    \code
    // Adjust the paint state
    ctx.strokeStyle = "#00414A";
    ctx.fillStyle = "#2CDE85";
    ctx.lineWidth = 10;
    ctx.translate(100, 65);
    ctx.rotate(-0.4);
    ctx.translate(-100, -65);
    ctx.beginPath();
    ctx.roundRect(20, 40, 160, 50, 20);
    ctx.fill();
    ctx.stroke();
    // Reset to default paint state
    ctx.reset();
    ctx.fillRect(20, 140, 60, 40);
    ctx.strokeRect(120, 140, 60, 40);
    \endcode
    \endtable

    \sa save(), restore()
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

    Pushes and saves the current render state into a state stack.
    A matching \l restore() must be used to restore the state.

    \note The current path is NOT part of the drawing state. The path can be reset by
    invoking the beginPath() method.
    \table
    \row
    \li \inlineimage canvas2d-save.webp
    \li
    \code
    ctx.strokeRect(20, 20, 160, 40);
    // Save and adjust the paint state
    ctx.save();
    ctx.strokeStyle = "black";
    ctx.lineWidth = 3;
    ctx.rotate(0.1);
    ctx.strokeRect(20, 80, 180, 20);
    // Restore the saved paint state
    ctx.restore();
    ctx.strokeRect(20, 140, 160, 40);
    \endcode
    \endtable

    \sa restore()
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

    Rotates the current coordinate system clockwise by \a angle.

    The angle is specified in radians.
    \table
    \row
    \li \inlineimage canvas2d-rotate.webp
    \li
    \code
    ctx.translate(100, 100);
    ctx.rotate(Math.PI / 4);
    ctx.translate(-100, -100);
    ctx.beginPath();
    ctx.roundRect(20, 70, 160, 60, 10);
    ctx.fill();
    ctx.stroke();
    ctx.fillStyle = "black";
    ctx.fillText("Cute!", 100, 100);
    \endcode
    \endtable
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
    \qmlmethod object Canvas2DContext::scale(real scale)

    Scales the current coordinate system by \a scale. Both x and y coordinates
    are scaled evenly.
    \table
    \row
    \li \inlineimage canvas2d-scale.webp
    \li
    \code
    for (let i = 0; i < 20; i++) {
        ctx.beginPath();
        ctx.roundRect(20, 20, 160, 160, 10);
        ctx.stroke();
        ctx.translate(100, 100);
        ctx.scale(0.8);
        ctx.translate(-100, -100);
    }
    \endcode
    \endtable
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
    \qmlmethod object Canvas2DContext::skew(real angleX, real angleY)

    Skews (shears) the current coordinate system along X axis by \a angleX
    and along Y axis by \a angleY. The default value of \a angleY is \c 0 when
    only a single parameter is provided. Angles are specified in radians.
    \table
    \row
    \li \inlineimage canvas2d-skew.webp
    \li
    \code
    ctx.translate(100, 100);
    ctx.skew(-0.6);
    ctx.translate(-100, -100);
    ctx.beginPath();
    ctx.roundRect(40, 70, 120, 60, 10);
    ctx.fill();
    ctx.stroke();
    ctx.fillStyle = "black";
    ctx.fillText("Cute!", 100, 100);
    \endcode
    \endtable
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

    Translates the current coordinate system by \a x and \a y.
    \table
    \row
    \li \inlineimage canvas2d-translate.webp
    \li
    \code
    function paintRect() {
        ctx.beginPath();
        ctx.roundRect(20, 20, 160, 60, 10);
        ctx.fill();
        ctx.stroke();
    };
    paintRect();
    ctx.translate(0, 100);
    paintRect();
    \endcode
    \endtable
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
    \qmlmethod object Canvas2DContext::transform(transform2d transform)

    Multiplies the current coordinate system by specified \a transform.
    \table
    \row
    \li \inlineimage canvas2d-transform2.webp
    \li
    \code
    let t = ctx.createTransform2D();
    t.translate(100, 100);
    t.rotate(36);
    t.translate(-100, -100);
    for (let i = 0; i < 10; i++) {
        ctx.transform(t);
        ctx.beginPath();
        ctx.roundRect(80, 15, 40, 20, 10);
        ctx.fill();
        ctx.stroke();
    }
    \endcode
    \endtable
*/
/*!
    \qmlmethod object Canvas2DContext::transform(real a, real b, real c, real d, real e, real f)

    Multiplies the current coordinate system by the specified transform
    (\a a, \a b, \a c, \a d, \a e, \a f).

    This method is similar to setTransform(), but instead of replacing
    the old transform matrix, this method applies the given transform matrix
    to the current matrix by multiplying to it.

    \sa setTransform()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_transform(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (argc >= 6) {
        r->d()->context()->transform( argv[0].toNumber()
                                     , argv[1].toNumber()
                                     , argv[2].toNumber()
                                     , argv[3].toNumber()
                                     , argv[4].toNumber()
                                     , argv[5].toNumber());
    } else if (argc >= 1) {
        QV4::ScopedValue value(scope, argv[0]);
        if (value->as<Object>()) {
            QTransform t = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QTransform>()).value<QTransform>();
            r->d()->context()->transform(t.m11(), t.m12(), t.m21(), t.m22(), t.m31(), t.m32());
        }
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::setTransform(transform2d transform)

    Resets the current transform and uses \a transform instead.
    \table
    \row
    \li \inlineimage canvas2d-transform.webp
    \li
    \code
    ctx.beginPath();
    ctx.roundRect(80, 20, 40, 40, 10);
    ctx.fill();
    ctx.stroke();
    let t = ctx.createTransform2D();
    t.translate(100, 20);
    t.rotate(45);
    t.scale(2.0, 2.0);
    ctx.setTransform(t);
    ctx.beginPath();
    ctx.roundRect(20, 20, 40, 40, 10);
    ctx.fill();
    ctx.stroke();
    \endcode
    \endtable

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

    Holds the current alpha (transparency) value applied to rendering
    operations. This alpha value is
    applied to all rendered shapes. Already transparent paths will get
    proportionally more transparent as well.
    Alpha should be between 0.0 (fully transparent) and 1.0 (fully opaque).
    By default alpha is \c 1.0.
    \table
    \row
    \li \inlineimage canvas2d-globalalpha.webp
    \li
    \code
    ctx.fillStyle = "#d9f720";
    for (let i = 0; i < 4; i++) {
        let x = 100 * (i % 2);
        let y = 100 * Math.floor(i / 2);
        ctx.globalAlpha = 1.0 - i * 0.3;
        ctx.fillRect(x, y, 100, 100);
        ctx.drawImage("qt_logo2.png", x, y, 100, 100);
    }
    \endcode
    \endtable
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

    Holds the current brightness value applied to rendering
    operations. This brightess is
    applied to all rendered shapes. A value of 0 will cause painting
    to be completely black. Value can also be bigger than 1.0, to
    increase the brightness.
    By default, brightness is \c 1.0.
    \table
    \row
    \li \inlineimage canvas2d-globalbrightness.webp
    \li
    \code
    ctx.fillStyle = "#d9f720";
    for (let i = 0; i < 4; i++) {
        let x = 100 * (i % 2);
        let y = 100 * Math.floor(i / 2);
        ctx.globalBrightness = 1.5 - i * 0.45;
        ctx.fillRect(x, y, 100, 100);
        ctx.drawImage("qt_logo2.png", x, y, 100, 100);
    }
    \endcode
    \endtable
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

    Holds the current contrast value applied to rendering
    operations. This contrast is
    applied to all rendered shapes. A value of 0 will cause painting
    to be completely gray (0.5, 0.5, 0.5). Value can also be bigger
    than 1.0, to increase the contrast.
    By default, contrast is \c 1.0.
    \table
    \row
    \li \inlineimage canvas2d-globalcontrast.webp
    \li
    \code
    ctx.fillStyle = "#d9f720";
    for (let i = 0; i < 4; i++) {
        let x = 100 * (i % 2);
        let y = 100 * Math.floor(i / 2);
        ctx.globalContrast = 1.5 - i * 0.45;
        ctx.fillRect(x, y, 100, 100);
        ctx.drawImage("qt_logo2.png", x, y, 100, 100);
    }
    \endcode
    \endtable
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

    Holds the current saturate value applied to rendering
    operations. This saturations is
    applied to all rendered shapes. A value of 0 will disable saturation
    and cause painting to be completely grayscale. Value can also be bigger
    than 1.0, to increase the saturation.
    By default, saturation is \c 1.0.
    \table
    \row
    \li \inlineimage canvas2d-globalsaturate.webp
    \li
    \code
    ctx.fillStyle = "#d9f720";
    for (let i = 0; i < 4; i++) {
        let x = 100 * (i % 2);
        let y = 100 * Math.floor(i / 2);
        ctx.globalSaturate = 1.5 - i * 0.5;
        ctx.fillRect(x, y, 100, 100);
        ctx.drawImage("qt_logo2.png", x, y, 100, 100);
    }
    \endcode
    \endtable
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

    Holds the current composition operation. This mode is
    applied to all painting operations. Allowed operations are:

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
    The default fill style is solid black ('#000000').
    The style can be either a string containing a CSS color, \l{colorvaluetypedocs}{QML color}, or a canvas brush object.
    Invalid values are ignored.
    This property accepts several color syntaxes:
    \list
    \li 'rgb(red, green, blue)' - for example: 'rgb(255, 100, 55)' or 'rgb(100%, 70%, 30%)'
    \li 'rgba(red, green, blue, alpha)' - for example: 'rgb(255, 100, 55, 1.0)' or 'rgb(100%, 70%, 30%, 0.5)'
    \li 'hsl(hue, saturation, lightness)'
    \li 'hsla(hue, saturation, lightness, alpha)'
    \li '#RRGGBB' - for example: '#00FFCC'
    \li '#AARRGGBB' - for example: '#8000FFCC'
    \li SVG color name - for example: 'black', 'green' or 'lightsteelblue'
    \li Qt.hsla(hue, saturation, lightness, alpha) - for example: Qt.hsla(0.3, 0.7, 1, 1.0)
    \li Qt.rgba(red, green, blue, alpha) - for example: Qt.rgba(0.3, 0.7, 1, 1.0)
    \endlist
    If \c fillStyle or \l strokeStyle is assigned many times in a loop, the Qt.rgba() syntax should be chosen, as it has the
    best performance, because it's already a valid QColor value and does not need to be parsed every time.

    \table
    \row
    \li \inlineimage canvas2d-fillstyle.webp
    \li
    \code
    ctx.fillStyle = "black";
    ctx.fillRect(20, 20, 160, 160);
    ctx.fillStyle = Qt.rgba(0, 0.25, 0.3, 1);
    ctx.fillRect(40, 40, 120, 120);
    ctx.fillStyle = "#2CDE85";
    ctx.fillRect(60, 60, 80, 80);
    \endcode
    \endtable

    \table
    \row
    \li \inlineimage canvas2d-fillstyle2.webp
    \li
    \code
    const g2 = ctx.createRadialGradient(140, 40, 300);
    g2.addColorStop(0, Qt.rgba(0.2, 0.8, 0.6));
    g2.addColorStop(1, Qt.rgba(0, 0.25, 0.3, 1));
    ctx.fillStyle = g2;
    ctx.fillRect(20, 20, 160, 160);
    g2.setCenterPosition(100, 100);
    ctx.fillStyle = g2;
    ctx.fillRect(40, 40, 120, 120);
    \endcode
    \endtable

    \sa createLinearGradient()
    \sa createRadialGradient()
    \sa createConicalGradient()
    \sa createBoxGradient()
    \sa createBoxShadow()
    \sa createPattern()
    \sa createGridPattern()
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

static QCanvasBrush toBrush(const QV4::ScopedValue &value)
{
    QVariant var = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QCanvasBrush>());
    if (var.canConvert<QCanvasLinearGradient>())
        return var.value<QCanvasLinearGradient>();
    else if (var.canConvert<QCanvasRadialGradient>())
        return var.value<QCanvasRadialGradient>();
    else if (var.canConvert<QCanvasConicalGradient>())
        return var.value<QCanvasConicalGradient>();
    else if (var.canConvert<QCanvasBoxGradient>())
        return var.value<QCanvasBoxGradient>();
    else if (var.canConvert<QCanvasBoxShadow>())
        return var.value<QCanvasBoxShadow>();
    else if (var.canConvert<QCanvasCustomBrush>())
        return var.value<QCanvasCustomBrush>();
    else if (var.canConvert<QCanvasGridPattern>())
        return var.value<QCanvasGridPattern>();
    else if (var.canConvert<QCanvasImagePattern>())
        return var.value<QCanvasImagePattern>();
    return var.value<QCanvasBrush>();
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
            QCanvasBrush g = toBrush(value);
            r->d()->context()->state.fillStyle = g;

            r->d()->context()->buffer()->setFillStyle(g);
            r->d()->context()->m_fillStyle.set(scope.engine, value);
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

/*!
    \qmlproperty string Canvas2DContext::fillRule

    Holds the current fill rule used for filling shapes.
    This value is applied to all fill() calls
    after the rule has been set. The default fill rule is \c "nonzero"

    The following fill rules are supported:

    \value "nonzero" (or "WindingFill")
        The path is filled using the non zero winding rule. With this rule,
        we determine whether a point is inside the shape by using the following method.
        Draw a horizontal line from the point to a location outside the shape. Determine
        whether the direction of the line at each intersection point is up or down.
        The winding number is determined by summing the direction of each intersection.
        If the number is non zero, the point is inside the shape. This fill mode can also
        in most cases be considered as the intersection of closed shapes. This mode is the default.

    \value "evenodd" (or "OddEvenFill")
        The path is filled using the odd even fill rule. With this rule, we determine
        whether a point is inside the shape by using the following method. Draw a horizontal
        line from the point to a location outside the shape, and count the number of
        intersections. If the number of intersections is an odd number, the point is
        inside the shape.

    \table
    \row
    \li \inlineimage canvas2d-fillrule.webp
    \li
    \code
    function paintStar() {
        ctx.beginPath();
        ctx.moveTo(120, 60);
        for (let i = 1; i < 6; ++i) {
            ctx.lineTo(60 + 60 * Math.cos(0.8 * i * Math.PI),
                60 + 60 * Math.sin(0.8 * i * Math.PI));
        }
        ctx.fill();
        ctx.stroke();
    };
    ctx.fillRule = "nonzero";
    paintStar();
    ctx.translate(75, 75);
    ctx.fillRule = "evenodd";
    paintStar();
    \endcode
    \endtable

    \sa fill()
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_fillRule(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    if (r->d()->context()->state.fillRule == QCanvasPainter::FillRule::EvenOdd)
        RETURN_RESULT(scope.engine->newString(QStringLiteral("evenodd")));

    RETURN_RESULT(scope.engine->newString(QStringLiteral("nonzero")));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_fillRule(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());
    if (value->isString()) {
        QString mode = value->toQString();
        if (mode == QStringLiteral("nonzero") || mode == QStringLiteral("WindingFill"))
            r->d()->context()->state.fillRule = QCanvasPainter::FillRule::NonZero;
        else if (mode == QStringLiteral("evenodd") || mode == QStringLiteral("OddEvenFill"))
            r->d()->context()->state.fillRule = QCanvasPainter::FillRule::EvenOdd;
        else
            THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "fillRule: Incorrect arguments")
    }
    r->d()->context()->buffer()->setFillRule(r->d()->context()->state.fillRule);
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty variant Canvas2DContext::strokeStyle
    Holds the current color or style to use for the lines around shapes,
    The style can be either a string containing a CSS color, \l{colorvaluetypedocs}{QML color}, or a canvas brush object.
    Invalid values are ignored.

    The default value is black ('#000000').

    \table
    \row
    \li \inlineimage canvas2d-strokestyle.webp
    \li
    \code
    ctx.strokeStyle = "black";
    ctx.strokeRect(20, 20, 160, 160);
    ctx.strokeStyle = Qt.rgba(0, 0.25, 0.3, 1);
    ctx.strokeRect(40, 40, 120, 120);
    ctx.strokeStyle = "#2CDE85";
    ctx.strokeRect(60, 60, 80, 80);
    \endcode
    \endtable

    \table
    \row
    \li \inlineimage canvas2d-strokestyle2.webp
    \li
    \code
    const g1 = ctx.createLinearGradient(180, 20, 20, 180);
    g1.addColorStop(0, Qt.rgba(0.2, 0.8, 0.6));
    g1.addColorStop(1, "black");
    ctx.strokeStyle = g1;
    ctx.strokeRect(20, 20, 160, 160);
    g1.addColorStop(1, "yellow");
    ctx.strokeStyle = g1;
    ctx.strokeRect(40, 40, 120, 120);
    \endcode
    \endtable

     \sa createLinearGradient()
     \sa createRadialGradient()
     \sa createConicalGradient()
     \sa createBoxGradient()
     \sa createBoxShadow()
     \sa createPattern()
     \sa createGridPattern()
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
            QCanvasBrush g = toBrush(value);
            r->d()->context()->state.strokeStyle = g;
            r->d()->context()->buffer()->setStrokeStyle(g);
            r->d()->context()->m_strokeStyle.set(scope.engine, value);
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
    Returns a \l{lineargradient2d} object that represents a linear gradient that transitions the color along a line between
    the start point (\a x0, \a y0) and the end point (\a x1, \a y1).

    Gradients must have two or more color stops, representing color shifts positioned from 0 to 1 between
    the gradient's starting and end points.

    \table
    \row
    \li \inlineimage lineargradient-example.webp
    \li
    \code
    const lg = ctx.createLinearGradient(0, 0, 200, 200);
    lg.addColorStop(0.0, "#1a2a6c");
    lg.addColorStop(0.5, "#b21f1f");
    lg.addColorStop(1.0, "#fdbb2d");
    ctx.fillStyle = lg;
    ctx.fillRect(0, 0, 200, 200);
    \endcode
    \endtable

    \sa lineargradient2d::addColorStop()
    \sa createRadialGradient()
    \sa createConicalGradient()
    \sa createPattern()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createLinearGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);

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

        QCanvasLinearGradient lg(x0, y0, x1, y1);
        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(lg)));
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createRadialGradient(real x0, real y0, real r0, real x1, real y1, real r1)

    Returns a \l{radialgradient2d} object that represents a radial gradient that
    paints along the cone given by the start circle with origin (\a x0, \a y0)
    and radius \a r0, and the end circle with origin (\a x1, \a y1) and radius
    \a r1.

    \table
    \row
    \li \inlineimage radialgradient-example.webp
    \li
    \code
    const rg = ctx.createRadialGradient(100, 100, 100, 0);
    rg.addColorStop(0.0, "#fdbb2d");
    rg.addColorStop(0.6, "#b21f1f");
    rg.addColorStop(1.0, "#1a2a6c");
    ctx.fillStyle = rg;
    ctx.fillRect(0, 0, 200, 200);
    \endcode
    \endtable

    \sa radialgradient2d::addColorStop()
    \sa createLinearGradient()
    \sa createConicalGradient()
    \sa createPattern()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createRadialGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);

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

        QCanvasRadialGradient rg;
        if (extended)
            rg = QCanvasRadialGradient(icx, icy, iRad, ocx, ocy, oRad);
        else
            rg = QCanvasRadialGradient(icx, icy, oRad, iRad);
        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(rg)));
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createConicalGradient(real x, real y, real angle)

    Returns a \l{conicalgradient2d} object that represents a conical gradient that
    interpolates colors counter-clockwise around a center point (\a x, \a y)
    with a start angle \a angle in units of radians.

    \table
    \row
    \li \inlineimage conicalgradient-example.webp
    \li
    \code
    const cg = ctx.createConicalGradient(100, 100, 1.75 * Math.PI);
    cg.addColorStop(0.0, "#fdbb2d");
    cg.addColorStop(0.5, "#1a2a6c");
    cg.addColorStop(1.0, "#fdbb2d");
    ctx.fillStyle = cg;
    ctx.beginPath();
    ctx.ellipseRect(20, 20, 160, 160);
    ctx.fill();
    \endcode
    \endtable

    \sa conicalgradient2d::addColorStop()
    \sa createLinearGradient()
    \sa createRadialGradient()
    \sa createPattern()
    \sa fillStyle
    \sa strokeStyle
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createConicalGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);

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

        QCanvasConicalGradient cg(x, y, angle);
        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(cg)));
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

    Returns a \l{boxgradient2d} object that represents a box gradient that
    covers the rectangle area (\a x, \a y, \a width, \a height) with feather
    (smoothing) \a feather and corner radius \a radius.

    \table
    \row
    \li \inlineimage boxgradient-example.webp
    \li
    \code
    const bg = ctx.createBoxGradient(20, 20, 160, 160, 20, 50);
    bg.addColorStop(0.0, "#1a2a6c");
    bg.addColorStop(0.2, "#fdbb2d");
    bg.addColorStop(1.0, "transparent");
    ctx.fillStyle = bg;
    ctx.fillRect(20, 20, 160, 160);
    \endcode
    \endtable

    \sa boxgradient2d::addColorStop()
    \sa fillStyle
    \sa strokeStyle
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createBoxGradient(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);

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

        QCanvasBoxGradient bg(x, y, w, h, feather, radius);
        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(bg)));
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createBoxShadow(real x, real y, real width, real height,
                                                       real blur, string color, real radius)

    Returns a \l{boxshadow2d} object with color \a color that represents
    a box shadow that covers the rectangle area (\a x, \a y, \a width, \a height)
    with blur \a blur and corner radius \a radius.

    \table
    \row
    \li \inlineimage canvas2d-shadowbox.webp
    \li
    \code
    let offsetX = -2;
    let offsetY = 4;
    let shadow = ctx.createBoxShadow(40 + offsetX,
                                     40 + offsetY,
                                     120, 120,
                                     30, "black", 0);
    ctx.drawBoxShadow(shadow);
    ctx.beginPath();
    ctx.roundRect(40, 40, 120, 120, 30);
    ctx.fillStyle = "#2CDE85";
    ctx.fill();
    \endcode
    \endtable

    \sa drawBoxShadow()
*/

/*!
    \qmlmethod object Canvas2DContext::createBoxShadow(real x, real y, real width, real height,
                                                       real blur, string color,
                                                       real radiusTopLeft, real radiusTopRight,
                                                       real radiusBottomRight, real radiusBottomLeft)

    Returns a \l{boxshadow2d} object with color \a color that represents
    a box shadow that covers the rectangle area (\a x, \a y, \a width, \a height)
    with blur \a blur and corner radius (\a radiusTopLeft, \a radiusTopRight,
    \a radiusBottomRight, \a radiusBottomLeft).

    \sa drawBoxShadow()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createBoxShadow(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);

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

        QCanvasBoxShadow b(x, y, w, h, 0, blur, color);
        b.setTopLeftRadius(radiusTL);
        b.setTopRightRadius(radiusTR);
        b.setBottomRightRadius(radiusBR);
        b.setBottomLeftRadius(radiusBL);
        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(b)));
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createGridPattern(real x, real y, real width, real height,
                                                       string lineColor, string backgroundColor,
                                                       real lineWidth, real feather, real angle)

    Returns a \l{gridpattern2d} object that covers the rectangle area (\a x, \a y, \a width, \a height).
    The grid uses \a lineColor for lines and \a backgroundColor for the background.
    The line width is \a lineWidth, line feather is \a feather and the rotation angle is \a angle in radians.

    \table
    \row
    \li \inlineimage gridpattern-example.webp
    \li
    \code
    const grid = ctx.createGridPattern(0, 0, 10, 10,
                        "#404040", "#202020");
    ctx.fillStyle = grid;
    ctx.fillRect(0, 0, 200, 200);
    \endcode
    \endtable
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createGridPattern(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);

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

        QCanvasGridPattern gp(x, y, w, h, lineColor, backgroundColor, lineWidth, feather, angle);
        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(gp)));
    }

    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createPath2D()

    Returns a new path2d object. Calling this is equal to HTML canvas "new Path2D()" command.
    \sa fill(), stroke()
  */
/*!
    \qmlmethod object Canvas2DContext::createPath2D(path2d path)

    Returns a new path2d object, with the copy of \a path.
    Calling this is equal to HTML canvas "new Path2D(path)" command.
    \sa fill(), stroke()
  */
/*!
    \qmlmethod object Canvas2DContext::createPath2D(string svgPath)

    Returns a new path2d object, with the content of \a svgPath.
    Calling this is equal to HTML canvas "new Path2D(d)" command.
    \sa fill(), stroke()
  */

QV4::ReturnedValue QCanvasJSContext2DPrototype::method_createPath2D(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, thisObject->as<QCanvasJSContext2D>());
    CHECK_CONTEXT(r)

    QCanvasPath path;
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
            path.addPath(svgPath, transform);
        } else if (arg1->isObject()) {
            // Path2D as an parameter.
            QCanvasPath inPath = QV4::ExecutionEngine::toVariant(arg1, QMetaType::fromType<QCanvasPath>()).value<QCanvasPath>();
            if (!inPath.isEmpty())
                path.addPath(inPath);
        }
    }

    RETURN_RESULT(scope.engine->fromVariant(path));
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
    \qmlmethod object Canvas2DContext::drawBoxShadow(boxshadow2d shadow)

    Draws a given box \a shadow. The shadow will be painted with
    position, size, color, blur etc. defined by \a shadow.
    Calling \l beginPath() before this method is not required.

    \table
    \row
    \li \inlineimage canvas2d-shadowbox.webp
    \li
    \code
    let offsetX = -2;
    let offsetY = 4;
    let shadow = ctx.createBoxShadow(40 + offsetX,
                                     40 + offsetY,
                                     120, 120,
                                     30, "black", 0);
    ctx.drawBoxShadow(shadow);
    ctx.beginPath();
    ctx.roundRect(40, 40, 120, 120, 30);
    ctx.fillStyle = "#2CDE85";
    ctx.fill();
    \endcode
    \endtable

    \sa createBoxShadow()
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_drawBoxShadow(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{

    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    QV4::ScopedValue value(scope, argc ? argv[0] : QV4::Value::undefinedValue());
    QCanvasBoxShadow s = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QCanvasBoxShadow>()).value<QCanvasBoxShadow>();
    r->d()->context()->drawBoxShadow(&s);
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::createPattern(Image image, string repetition)
    Returns an \l{imagepattern2d} object that uses the given image and repeats in the
    direction(s) given by the repetition argument.

    The \a image parameter must be a valid Image item
    or a loaded image url. If there is no image data, this function throws an
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
        QCanvasImagePattern pattern;
        QString filename = argv[0].toQStringNoThrow();
        QImage patternTexture = r->d()->context()->createPixmap(QUrl(filename))->image();
        if (!patternTexture.isNull()) {
            QString repetition = argv[1].toQStringNoThrow();
            // "repeat" is the default, even if the string is empty.
            QCanvasPainter::ImageFlags flags;
            if (repetition == QStringLiteral("repeat") || repetition.isEmpty()) {
                flags.setFlag(QCanvasPainter::ImageFlag::Repeat, true);
            } else if (repetition == QStringLiteral("repeat-x")) {
                flags.setFlag(QCanvasPainter::ImageFlag::RepeatX, true);
            } else if (repetition == QStringLiteral("repeat-y")) {
                flags.setFlag(QCanvasPainter::ImageFlag::RepeatY, true);
            } // else "no-repeat"

            pattern.setImageSize(patternTexture.size()); // Default to image original size
            r->d()->context()->addImagePattern(pattern, filename, patternTexture, flags);
        }

        RETURN_RESULT(scope.engine->fromVariant(QVariant::fromValue(pattern)));
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

    \table
    \row
    \li \inlineimage canvas2d-linecap.webp
    \li
    \code
    let path = ctx.createPath2D();
    path.moveTo(40, 60);
    path.lineTo(160, 60);
    ctx.lineCap = "butt";
    ctx.stroke(path);
    ctx.lineCap = "square";
    ctx.translate(0, 40);
    ctx.stroke(path);
    ctx.lineCap = "round";
    ctx.translate(0, 40);
    ctx.stroke(path);
    \endcode
    \endtable
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

    \table
    \row
    \li \inlineimage canvas2d-linejoin.webp
    \li
    \code
    let path = ctx.createPath2D();
    path.moveTo(40, 20);
    path.lineTo(100, 80);
    path.lineTo(160, 40);
    path.lineTo(160, 70);
    ctx.lineJoin = "miter";
    ctx.stroke(path);
    ctx.lineJoin = "bevel";
    ctx.translate(0, 50);
    ctx.stroke(path);
    ctx.lineJoin = "round";
    ctx.translate(0, 50);
    ctx.stroke(path);
    \endcode
    \endtable

    \sa miterLimit
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

    Holds the current line width. The default line width is
    \c 1.0. When antialiasing is enabled, the line widths
    under a single pixel automatically fade the opacity,
    creating a smooth output.
    \table
    \row
    \li \inlineimage canvas2d-linewidth.webp
    \li
    \code
    for (let i = 1; i < 10 ; i++) {
        let y = i * 20;
        ctx.lineWidth = 0.5 * i;
        ctx.beginPath();
        ctx.moveTo(20, y);
        ctx.bezierCurveTo(80, y + 20, 120,
                          y - 20, 180, y);
        ctx.stroke();
    }
    \endcode
    \endtable

    \sa stroke()
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

    Holds the current miter limit length. Miter limit controls when a sharp corner
    is beveled. When the corner length would become longer than this limit,
    a "bevel" \l lineJoin will be applied between the lines instead.
    This only has effect with the "miter" line join.
    The default limit is \c 10.0.

    \sa lineJoin
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

    Holds the current antialias amount. More antialias means smoother
    painting. This only affects fill and stroke painting, not images
    or texts.
    The default value is \c 1.0 and the maximum value is \c 10.0.

    Antialiasing can be modified per-path so it can be set before each stroke/fill.
    \table
    \row
    \li \inlineimage canvas2d-antialias.webp
    \li
    \code
    ctx.lineWidth = 6;
    for (let i = 1; i < 10 ; i++) {
        let y = i * 20;
        ctx.antialias = i;
        ctx.beginPath();
        ctx.moveTo(20, y);
        ctx.bezierCurveTo(80, y + 20, 120,
                          y - 20, 180, y);
        ctx.stroke();
    }
    \endcode
    \endtable

    \sa textAntialias
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

/*!
    \qmlproperty real Canvas2DContext::textAntialias

    Holds the current text antialias amount. The value is a
    multiplier to normal antialiasing, meaning that \c 0.0 disables
    antialiasing and \c 2.0 doubles it. The default value \c is 1.0.

    \note Due to the text antialiasing technique used (SDF),
    the maximum antialiasing amount is quite limited and this
    affects less when the font size is small.
    \table
    \row
    \li \inlineimage canvas2d-textantialias.webp
    \li
    \code
    ctx.font = "22px 'Titillium Web'";
    ctx.textAntialias = 1.0;
    ctx.fillText("Antialiasing: 1.0", 100, 25);
    ctx.textAntialias = 2.0;
    ctx.fillText("Antialiasing: 2.0", 100, 75);
    ctx.textAntialias = 3.0;
    ctx.fillText("Antialiasing: 3.0", 100, 125);
    ctx.textAntialias = 4.0;
    ctx.fillText("Antialiasing: 4.0", 100, 175);
    \endcode
    \endtable
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_textAntialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.textAntialias));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_textAntialias(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    qreal w = argc ? argv[0].toNumber() : -1;

    if (w >= 0 && qt_is_finite(w) && w != r->d()->context()->state.textAntialias) {
        r->d()->context()->state.textAntialias = w;
        r->d()->context()->buffer()->setTextAntialias(w);
    }
    RETURN_UNDEFINED();
}

/*!
    \qmlproperty real Canvas2DContext::textLineHeight

    Holds the current text line height adjustment amount
    in pixels. The default line height is \c 0.
    \table
    \row
    \li \inlineimage canvas2d-textlineheight.webp
    \li
    \code
    ctx.strokeRect(40, 5, 120, 60);
    ctx.strokeRect(40, 70, 120, 60);
    ctx.strokeRect(40, 135, 120, 60);
    ctx.textLineHeight = -10;
    ctx.fillText("Text with line height: -10",
                  40, 5, 120, 60);
    ctx.textLineHeight = 0;
    ctx.fillText("Text with line height: 0",
                  40, 70, 120, 60);
    ctx.textLineHeight = 10;
    ctx.fillText("Text with line height: 10",
                  40, 135, 120, 60);
    \endcode
    \endtable

*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_textLineHeight(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    RETURN_RESULT(QV4::Encode(r->d()->context()->state.textLineHeight));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_textLineHeight(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 0) {
        qreal h = argv[0].toNumber();

        if (qt_is_finite(h) && h != r->d()->context()->state.textLineHeight) {
            r->d()->context()->state.textLineHeight = h;
            r->d()->context()->buffer()->setTextLineHeight(h);
        }
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
    \qmlmethod object Canvas2DContext::clearRect(real x, real y, real width, real height)

    Erases the pixels in a rectangular area by filling the rectangle
    specified by \a x, \a y, \a width, \a height with transparent black.
    As clearing does not need blending, it can be faster than fillRect().
    \table
    \row
    \li \inlineimage canvas2d-clearrect.webp
    \li
    \code
    ctx.beginPath();
    ctx.circle(100, 100, 80);
    ctx.fill();
    ctx.stroke();
    ctx.clearRect(60, 0, 80, 120);
    \endcode
    \endtable
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
    \qmlmethod object Canvas2DContext::fillRect(real x, real y, real width, real height)

    Draws a filled rectangle into the specified position ( \a x, \a y) with size \a width, \a height.
    \note This is provided for convenience. When filling more than just a single rect,
    prefer using \l rect().
    \table
    \row
    \li \inlineimage canvas2d-fillrect.webp
    \li
    \code
    ctx.fillRect(20, 20, 160, 160);
    // The above code does same as:
    // ctx.beginPath();
    // ctx.rect(20, 20, 160, 160);
    // ctx.fill();
    \endcode
    \endtable

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
    \qmlmethod object Canvas2DContext::strokeRect(real x, real y, real width, real height)

    Draws a stroked rectangle into the specified position ( \a x, \a y) with size \a width, \a height.
    \note This is provided for convenience. When stroking more than just a single rect,
    prefer using rect().
    \table
    \row
    \li \inlineimage canvas2d-strokerect.webp
    \li
    \code
    ctx.strokeRect(20, 20, 160, 160);
    // The above code does same as:
    // ctx.beginPath();
    // ctx.rect(20, 20, 160, 160);
    // ctx.stroke();
    \endcode
    \endtable

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

    Begins drawing a new path while clearing the current path.
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

    Sets the current scissor rectangle to (\a x, \a y, \a width, \a height).
    The scissor rectangle is transformed by the current transform.
    \note Clipping has some performance cost and it should only be used
    when needed.
    \table
    \row
    \li \inlineimage canvas2d-cliprect.webp
    \li
    \code
    ctx.clipRect(20, 20, 160, 160);
    ctx.beginPath();
    ctx.circle(40, 40, 110);
    ctx.fill();
    ctx.fillStyle = "black";
    ctx.fillText("Clip me...", 45, 100);
    ctx.strokeRect(20, 20, 160, 160);
    \endcode
    \endtable

    \sa resetClipping()
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
    \sa clipRect()
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

    Fills the current path with the current fill style, and the current fill rule.
    \table
    \row
    \li \inlineimage canvas2d-fill.webp
    \li
    \code
    ctx.beginPath();
    ctx.rect(20, 20, 40, 160);
    ctx.rect(140, 20, 40, 160);
    ctx.circle(100, 100, 60);
    ctx.fill();
    \endcode
    \endtable

    \sa fillStyle, fillRule, {http://www.w3.org/TR/2dcontext/#dom-context-2d-fill}{W3C 2d context standard for fill}
*/
/*!
    \qmlmethod object Canvas2DContext::fill(string fillRule)

    Fills the current path with the current fill style, and fill rule \a fillRule.

    \sa fillStyle, fillRule
*/
/*!
    \qmlmethod object Canvas2DContext::fill(path2d path, int pathGroup)

    Fills the \a path with the current fill style and fill rule, and belonging
    into optional \a pathGroup. Painting through QCanvasPath is optimal
    when the path contains more commands and is mostly static.

    When \a pathGroup is \c -1, the path will not be cached on the GPU side.
    This is the default. To request caching of path data, pass a
    value equal or greater than \c 0. More information about using path
    cache groups can be found in the \l{QCanvasPath} documentation.

    Calling beginPath() before this method is not required.

    \table
    \row
    \li \inlineimage canvas2d-fill2.webp
    \li
    \code
    // myPath is path2d
    if (myPath.isEmpty()) {
        for (let i = 0; i < 16; i++) {
            let w = 100 + 60 * Math.sin(i);
            myPath.rect(100 - w * 0.5,
                        22 + i * 10,
                        w, 6);
        }
    }
    ctx.fill(myPath);
    \endcode
    \endtable

    \sa fillStyle, path2d
*/
/*!
    \qmlmethod object Canvas2DContext::fill(path2d path, string fillRule, int pathGroup)

    Fills the \a path with the current fill style and using \a fillRule.
    The cache group parameter \a pathGroup is optional.

    \sa fillStyle, fillRule, path2d
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_fill(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r);
    if (argc == 0) {
        r->d()->context()->fill();
    } else {
        QV4::ScopedValue arg1(scope, argv[0]);
        if (arg1->as<Object>()) {
            // fill a path
            QCanvasPath p = QV4::ExecutionEngine::toVariant(arg1, QMetaType::fromType<QCanvasPath>()).value<QCanvasPath>();
            if (argc >= 2) {
                QV4::ScopedValue arg2(scope, argv[1]);
                if (arg2->isInt32()) {
                    int pathGroup = arg2->toInteger();
                    r->d()->context()->fillPath(p, pathGroup);
                } else if (arg2->isString()) {
                    auto rule = qcanvas_fill_rule_from_string(arg2->toQString());
                    int pathGroup = -1;
                    if (argc >= 3) {
                        QV4::ScopedValue arg3(scope, argv[2]);
                        pathGroup = arg3->toInteger();
                    }
                    r->d()->context()->fillPath(p, rule, pathGroup);
                } else {
                    THROW_DOM(DOMEXCEPTION_NOT_SUPPORTED_ERR, "fill: Incorrect arguments")
                }
            } else {
                r->d()->context()->fillPath(p);
            }
        } else if (arg1->isString()) {
            // Fill with a fillRule
            auto rule = qcanvas_fill_rule_from_string(arg1->toQString());
            r->d()->context()->fill(rule);
        }
    }
    RETURN_RESULT(*thisObject);
}

/*!
    \qmlmethod object Canvas2DContext::stroke()

    Strokes the current path with the current stroke style.
    \table
    \row
    \li \inlineimage canvas2d-stroke.webp
    \li
    \code
    ctx.beginPath();
    ctx.rect(20, 20, 40, 160);
    ctx.rect(140, 20, 40, 160);
    ctx.circle(100, 100, 60);
    ctx.stroke();
    \endcode
    \endtable

    \sa strokeStyle, {http://www.w3.org/TR/2dcontext/#dom-context-2d-stroke}{W3C 2d context standard for stroke}
*/
/*!
    \qmlmethod object Canvas2DContext::stroke(path2d path, int pathGroup)

    Strokes the \a path with the current stroke style and belonging into
    \a pathGroup. Painting through QCanvasPath is optimal when the path
    contains more commands and is mostly static.

    When \a pathGroup is \c -1, the path's rendering-related data will not be
    cached. This is the default. To request caching of path data, pass a
    value equal or greater to \c 0. More information about using path cache
    groups can be found in the \l{QCanvasPath} documentation.

    Calling beginPath() before this method is not required.

    \table
    \row
    \li \inlineimage canvas2d-stroke2.webp
    \li
    \code
    // myPath is path2d
    if (myPath.isEmpty()) {
        for (let i = 0; i < 16; i++) {
            let h = 100 + 60 * Math.sin(i);
            myPath.rect(22 + i * 10,
                        180 - h,
                        6, h);
        }
    }
    ctx.stroke(myPath);
    \endcode
    \endtable

    \sa strokeStyle, path2d, {http://www.w3.org/TR/2dcontext/#dom-context-2d-stroke}{W3C 2d context standard for stroke}
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
        QV4::ScopedValue value(scope, argv[0]);
        if (value->as<Object>()) {
            QCanvasPath p = QV4::ExecutionEngine::toVariant(value, QMetaType::fromType<QCanvasPath>()).value<QCanvasPath>();
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
    the order they are shown above. In addition, a font family with spaces in
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

    \table
    \row
    \li \inlineimage canvas2d-textalign.webp
    \li
    \code
    ctx.font = "22px 'Titillium Web'";
    ctx.fillRect(100, 0, 1, 200);
    ctx.textAlign = "left";
    ctx.fillText("Left", 100, 40);
    ctx.textAlign = "center";
    ctx.fillText("Center", 100, 70);
    ctx.textAlign = "right";
    ctx.fillText("Right", 100, 100);
    ctx.textAlign = "start";
    ctx.fillText("Start", 100, 130);
    ctx.textAlign = "end";
    ctx.fillText("End", 100, 160);
    \endcode
    \endtable

    \sa textBaseline
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

    \table
    \row
    \li \inlineimage canvas2d-textbaseline.webp
    \li
    \code
    ctx.font = "22px 'Titillium Web'";
    ctx.fillRect(0, 60, 200, 1);
    ctx.fillRect(0, 140, 200, 1);
    ctx.textBaseline = "bottom";
    ctx.fillText("Bottom", 40, 60);
    ctx.textBaseline = "middle";
    ctx.fillText("Middle", 100, 60);
    ctx.textBaseline = "top";
    ctx.fillText("Top", 160, 60);
    ctx.textBaseline = "alphabetic";
    ctx.fillText("Alphabetic", 50, 140);
    ctx.textBaseline = "hanging";
    ctx.fillText("Hanging", 150, 140);
    \endcode
    \endtable

    \sa textAlign
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
    \qmlproperty string Canvas2DContext::textWrapMode

    Holds the current text wrap mode, so how the text is wrapped
    to multiple lines. The possible values are:

    \value "nowrap"        (default) No wrapping will be performed. If the text contains insufficient newlines, then contentWidth will exceed a set width.
    \value "wrap"          If possible, wrapping occurs at a word boundary; otherwise it will occur at the appropriate point on the line, even in the middle of a word.
    \value "wordwrap"      Wrapping is done on word boundaries only. If a word is too long, content width will exceed a set width.
    \value "wrapanywhere"  Wrapping is done at any point on a line, even if it occurs in the middle of a word.

    The default wrap mode is \c "nowrap".

    \table
    \row
    \li \inlineimage canvas2d-textwrapmode.webp
    \li
    \code
    ctx.strokeRect(50, 5, 100, 60);
    ctx.strokeRect(50, 70, 100, 60);
    ctx.strokeRect(50, 135, 100, 60);
    let s = "This is a long string.";
    ctx.textWrapMode = "nowrap";
    ctx.fillText(s, 50, 5, 100, 60);
    ctx.textWrapMode = "wrap";
    ctx.fillText(s, 50, 70, 100, 60);
    ctx.textWrapMode = "wrapanywhere";
    ctx.fillText(s, 50, 135, 100, 60);
    \endcode
    \endtable
*/
QV4::ReturnedValue QCanvasJSContext2D::method_get_textWrapMode(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    switch (r->d()->context()->state.textWrapMode) {
    case QCanvasPainter::WrapMode::WordWrap:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("wordwrap")));
    case QCanvasPainter::WrapMode::Wrap:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("wrap")));
    case QCanvasPainter::WrapMode::WrapAnywhere:
        RETURN_RESULT(scope.engine->newString(QStringLiteral("wrapanywhere")));
    case QCanvasPainter::WrapMode::NoWrap:
    default:
        break;
    }
    RETURN_RESULT(scope.engine->newString(QStringLiteral("nowrap")));
}

QV4::ReturnedValue QCanvasJSContext2D::method_set_textWrapMode(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)
    QV4::ScopedString s(scope, argc ? argv[0] : QV4::Value::undefinedValue(), QV4::ScopedString::Convert);
    if (scope.hasException())
        RETURN_UNDEFINED();
    QString textWrapMode = s->toQString();

    QCanvasPainter::WrapMode tw;
    if (textWrapMode == QStringLiteral("nowrap"))
        tw = QCanvasPainter::WrapMode::NoWrap;
    else if (textWrapMode == QStringLiteral("wordwrap"))
        tw = QCanvasPainter::WrapMode::WordWrap;
    else if (textWrapMode == QStringLiteral("wrap"))
        tw = QCanvasPainter::WrapMode::Wrap;
    else if (textWrapMode == QStringLiteral("wrapanywhere"))
        tw = QCanvasPainter::WrapMode::WrapAnywhere;
    else
        RETURN_UNDEFINED();

    r->d()->context()->buffer()->setTextWrapMode(tw);

    if (tw != r->d()->context()->state.textWrapMode)
        r->d()->context()->state.textWrapMode = tw;

    RETURN_UNDEFINED();
}

/*!
    \qmlmethod object Canvas2DContext::fillText(text, x, y, maxWidth)

    Draws \a text string at specified location (\a x, \a y), with current textAlign and textBaseline.
    To make the text wrap into multiple lines, set optional \a maxWidth parameter to preferred
    row width in pixels. White space is stripped at the beginning of the rows,
    the text is split at word boundaries or when new-line characters are encountered.
    Words longer than the max width are split at nearest character (i.e. no hyphenation).

    \sa font, textAlign, textBaseline, textWrapMode, strokeText
*/
/*!
    \qmlmethod object Canvas2DContext::fillText(text, x, y, width, height)

    Draws \a text string inside rect (\a x, \a y, \a width, \a height), with
    current textAlign and textBaseline. Width of the rect parameter is used
    as maxWidth.

    It is often useful to set the text baseline to \l TextBaseline::Top or
    \l TextBaseline::Middle when painting text with this method.

    \sa font, textAlign, textBaseline, textWrapMode, strokeText
*/
QV4::ReturnedValue QCanvasJSContext2DPrototype::method_fillText(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QCanvasJSContext2D> r(scope, *thisObject);
    CHECK_CONTEXT(r)

    if (argc >= 5) {
        qreal x = argv[1].toNumber();
        qreal y = argv[2].toNumber();
        qreal width = argv[3].toNumber();
        qreal height = argv[4].toNumber();
        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(width) || !qt_is_finite(height))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->drawTextRect(argv[0].toQStringNoThrow(), x, y, width, height);
    } else if (argc >= 3) {
        qreal x = argv[1].toNumber();
        qreal y = argv[2].toNumber();
        qreal maxWidth = -1;
        if (argc >= 4) maxWidth = argv[3].toNumber();
        if (!qt_is_finite(x) || !qt_is_finite(y) || !qt_is_finite(maxWidth))
            RETURN_UNDEFINED();

        r->d()->context()->buffer()->drawText(argv[0].toQStringNoThrow(), x, y, maxWidth);
    }

    RETURN_RESULT(*thisObject);
}
/*!
    \qmlmethod object Canvas2DContext::strokeText(text, x, y)

    Strokes the given \a text at a position specified by (\a x, \a y).

    \sa font, textAlign, textBaseline, fillText
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
    This drawing is subject to the current context clip path.

    \table
    \row
    \li \inlineimage canvas2d-drawimage.webp
    \li
    \code
    ctx.drawImage("qt_logo.png", 36, 36);
    \endcode
    \endtable

    \sa Image, Canvas2D::loadImage, Canvas2D::isImageLoaded, Canvas2D::imageLoaded
    \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-drawimage}{W3C 2d context standard for drawImage}
*/
/*!
    \qmlmethod object Canvas2DContext::drawImage(variant image, real dx, real dy, real dw, real dh)
    This is an overloaded function.
    Draws the given item as \a image onto the canvas at point (\a dx, \a dy) and with width \a dw,
    height \a dh.

    \table
    \row
    \li \inlineimage canvas2d-drawimage2.webp
    \li
    \code
    ctx.drawImage("qt_logo.png", 50, 0, 100, 200);
    \endcode
    \endtable

    \sa Image, Canvas2D::loadImage(), Canvas2D::isImageLoaded, Canvas2D::imageLoaded
    \sa {http://www.w3.org/TR/2dcontext/#dom-context-2d-drawimage}{W3C 2d context standard for drawImage}
*/
/*!
    \qmlmethod object Canvas2DContext::drawImage(variant image, real sx, real sy, real sw, real sh, real dx, real dy, real dw, real dh)
    This is an overloaded function.
    Draws the given item as \a image from source point (\a sx, \a sy) and source width \a sw, source height \a sh
    onto the canvas at point (\a dx, \a dy) and with width \a dw, height \a dh.

    \table
    \row
    \li \inlineimage canvas2d-drawimage3.webp
    \li
    \code
    ctx.drawImage("qt_logo.png",
                   20, 30, 54, 76,
                   0, 0, 200, 200);
    \endcode
    \endtable

    \sa Image, Canvas2D::loadImage(), Canvas2D::isImageLoaded, Canvas2D::imageLoaded
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

void QCanvas2DContext::fill(QCanvasPainter::FillRule fillRule)
{
    buffer()->fill(fillRule);
}

void QCanvas2DContext::stroke()
{
    buffer()->stroke();
}

void QCanvas2DContext::fillPath(const QCanvasPath &path, int pathGroup)
{
    buffer()->fillPath(path, pathGroup);
}

void QCanvas2DContext::fillPath(const QCanvasPath &path, QCanvasPainter::FillRule fillRule, int pathGroup)
{
    buffer()->fillPath(path, fillRule, pathGroup);
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

void QCanvas2DContext::addImagePattern(const QCanvasImagePattern &pattern, const QString &url, const QImage &image, QCanvasPainter::ImageFlags flags)
{
    m_canvas->addImagePattern(pattern, url, image, flags);
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
    proto->defineAccessorProperty(QStringLiteral("textWrapMode"), QCanvasJSContext2D::method_get_textWrapMode, QCanvasJSContext2D::method_set_textWrapMode);
    proto->defineAccessorProperty(QStringLiteral("lineJoin"), QCanvasJSContext2D::method_get_lineJoin, QCanvasJSContext2D::method_set_lineJoin);
    proto->defineAccessorProperty(QStringLiteral("lineWidth"), QCanvasJSContext2D::method_get_lineWidth, QCanvasJSContext2D::method_set_lineWidth);
    proto->defineAccessorProperty(QStringLiteral("textAlign"), QCanvasJSContext2D::method_get_textAlign, QCanvasJSContext2D::method_set_textAlign);
    proto->defineAccessorProperty(QStringLiteral("lineDashOffset"), QCanvasJSContext2D::method_get_lineDashOffset, QCanvasJSContext2D::method_set_lineDashOffset);
    proto->defineAccessorProperty(QStringLiteral("antialias"), QCanvasJSContext2D::method_get_antialias, QCanvasJSContext2D::method_set_antialias);
    proto->defineAccessorProperty(QStringLiteral("textAntialias"), QCanvasJSContext2D::method_get_textAntialias, QCanvasJSContext2D::method_set_textAntialias);
    proto->defineAccessorProperty(QStringLiteral("textLineHeight"), QCanvasJSContext2D::method_get_textLineHeight, QCanvasJSContext2D::method_set_textLineHeight);
    contextPrototype = proto;
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

    if (newState.fillStyle.type() != QCanvasBrush::BrushType::Invalid && newState.fillStyle != state.fillStyle)
        buffer()->setFillStyle(newState.fillStyle);

    if (newState.strokeStyle.type() != QCanvasBrush::BrushType::Invalid && newState.strokeStyle != state.strokeStyle)
        buffer()->setStrokeStyle(newState.strokeStyle);

    if (newState.fillColor != state.fillColor)
        buffer()->setFillColor(newState.fillColor);

    if (newState.strokeColor != state.strokeColor)
        buffer()->setStrokeColor(newState.strokeColor);

    if (newState.lineWidth != state.lineWidth)
        buffer()->setLineWidth(newState.lineWidth);

    if (newState.antialias != state.antialias)
        buffer()->setAntialias(newState.antialias);

    if (newState.textAntialias != state.textAntialias)
        buffer()->setTextAntialias(newState.textAntialias);

    if (newState.textLineHeight != state.textLineHeight)
        buffer()->setTextLineHeight(newState.textLineHeight);

    if (newState.textBaseline != state.textBaseline)
        buffer()->setTextBaseline(newState.textBaseline);

    if (newState.textWrapMode != state.textWrapMode)
        buffer()->setTextWrapMode(newState.textWrapMode);

    if (newState.lineCap != state.lineCap)
        buffer()->setLineCap(newState.lineCap);

    if (newState.lineJoin != state.lineJoin)
        buffer()->setLineJoin(newState.lineJoin);

    if (newState.pathWinding != state.pathWinding)
        buffer()->setPathWinding(newState.pathWinding);

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
