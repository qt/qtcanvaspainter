// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvascustombrush.h"
#include "engine/qcpainterengineutils_p.h"
#include "qcanvascustombrush_p.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>

QT_BEGIN_NAMESPACE


static QShader getCustomShader(const QString &name)
{
    if (name.isEmpty())
        return QShader();

    QFile f(name);
    if (f.open(QIODevice::ReadOnly)) {
        return QShader::fromSerialized(f.readAll());
    } else {
        qWarning() << "Unable to create custom brush with shader:" << name;
        return QShader();
    }
}

/*!
    \class QCanvasCustomBrush
    \since 6.11
    \preliminary
    \brief QCanvasCustomBrush is a brush with custom shaders.
    \inmodule QtCanvasPainter

    QCanvasCustomBrush is a stroke/fill brush with custom vertex and/or fragment shaders.

    These shaders are expected to be written in Vulkan-style GLSL, similarly to
    Qt Quick \l ShaderEffect shaders. They must always contain a \c QC_INCLUDE
    statement, either with \c{"customfrag.glsl"} or \c{"customvert.glsl"}. This
    makes available a uniform block, the image and font textures, and a few
    helper functions.

    \c iTime is an example of a commonly used member in the built-in uniform
    block. Calling setAnimationRunning() with \c true will make this value update
    automatically every frame, and can be used to drive animated content.

    \section1 Built-in Shader Inputs, Uniforms, and Helper Functions

    The \c QC_INCLUDE statement is not a standard preprocessor directive. It is
    handled by qt_add_custom_brush_shaders() at build time, before the shader is
    passed to the regular shader compilation pipeline. The statement is replaced
    by a block of GLSL source code that declares the shader inputs and outputs,
    the texture samplers, a shared uniform block, and, for fragment shaders, a
    set of helper functions. Use \c{"customvert.glsl"} in vertex shaders and
    \c{"customfrag.glsl"} in fragment shaders.

    This means that a custom brush shader does not declare these inputs,
    outputs, samplers, or uniforms itself; they are all made available by the
    \c QC_INCLUDE statement.

    \section2 Vertex Shader Interface

    A vertex shader that includes \c{"customvert.glsl"} has the following inputs
    and outputs declared:

    \list
    \li \c{in vec2 vertex} - The vertex position in the canvas coordinate
        system.
    \li \c{in vec2 tcoord} - The texture coordinate associated with the vertex.
    \li \c{out vec2 texCoord} - Forwarded to the fragment shader. Typically set
        to \c tcoord.
    \li \c{out vec2 fragCoord} - Forwarded to the fragment shader. Typically set
        to \c vertex.
    \endlist

    \note These variables are available implicitly via the \c QC_INCLUDE
    directive. The vertex shader snippet itself must not declare them.

    In addition, the following transformation-related uniforms are available:

    \list
    \li \c{vec4 viewRect} - The viewport rectangle, as (x, y, width, height).
    \li \c{int ndcIsYDown} - Non-zero when the normalized device coordinate
        system has its Y axis pointing downwards, as is the case with some
        graphics APIs. Take this into account when computing \c gl_Position.
    \li \c{mat3 vertMatrix} - The current transformation matrix.
    \endlist

    A vertex shader must write \c gl_Position, and is expected to forward
    \c texCoord and \c fragCoord to the fragment stage.

    \note Custom vertex shaders are less common. Most custom brushes are
    expected to use the default, built-in vertex shader in combination with a
    custom, application-provided fragment shader.

    A typical vertex shader looks like this:

    \code
        #version 440

        QC_INCLUDE "customvert.glsl"

        void main()
        {
            texCoord = tcoord;
            fragCoord = vertex;
            vec2 v = (vertMatrix * vec3(vertex, 1.0)).xy;
            if (ndcIsYDown != 0)
                gl_Position = vec4(2.0 * (v.x + viewRect.x) / viewRect.z - 1.0,
                                   -1.0 + 2.0 * (v.y + viewRect.y) / viewRect.w, 0.0, 1.0);
            else
                gl_Position = vec4(2.0 * (v.x + viewRect.x) / viewRect.z - 1.0,
                                   1.0 - 2.0 * (v.y + viewRect.y) / viewRect.w, 0.0, 1.0);
        }
    \endcode

    \section2 Fragment Shader Interface

    A fragment shader that includes \c{"customfrag.glsl"} has the following
    inputs and output declared:

    \list
    \li \c{in vec2 texCoord} - The interpolated texture coordinate.
    \li \c{in vec2 fragCoord} - The interpolated fragment position, in the same
        coordinate system as the geometry. Commonly used to drive procedural
        effects.
    \li \c{out vec4 fragColor} - The resulting fragment color, which the shader
        must write. The expected output uses premultiplied alpha.
    \endlist

    \note These variables are available implicitly via the \c QC_INCLUDE
    directive. The fragment shader snippet itself must not declare them.

    Two texture samplers are available:

    \list
    \li \c{sampler2D tex} - The image texture.
    \li \c{sampler2D fontTex} - The font texture, holding a signed distance
        field of the glyphs. Relevant when the brush is used to fill text.
    \endlist

    The convenience constants \c TAU (equal to 2 * pi) and \c SQRT2 are also
    defined.

    \section2 Common Uniforms

    Both vertex and fragment shaders that use \c QC_INCLUDE have access to a
    shared uniform block. The most commonly used members are:

    \list
    \li \c{float iTime} - A time value, in seconds, that is updated every frame
        while isAnimationRunning() is \c true. Use it to drive animations. See
        setAnimationRunning().
    \li \c{vec4 data1}, \c{vec4 data2}, \c{vec4 data3}, \c{vec4 data4} - Custom
        data exposed to the shader. Set these from C++ via setData1(),
        setData2(), setData3(), and setData4().
    \li \c{float globalAlpha} - The painter's current global opacity. Fragment
        shaders should normally multiply \c fragColor by this value.
    \li \c{vec4 colorEffects} - The active color effect parameters. Normally
        applied through applyColorEffects() rather than accessed directly.
    \li \c{float fontAlphaMin}, \c{float fontAlphaMax} - The signed distance
        field thresholds used when antialiasing glyphs.
    \endlist

    \section2 Fragment Shader Helper Functions

    The fragment shader include provides the following helper functions:

    \list
    \li \c{float clipMask()} - Returns the clip (scissor) coverage, in the
        [0, 1] range, for the current fragment. Multiply \c fragColor by this
        value to honor the painter's clipping.
    \li \c{float antialiasingAlpha()} - Returns the antialiasing coverage, in
        the [0, 1] range, derived from \c texCoord. Multiply \c fragColor by
        this value to get antialiased edges.
    \li \c{float sdfFontAlphaRaw()} - Returns the raw signed distance field
        value sampled from \c fontTex at \c texCoord, without antialiasing.
        Apply \c smoothstep() manually as needed.
    \li \c{float sdfFontAlpha()} - Returns the glyph alpha sampled from
        \c fontTex at \c texCoord, with the default antialiasing applied based
        on \c fontAlphaMin and \c fontAlphaMax.
    \li \c{void applyColorEffects(inout vec4 color)} - Applies the active
        contrast, brightness, and saturation effects to \c color in place.
    \endlist

    A typical fragment shader computes \c fragColor, multiplies it by
    \c globalAlpha, optionally multiplies by \c clipMask() and
    \c antialiasingAlpha() to support clipping and antialiasing, and finally
    calls applyColorEffects().

    When text is involved, \c sdfFontAlpha() should be taken into account too. For
    example:

    \code
        #version 440

        QC_INCLUDE "customfrag.glsl"

        void main()
        {
            float a = 0.6 + 0.2 * sin(0.1 * fragCoord.x + 4.0 * iTime);
            vec4 color = vec4(a, a, a, 1.0);
            fragColor = sdfFontAlpha() * globalAlpha * color;
            applyColorEffects(fragColor);
        }
    \endcode

    \section1 Adding the Shaders to the Project

    Shaders that are used with QCanvasCustomBrush must always be added to the
    application project via the \l qt_add_custom_brush_shaders CMake function, provided by
    the Qt Canvas Painter package. This function performs additional
    preprocessing at build time before internally invoking the standard \c
    qt_add_shaders().

    For example:
    \code
        qt_add_custom_brush_shaders(app "app_custombrush_shaders"
            PREFIX
                "/shaders"
            FILES
                brush1.frag
        )
    \endcode

    \section1 Using the Brush

    At run time, the generated \c{.qsb} file can be used for example like this:
    \code
        QCanvasCustomBrush customBrush(":/shaders/brush1.frag.qsb"));
        customBrush.setAnimationRunning(true); // iTime updates automatically
        // expose custom data to the shader in data1
        customBrush.setData1(QVector4D(1.0, 2.0, 3.0, 4.0));
    \endcode

    The QCanvasCustomBrush can then be used in a fill, for example:
    \code
        painter->setFillStyle(customBrush);
    \endcode

    See \l qt_add_custom_brush_shaders for the details of the CMake function, and \l{Qt
    Shader Tools}{the Qt Shader Tools module documentation} for working with
    cross-platform shader code in Qt.

    \note Shaders for custom brushes must always contain the \c QC_INCLUDE
    statement and must be added to the project via the \l qt_add_custom_brush_shaders CMake
    function. qt_add_shaders() is not suitable for custom brush shaders.

    \note qt_add_custom_brush_shaders translates the shader code to the following targets:
    GLSL \c{300 es}, \c 150, \c 130, HLSL \c{5.0}, and MSL \c{1.2}. There is
    currently no further configurability offered for this.

    \sa qt_add_custom_brush_shaders, {Qt Canvas Painter - Gallery Example}
*/

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanvasCustomBrushPrivate)

QCanvasCustomBrush::QCanvasCustomBrush(QCanvasCustomBrushPrivate *p)
    : d(p)
{
}

void QCanvasCustomBrush::detach()
{
    if (d)
        d.detach();
}

/*!
    Constructs a default custom brush.
*/

QCanvasCustomBrush::QCanvasCustomBrush()
    : d(new QCanvasCustomBrushPrivate)
{
}

/*!
    Destroys the custom brush.
*/

QCanvasCustomBrush::~QCanvasCustomBrush() = default;

QCanvasCustomBrush::QCanvasCustomBrush(const QCanvasCustomBrush &) = default;
QCanvasCustomBrush &QCanvasCustomBrush::operator=(const QCanvasCustomBrush &) = default;

QCanvasCustomBrush::operator QCanvasBrush() const
{
    return QCanvasBrushPrivate::create(d.get());
}

template<> QCanvasCustomBrush QCanvasBrush::as<QCanvasCustomBrush>() const
{
    Q_ASSERT(type() == BrushType::Custom);
    return QCanvasCustomBrushPrivate::create(
        static_cast<QCanvasCustomBrushPrivate *>(QCanvasBrushPrivate::get(*this)));
}

/*!
   Returns the custom brush as a QVariant.
*/

QCanvasCustomBrush::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasCustomBrush::operator!=(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs)

    \return \c true if the custom brush \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasCustomBrush::operator==(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs)

    \return \c true if the custom brush \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs) noexcept
{
    auto *d = QCanvasCustomBrushPrivate::get(&lhs);
    auto *bd = QCanvasCustomBrushPrivate::get(&rhs);
    if (bd == d)
        return true;

    return d->equals(*bd);
}

