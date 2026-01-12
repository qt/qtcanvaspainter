// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCPAINTERRHIRENDERER_P_H
#define QCPAINTERRHIRENDERER_P_H

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

#include <rhi/qrhi.h>
#include "qcpainterengineutils_p.h"
#include "qccustombrush_p.h"
#include "qcoffscreencanvas.h"
#include <functional>

QT_BEGIN_NAMESPACE

class QCPainterEngine;
struct QCRHIContext;
class QCPainter;
class QCCustomBrush;
struct QCRHIPipelineStateKey;
struct QCRHISamplerDesc;
struct QCRHICall;
struct QCRHICommonUniforms;
struct QCRHICachedPath;
struct QCRHICachedPathGroup;
class QCPainterPath;

struct QCRHITexture
{
    QRhiTexture *tex = nullptr;
    int id = 0;
    int width = 0;
    int height = 0;
    QCPainter::ImageFlags flags = {};
};

struct QCRhiCanvas
{
    QRhiTexture *tex = nullptr;
    QRhiRenderBuffer *msaaColorBuffer = nullptr;
    QRhiRenderBuffer *ds = nullptr;
    QRhiTextureRenderTarget *rt = nullptr;
    QRhiRenderPassDescriptor *rp = nullptr;
    QCOffscreenCanvas::Flags flags;
    bool isNull() const { return !tex; }
    void destroy();
};

bool operator==(const QCRhiCanvas &a, const QCRhiCanvas &b) noexcept;
bool operator!=(const QCRhiCanvas &a, const QCRhiCanvas &b) noexcept;

class QCPainterRhiRenderer
{
public:
    enum RenderFlag {
        Antialiasing = 1 << 0,
        StencilStrokes = 1 << 1,
        TransformedClipping = 1 << 3,
        SimpleClipping = 1 << 4,
    };
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    QCPainterRhiRenderer();
    ~QCPainterRhiRenderer();
    void create(QRhi *rhi, QCPainter *painter);
    void destroy();
    bool isValid() const { return ctx != nullptr; }

    QCPainterEngine *engine() { return m_e; }

    // beginPrepare() and endPrepare() must be called outside a render pass.
    //
    // If devicePixelRatio is 0, rt->devicePixelRatio() is used. Specifying an
    // override is relevant when the target is a texture that is independent
    // from any on-screen window: rt's dpr is 1 then, but we may still want to
    // take an on-screen widget or window's dpr into account. (because we know
    // that the texture is eventually used in that window for something)
    //
    void beginPrepare(QRhiCommandBuffer *cb,
                      QRhiRenderTarget *rt,
                      float devicePixelRatio = 0.0f);
    void endPrepare();

    // Conveniences doing both beginPrepare and engine->beginPaint
    void beginPrepareAndPaint(QRhiCommandBuffer *cb,
                              QRhiRenderTarget *rt);
    void beginPrepareAndPaint(QRhiCommandBuffer *cb,
                              QRhiRenderTarget *rt,
                              float logicalWidth, float logicalHeight, float dpr);
    void endPrepareAndPaint();

    // then when recording the render pass, call render()
    void render();

    void resetForNewFrame(); // to be called once for the whole frame
    void resetForPass(); // called by beginPrepare()

    bool hasDrawCalls() const;

    void setFlag(RenderFlags flag, bool enable);

    bool renderCreate();

    QCRHITexture *renderCreateNativeTexture(QRhiTexture *texture, QCPainter::ImageFlags flags = {});
    QCRHITexture *renderUpdateNativeTexture(QRhiTexture *oldTexture, QRhiTexture *texture);
    bool isOffscreenCanvasYUp() const;

    int renderCreateTexture(QCTextureFormat type, int w, int h, QCPainter::ImageFlags imageFlags, const uchar* data);
    bool renderDeleteTexture(int image);
    bool renderUpdateTexture(int image, int x, int y, int w, int h, const uchar* data);
    void setViewport(float x, float y, float width, float height);
    void renderFill(const QCPaint &paint, const QCState &state,
                    const QRectF &bounds,
                    const QCPaths &paths, int pathsCount,
                    QCPainterPath *painterPath, int pathGroup,
                    const QTransform &pathTransform);
    void renderStroke(const QCPaint &paint, const QCState &state,
                      float strokeWidth,
                      const QCPaths &paths, int pathsCount,
                      QCPainterPath *painterPath, int pathGroup,
                      const QTransform &pathTransform);
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    void renderTextFill(
        const QCPaint &paint,
        const QCState &state,
        const QCRhiDistanceFieldGlyphCache::VertexList &verts,
        const QCRhiDistanceFieldGlyphCache::IndexList &indices);
    void renderTextFillCustom(
        const QCPaint &paint,
        const QCState &state,
        QCCustomBrush *brush,
        const QCRhiDistanceFieldGlyphCache::VertexList &verts,
        const QCRhiDistanceFieldGlyphCache::IndexList &indices);
    int populateFont(
        const QFont &font,
        const QRectF &rect,
        const QString &text,
        QCRhiDistanceFieldGlyphCache::VertexList &vertices,
        QCRhiDistanceFieldGlyphCache::IndexList &indices,
        int *textureWidth,
        int *textureHeight);
#endif
    void renderDelete();