bool QCanvasCustomBrushPrivate::equals(const QCanvasBrushPrivate &other) const noexcept
{
    Q_ASSERT(other.type == type);
    const auto &bd = static_cast<const QCanvasCustomBrushPrivate &>(other);

    if (fragmentShader != bd.fragmentShader
        || vertexShader != bd.vertexShader
        || timeRunning != bd.timeRunning
        || time != bd.time
        || data[0] != bd.data[0]
        || data[1] != bd.data[1]
        || data[2] != bd.data[2]
        || data[3] != bd.data[3])
        return false;

    return true;
}

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasCustomBrush &brush)
    \relates QCanvasCustomBrush

    Writes the given \a brush to the given \a stream and returns a
    reference to the \a stream.

    \note This function serializes the shaders loaded from the .qsb files, not
    the filenames.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCanvasCustomBrush &b)
{
    // Avoid serializing null QShaders to preserve equality after writing and
    // reading back a custom brush to/from a stream. This is because a QShader
    // returned by fromSerialized() is never "null".
    s << b.d->fragmentShader.isValid();
    if (b.d->fragmentShader.isValid())
        s << b.d->fragmentShader.serialized();
    s << b.d->vertexShader.isValid();
    if (b.d->vertexShader.isValid())
        s << b.d->vertexShader.serialized();
    s << b.d->timeRunning;
    s << b.d->time;
    for (size_t i = 0; i < std::size(b.d->data); ++i)
        s << b.d->data[i];
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasCustomBrush &brush)
    \relates QCanvasCustomBrush

    Reads the given \a brush from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCanvasCustomBrush &b)
{
    b.detach();
    b.d->fragmentShader = {};
    b.d->vertexShader = {};
    bool hasFragmentShader, hasVertexShader;
    s >> hasFragmentShader;
    if (hasFragmentShader) {
        QByteArray fragmentShaderData;
        s >> fragmentShaderData;
        b.d->fragmentShader = QShader::fromSerialized(fragmentShaderData);
    }
    s >> hasVertexShader;
    if (hasVertexShader) {
        QByteArray vertexShaderData;
        s >> vertexShaderData;
        b.d->vertexShader = QShader::fromSerialized(vertexShaderData);
    }
    s >> b.d->timeRunning;
    s >> b.d->time;
    for (size_t i = 0; i < std::size(b.d->data); ++i)
        s >> b.d->data[i];
    return s;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasCustomBrush &)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCanvasCustomBrush()";
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM


/*!
    Sets the custom brush to use \a fragmentShader.
    This must be path to a valid qsb file. The file can be a local file or
    embedded in the application via the \l{The Qt Resource System}.
*/

void QCanvasCustomBrush::setFragmentShader(const QString &fragmentShader)
{
    detach();
    d->fragmentShader = getCustomShader(fragmentShader);
}

/*!
    Sets the custom brush to use \a fragmentShader.
*/

void QCanvasCustomBrush::setFragmentShader(const QShader &fragmentShader)
{
    detach();
    d->fragmentShader = fragmentShader;
}

/*!
    Sets the custom brush to use \a vertexShader.
    This must be path to a valid qsb file. The file can be a local file or
    embedded in the application via the \l{The Qt Resource System}.
*/

void QCanvasCustomBrush::setVertexShader(const QString &vertexShader)
{
    detach();
    d->vertexShader = getCustomShader(vertexShader);
}

/*!
    Sets the custom brush to use \a vertexShader.
*/

void QCanvasCustomBrush::setVertexShader(const QShader &vertexShader)
{
    detach();
    d->vertexShader = vertexShader;
}

/*!
    Returns true if the time is running.
*/

bool QCanvasCustomBrush::isAnimationRunning() const
{
    return d->timeRunning;
}

/*!
    Sets the time running state to \a running. When this is true, the shader
    uniform \c iTime is updated automatically, and can be used to get the
    current animation running time in the shader.

    The default value is \c false.
*/

void QCanvasCustomBrush::setAnimationRunning(bool running)
{
    detach();
    d->timeRunning = running;
}

/*!
    Sets the uniform data1 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData1(const QVector4D &data)
{
    detach();
    d->data[0] = data;
}

/*!
    Sets the uniform data2 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData2(const QVector4D &data)
{
    detach();
    d->data[1] = data;
}

/*!
    Sets the uniform data3 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData3(const QVector4D &data)
{
    detach();
    d->data[2] = data;
}

/*!
    Sets the uniform data4 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData4(const QVector4D &data)
{
    detach();
    d->data[3] = data;
}

QCanvasBrushPrivate *QCanvasCustomBrushPrivate::clone()
{
    return new QCanvasCustomBrushPrivate(*this);
}

QCPaint QCanvasCustomBrushPrivate::createPaint(QCanvasPainter *) const
{
    QCPaint empty;
    return empty;
}

QT_END_NAMESPACE