    bool isPathCached(QCPainterPath *path, int pathGroup) const;
    void removePathGroup(int pathGroup);

    static void textureFormatInfo(QRhiTexture::Format format, QSize size,
                                  quint32 *bpl, quint32 *byteSize, quint32 *bytesPerPixel);

    QCOffscreenCanvas createCanvas(QSize pixelSize, int sampleCount, QCOffscreenCanvas::Flags flags);
    void destroyCanvas(QCOffscreenCanvas &canvas);
    QRhiRenderTarget *canvasRenderTarget(const QCOffscreenCanvas &canvas);
    void recordCanvasRenderPass(QRhiCommandBuffer *cb, const QCOffscreenCanvas &canvas);
    void grabCanvas(const QCOffscreenCanvas &canvas, std::function<void(const QImage &)> callback, QRhiCommandBuffer *maybeCb);

    void recordRenderPass(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QColor &clearColor);

private:
    QRhiGraphicsPipeline *pipeline(const QCRHIPipelineStateKey &key,
                                   QRhiRenderPassDescriptor *rpDesc,
                                   QRhiShaderResourceBindings *srb);
    QRhiSampler *sampler(const QCRHISamplerDesc &samplerDescription);
    QRhiResourceUpdateBatch *resourceUpdateBatch();
    void commitResourceUpdates();
    QCRHITexture *findTexture(QRhiTexture *texture) const;
    QCRHITexture *findTexture(int id) const;
    QRhiShaderResourceBindings *createSrb(int brushImage, int fontImage);
    void updateVertUniforms(QCRHICall *call, const QTransform &transform);

    QCRHICall *allocCall();
    QCRHITexture *allocTexture();
    int allocPaths(int count);
    int allocVerts(int count);
    int allocIndices(int count);
    int allocCommonUniforms(int count);

    QCRHICommonUniforms *uniformPtr(int i) const;
    QCCustomBrushPrivate::CommonUniforms *customUniformPtr(int i) const;
    void preparePaint(QCRHICommonUniforms* frag, const QCPaint &paint,
                      const QCState &state, float width, float aa, float strokeThr,
                      float fontAlphaMin, float fontAlphaMax);
    void prepareCustomPaint(QCCustomBrushPrivate::CommonUniforms* frag, const QCPaint &paint,
                            QCCustomBrush *brush, const QCState &state,
                            float width, float aa, float strokeThr,
                            float fontAlphaMin, float fontAlphaMax);
    void bindPipeline(QCRHICall *call,
                      int pipelineIndex, int srbIndex,
                      const QRhiCommandBuffer::DynamicOffset &vertDynamicOffset,
                      const QRhiCommandBuffer::DynamicOffset &dynamicOffset,
                      bool indexedDraw,
                      bool *needsViewport);

private:
    friend class QCPainter;
    friend class QCPainterPrivate;

    QCContext* createRhiContext(QRhi *rhi);

    QCContext *ctx = nullptr;
    QCRHIContext *rhiCtx = nullptr;
    QCPainter *m_painter = nullptr;
    QCPainterEngine *m_e = nullptr;
    QVector<QCRhiCanvas> m_canvases;

    QVector<std::pair<QRhiReadbackResult, std::function<void(const QImage &)>>> m_canvasGrabs;

    // Log draw calls and triangle amounts for each painting type.
    int logFillDrawCallCount = 0;
    int logStrokeDrawCallCount = 0;
    int logTextDrawCallCount = 0;
    int logFillTriCount = 0;
    int logStrokeTriCount = 0;
    int logTextTriCount = 0;
};

QT_END_NAMESPACE

#endif // QCPAINTERRHIRENDERER_P_H
