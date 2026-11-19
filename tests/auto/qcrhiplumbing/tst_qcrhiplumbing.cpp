// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Frame capture needs qtbase built with the graphicsframecapture feature
// enabled. The feature flag is private, so cannot test for it in an
// application. Just enable it manually, when this is wanted.
// May cause warnings and some test cases failing, that is as expected.
//#define FRAME_CAPTURE

#include <QTest>
#include <algorithm>
#include <memory>
#include <rhi/qrhi.h>
#include <QFont>
#include <QRawFont>
#include <QGlyphRun>
#include <QtCore/qset.h>
#include <QTextLayout>

#ifdef FRAME_CAPTURE
#include <QtGui/private/qgraphicsframecapture_p.h>
#endif

#include "qcanvaspainter.h"
#include "qcanvaspainterfactory.h"
#include "qcanvasrhipaintdriver.h"
#include "qcanvasimagepattern.h"
#include "qcanvaspath.h"
#include <QtCanvasPainter/private/qcanvaspainter_p.h>
#include <QtCanvasPainter/private/qcpainterengine_p.h>
#include <QtCanvasPainter/private/qcpainterrhirenderer_p.h>

#if QT_CONFIG(opengl)
#include <QOffscreenSurface>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#define TST_GL
#endif

#if QT_CONFIG(vulkan)
#include <QVulkanInstance>
#define TST_VK
#endif

#ifdef Q_OS_WIN
#define TST_D3D11
#define TST_D3D12
#endif

#if QT_CONFIG(metal)
#define TST_MTL
#endif

Q_DECLARE_METATYPE(QRhi::Implementation)
Q_DECLARE_METATYPE(QRhiInitParams *)

class tst_CanvasRhiRendering : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void rhiTestData();
    void create_data();
    void create();
    void createShared_data();
    void createShared();
    void render_data();
    void render();
    void renderWithDepthTest_data();
    void renderWithDepthTest();
    void canvasRender_data();
    void canvasRender();
    void canvasGrab_data();
    void canvasGrab();
    void canvasGrabContext_data();
    void canvasGrabContext();
    void canvasRenderMipMap_data();
    void canvasRenderMipMap();
    void canvasRenderHqStroking_data();
    void canvasRenderHqStroking();
    void canvasRenderPathGroups_data();
    void canvasRenderPathGroups();
    void canvasRenderGlyphEviction_data();
    void canvasRenderGlyphEviction();
    void canvasRenderEmojiGlyphEviction_data();
    void canvasRenderEmojiGlyphEviction();
    void multiPathRender_data();
    void multiPathRender();
    void cachedPathVariantsUpdate_data();
    void cachedPathVariantsUpdate();
    void cachedStrokeScale_data();
    void cachedStrokeScale();
    void cachedPathTransformedClip_data();
    void cachedPathTransformedClip();
    void cachedFillScale_data();
    void cachedFillScale();
    void cachedCurveScale_data();
    void cachedCurveScale();
    void tessellationLevelLimit_data();
    void tessellationLevelLimit();
    void zeroSizeCurves_data();
    void zeroSizeCurves();
    void perspectiveTransformIgnored();
    void restoreClip_data();
    void restoreClip();
    void pathWindingNotKept_data();
    void pathWindingNotKept();
    void clockwiseFillWithoutWindingEnforce_data();
    void clockwiseFillWithoutWindingEnforce();
    void shapeDirection_data();
    void shapeDirection();

private:
    void setWindowType(QWindow *window, QRhi::Implementation impl);

    struct RenderTarget
    {
        QRhiTexture *tex = nullptr;
        QRhiRenderBuffer *ds = nullptr;
        QRhiTextureRenderTarget *rt = nullptr;
        QRhiRenderPassDescriptor *rp = nullptr;
        static void deleter(RenderTarget *rt) {
            delete rt->tex;
            delete rt->ds;
            delete rt->rt;
            delete rt->rp;
        }
    };
    using RenderTargetPtr = std::unique_ptr<RenderTarget, decltype(&RenderTarget::deleter)>;
    RenderTargetPtr createRenderTarget(QRhi *rhi);

    struct {
        QRhiNullInitParams null;
#ifdef TST_GL
        QRhiGles2InitParams gl;
#endif
#ifdef TST_VK
        QRhiVulkanInitParams vk;
#endif
#ifdef TST_D3D11
        QRhiD3D11InitParams d3d11;
#endif
#ifdef TST_D3D12
        QRhiD3D12InitParams d3d12;
#endif
#ifdef TST_MTL
        QRhiMetalInitParams mtl;
#endif
    } initParams;

#ifdef TST_VK
    QVulkanInstance vulkanInstance;
#endif
#ifdef TST_GL
    QOffscreenSurface *fallbackSurface = nullptr;
#endif
#ifdef FRAME_CAPTURE
    std::unique_ptr<QGraphicsFrameCapture> m_cap;
#endif

    QRhi::Flags rhiCreateFlags;
};

#ifdef FRAME_CAPTURE
// must be done very early, before any QRhi is ever created (RenderDoc hooking limitations etc.)
static std::unique_ptr<QGraphicsFrameCapture> createFrameCapture()
{
    std::unique_ptr<QGraphicsFrameCapture> cap(new QGraphicsFrameCapture);
    return cap;
}

static void configureFrameCapture(QGraphicsFrameCapture *cap, QRhi *rhi)
{
    if (rhi->backend() == QRhi::Null)
        return;

    cap->setRhi(rhi);
}

static void startFrameCapture(QGraphicsFrameCapture *cap, QRhi *rhi, const char *filePrefix)
{
    if (!cap->isLoaded())
        return;

    const QString capPrefix = QString::asprintf("%s_%s", filePrefix, rhi->backendName());
    cap->setCapturePrefix(capPrefix);
    cap->setCapturePath(QLatin1String("."));
    cap->startCaptureFrame();
}

static void endFrameCapture(QGraphicsFrameCapture *cap)
{
    if (cap->isLoaded() && cap->isCapturing()) {
        cap->endCaptureFrame();
        qDebug() << "Frame capture saved to" << cap->capturedFileName();
    }
}
#endif

void tst_CanvasRhiRendering::initTestCase()
{
#ifdef FRAME_CAPTURE
    m_cap = createFrameCapture();
#endif

    // Have QRhi's own resource leak checking active in release builds too.
    qputenv("QT_RHI_LEAK_CHECK", "1");

#ifdef TST_GL
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
#ifdef Q_OS_MACOS
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
    QSurfaceFormat::setDefaultFormat(fmt);

    initParams.gl.format = QSurfaceFormat::defaultFormat();
    fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
    initParams.gl.fallbackSurface = fallbackSurface;
#endif

#ifdef TST_VK
    const QVersionNumber supportedVersion = vulkanInstance.supportedApiVersion();
    if (supportedVersion >= QVersionNumber(1, 2))
        vulkanInstance.setApiVersion(QVersionNumber(1, 2));
    else if (supportedVersion >= QVersionNumber(1, 1))
        vulkanInstance.setApiVersion(QVersionNumber(1, 1));
    vulkanInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
    vulkanInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
    vulkanInstance.create();
    initParams.vk.inst = &vulkanInstance;
#endif

#ifdef TST_D3D11
    initParams.d3d11.enableDebugLayer = true;
#endif
#ifdef TST_D3D12
    initParams.d3d12.enableDebugLayer = true;
#endif
}

void tst_CanvasRhiRendering::cleanupTestCase()
{
#ifdef TST_VK
    vulkanInstance.destroy();
#endif

#ifdef TST_GL
    delete fallbackSurface;
#endif
}

void tst_CanvasRhiRendering::rhiTestData()
{
    QTest::addColumn<QRhi::Implementation>("impl");
    QTest::addColumn<QRhiInitParams *>("initParams");

#ifndef Q_OS_WEBOS
    QTest::newRow("Null") << QRhi::Null << static_cast<QRhiInitParams *>(&initParams.null);
#endif
#ifdef TST_GL
    if (QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::OpenGL))
        QTest::newRow("OpenGL") << QRhi::OpenGLES2 << static_cast<QRhiInitParams *>(&initParams.gl);
#endif
#ifdef TST_VK
    if (vulkanInstance.isValid())
        QTest::newRow("Vulkan") << QRhi::Vulkan << static_cast<QRhiInitParams *>(&initParams.vk);
#endif
#ifdef TST_D3D11
    QTest::newRow("Direct3D 11") << QRhi::D3D11 << static_cast<QRhiInitParams *>(&initParams.d3d11);
#endif
#ifdef TST_D3D12
    QTest::newRow("Direct3D 12") << QRhi::D3D12 << static_cast<QRhiInitParams *>(&initParams.d3d12);
#endif
#ifdef TST_MTL
    QTest::newRow("Metal") << QRhi::Metal << static_cast<QRhiInitParams *>(&initParams.mtl);
#endif

#ifdef FRAME_CAPTURE
    rhiCreateFlags |= QRhi::EnableDebugMarkers;
#endif
}

static constexpr int RT_WIDTH = 1280;
static constexpr int RT_HEIGHT = 720;

tst_CanvasRhiRendering::RenderTargetPtr tst_CanvasRhiRendering::createRenderTarget(QRhi *rhi)
{
    RenderTargetPtr result(nullptr, &RenderTarget::deleter);
    std::unique_ptr<QRhiTexture> tex(rhi->newTexture(QRhiTexture::RGBA8,
                                                     QSize(RT_WIDTH, RT_HEIGHT),
                                                     1,
                                                     QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!tex->create())
        return result;

    std::unique_ptr<QRhiRenderBuffer> ds(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, tex->pixelSize()));
    if (!ds->create())
        return result;

    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ tex.get() });
    rtDesc.setDepthStencilBuffer(ds.get());
    std::unique_ptr<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    std::unique_ptr<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.get());
    if (!rt->create())
        return result;

    result.reset(new RenderTarget);
    *result = { tex.release(), ds.release(), rt.release(), rp.release() };
    return result;
}

void tst_CanvasRhiRendering::create_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::create()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCOMPARE(painter, factory->painter());
    QVERIFY(factory->isValid());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

    QCanvasPainter *samePainter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *samePaintDriver = factory->paintDriver();
    QCOMPARE(samePainter, painter);
    QCOMPARE(samePaintDriver, pd);
    QVERIFY(factory->isValid());

    factory->destroy();
    QVERIFY(!factory->isValid());
    painter = factory->create(rhi.get());
    QVERIFY(painter);
}

void tst_CanvasRhiRendering::createShared_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::createShared()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    QCanvasPainterFactory *factory1 = QCanvasPainterFactory::sharedInstance(rhi.get());
    QVERIFY(factory1);
    QVERIFY(factory1->isValid());
    QCanvasPainter *painter1 = factory1->painter();
    QVERIFY(painter1);
    QCanvasRhiPaintDriver *pd1 = factory1->paintDriver();
    QVERIFY(pd1);

    QCanvasPainterFactory *factory2 = QCanvasPainterFactory::sharedInstance(rhi.get());
    QVERIFY(factory2);
    QVERIFY(factory2->isValid());
    QCOMPARE(factory1, factory2);
    QCanvasPainter *painter2 = factory2->painter();
    QCOMPARE(painter1, painter2);
    QCanvasRhiPaintDriver *pd2 = factory2->paintDriver();
    QCOMPARE(pd1, pd2);

    // sharedInstance with another QRhi should give a different factory
    std::unique_ptr<QRhi> anotherRhi(QRhi::create(impl, initParams));
    QVERIFY(anotherRhi);
    QCanvasPainterFactory *factory3 = QCanvasPainterFactory::sharedInstance(anotherRhi.get());
    QVERIFY(factory3);
    QVERIFY(factory3->isValid());
    QCOMPARE_NE(factory1, factory3);
    QCanvasPainter *painter3 = factory3->painter();
    QCOMPARE_NE(painter1, painter3);
    QCanvasRhiPaintDriver *pd3 = factory3->paintDriver();
    QCOMPARE_NE(pd1, pd3);
}

static void drawCircleInCenter(QCanvasPainter *painter)
{
    painter->beginPath();
    painter->circle(RT_WIDTH / 2, RT_HEIGHT / 2, std::min(RT_WIDTH, RT_HEIGHT) / 2);
    painter->setStrokeStyle(Qt::green);
    painter->setLineWidth(4);
    painter->stroke();
    painter->setFillStyle("#ff0000");
    painter->fill();
}

static void drawCircleInCenter(QCanvasPainter *painter, const QCanvasImage &imageForPattern)
{
    painter->beginPath();
    painter->circle(RT_WIDTH / 2, RT_HEIGHT / 2, std::min(RT_WIDTH, RT_HEIGHT) / 2);
    painter->setStrokeStyle(Qt::blue);
    painter->setLineWidth(4);
    painter->stroke();
    QCanvasImagePattern pattern(imageForPattern, 0, 0, 100, 100);
    painter->setFillStyle(pattern);
    painter->fill();
}

static void drawCircleAndTextInCenter(QCanvasPainter *painter)
{
    const QPointF center(RT_WIDTH / 2, RT_HEIGHT / 2);
    painter->beginPath();
    painter->circle(center.x(), center.y(), std::min(RT_WIDTH, RT_HEIGHT) / 2);
    painter->setStrokeStyle(Qt::green);
    painter->setLineWidth(4);
    painter->stroke();
    painter->setFillStyle("#ff0000");
    painter->fill();
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(24);
    painter->setFont(font1);
    painter->setFillStyle(Qt::blue);
    painter->fillText("Hello", center.x(), center.y());
}

static bool testColor(const QImage &image, int x, int y, const QColor &expected, bool warnOnMismatch = true)
{
    const int maxFuzz = 1;
    QRgb c1 = image.pixel(x, y);
    QRgb c2 = expected.rgba();
    bool result = qAbs(qRed(c1) - qRed(c2)) <= maxFuzz
                  && qAbs(qGreen(c1) - qGreen(c2)) <= maxFuzz
                  && qAbs(qBlue(c1) - qBlue(c2)) <= maxFuzz
                  && qAbs(qAlpha(c1) - qAlpha(c2)) <= maxFuzz;
    if (!result && warnOnMismatch)
        qWarning() << "Color mismatch at" << x << "," << y << ": got" << QColor(c1) << "expected" << QColor(c2);
    return result;
}

static QImage imageFromReadback(QRhi *rhi, const QRhiReadbackResult &readbackResult)
{
    QImage image(reinterpret_cast<const uchar *>(readbackResult.data.constData()),
                 readbackResult.pixelSize.width(),
                 readbackResult.pixelSize.height(),
                 QImage::Format_RGBA8888);

    if (rhi->isYUpInFramebuffer())
        return image.flipped();

    return image.copy();
}

static QImage imageFromReadback(QRhi *rhi, QRhiTexture *texture)
{
    QRhiReadbackResult readbackResult;
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture({ texture }, &readbackResult);
    cb->resourceUpdate(u);
    rhi->endOffscreenFrame();
    return imageFromReadback(rhi, readbackResult);
}

void tst_CanvasRhiRendering::render_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::render()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "render");
#endif

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

    QRhiReadbackResult readbackResult;
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    pd->resetForNewFrame();

    pd->beginPaint(cb, rt->rt);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);

    cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
    pd->renderPaint();
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture({ rt->tex }, &readbackResult);
    cb->endPass(u);

    rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    // cannot check rendering results with Null, because there is no rendering there
    if (impl == QRhi::Null)
        return;

    QImage image = imageFromReadback(rhi.get(), readbackResult);

    QVERIFY(testColor(image, 1, 1, Qt::black));
    QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
}

void tst_CanvasRhiRendering::renderWithDepthTest_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::renderWithDepthTest()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    const int centerX = RT_WIDTH / 2;
    const int centerY = RT_HEIGHT / 2;

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt, Qt::black, QSize(), 0.0f, QCanvasRhiPaintDriver::BeginPaintFlag::DepthTest);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
    // Depth buffer is cleared to the usual 1.0, so the the circle should show up normally.
    cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
    pd->renderPaint();
    cb->endPass();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, centerX, centerY, Qt::red));
    }

    // Now clear the depth buffer to 0, so that the default Less comparison
    // fails. First verify however that rendering without the DepthTest flag
    // still does what it should (ignoring the depth buffer contents).
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
    cb->beginPass(rt->rt, Qt::black, { 0.0f, 0 }); // note that depth is cleared to 0
    pd->renderPaint();
    cb->endPass();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, centerX, centerY, Qt::red));
    }

    // Now repeat with the DepthTest flag
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt, Qt::black, QSize(), 0.0f, QCanvasRhiPaintDriver::BeginPaintFlag::DepthTest);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
    cb->beginPass(rt->rt, Qt::black, { 0.0f, 0 }); // note that depth is cleared to 0
    pd->renderPaint();
    cb->endPass();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 1, 1, Qt::black));
        // the red circle should not be there
        QVERIFY(!testColor(image, centerX, centerY, Qt::red, false));
    }
}

void tst_CanvasRhiRendering::canvasRender_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRender()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRender");
#endif

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

    QCanvasOffscreenCanvas canvas;
    QVERIFY(canvas.isNull());
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    QVERIFY(canvas.texture());
    QCOMPARE(canvas.texture()->pixelSize().width(), RT_WIDTH);
    QCOMPARE(canvas.texture()->pixelSize().height(), RT_HEIGHT);
    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
    }

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    QCanvasImage canvasImage;
    QVERIFY(canvasImage.isNull());
    canvasImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat);
    QVERIFY(!canvasImage.isNull());
    QCOMPARE(canvasImage.width(), canvas.width());
    QCOMPARE(canvasImage.height(), canvas.height());
    QCOMPARE(canvasImage.size(), canvas.size());
    QCanvasImage secondRegistrationImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat);
    QCOMPARE(canvasImage, secondRegistrationImage);
    drawCircleInCenter(painter, canvasImage);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), rt->tex);
        int x = RT_WIDTH / 2;
        int y = RT_HEIGHT / 2;
        // the distorted circle in the middle
        QVERIFY(testColor(image, x, y, Qt::red));
        // black between columns and rows
        QVERIFY(testColor(image, x - 50, y, Qt::black));
        // the distorted circle to the left
        QVERIFY(testColor(image, x - 100, y, Qt::red));
    }

    // Now render some text.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    drawCircleAndTextInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), rt->tex);
        // the image only has red, green, blue, and black
        int redCount = 0, greenCount = 0, blueCount = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qRed(image.pixel(x, y)) > 240)
                    ++redCount;
                else if (qGreen(image.pixel(x, y)) > 240)
                    ++greenCount;
                else if (qBlue(image.pixel(x, y)) > 240)
                    ++blueCount;
            }
        }
        // the fill
        QCOMPARE_GT(redCount, 400000);
        // the border
        QCOMPARE_GT(greenCount, 2000);
        // the text, use a low threshold since the font may differ between
        // platforms, and so it may mean fewer or more blue pixels.
        QCOMPARE_GT(blueCount, 200);
    }

    QCanvasOffscreenCanvas canvas2 = canvas;
    QCOMPARE(canvas, canvas2);
    canvas2.setFillColor(Qt::red);
    QCOMPARE_NE(canvas, canvas2);

    QCanvasOffscreenCanvas canvas3 = canvas;
    painter->destroyCanvas(canvas);
    QVERIFY(canvas.isNull());
    QVERIFY(canvas3.isNull());
    QCOMPARE(canvas, canvas3);
    // This is evil, since destroyCanvas was used explicitly and canvas2 may still seem valid, but it is not.
    // Should not happen in practice, though.
    QVERIFY(!canvas2.isNull());
    QCOMPARE_NE(canvas, canvas2);

    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    QVERIFY(canvas.texture());

    // let the painter destroy 'canvas' automatically

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif
}

void tst_CanvasRhiRendering::canvasGrab_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasGrab()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd && painter);

    QCanvasOffscreenCanvas canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    const QSize expectedSize(RT_WIDTH, RT_HEIGHT);

    // A context object is mandatory. This one outlives every grab issued below,
    // so it never cancels anything. See canvasGrabContext() for cancellation.
    QObject context;

    // Grabbing outside of a frame. The callback is invoked before grabCanvas() returns.
    {
        int calls = 0;
        QImage grabbed;
        pd->grabCanvas(canvas, &context, [&calls, &grabbed](const QImage &image) {
            ++calls;
            // the QImage only references the readback data, which is gone once
            // the callback returns
            grabbed = image.copy();
        });
        QCOMPARE(calls, 1);
        QCOMPARE(grabbed.size(), expectedSize);
        if (impl != QRhi::Null) {
            QVERIFY(testColor(grabbed, 1, 1, Qt::black));
            QVERIFY(testColor(grabbed, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        }
    }

    // A move-only callable. Does not compile with a std::function based API.
    {
        auto moveOnlyState = std::make_unique<QSize>(expectedSize);
        bool called = false;
        QSize stateSeenInCallback;
        QImage grabbed;
        pd->grabCanvas(canvas, &context, [state = std::move(moveOnlyState), &called,
                                          &stateSeenInCallback, &grabbed](const QImage &image) {
            called = true;
            // the captured move-only state must have survived the type erasure
            stateSeenInCallback = *state;
            grabbed = image.copy();
        });
        QVERIFY(called);
        QCOMPARE(stateSeenInCallback, expectedSize);
        QCOMPARE(grabbed.size(), expectedSize);
        if (impl != QRhi::Null) {
            QVERIFY(testColor(grabbed, 1, 1, Qt::black));
            QVERIFY(testColor(grabbed, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        }
    }

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    // Grabbing while a frame is being recorded. The callback is invoked later,
    // by the time endOffscreenFrame() returns. Paint to the render target, not
    // to the canvas, so that the canvas contents are the ones drawn above.
    {
        int calls = 0;
        QImage grabbed;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        pd->grabCanvas(canvas, &context, [&calls, &grabbed](const QImage &image) {
            ++calls;
            grabbed = image.copy();
        });
        // Null performs resource updates immediately, so there the grab has
        // already completed at this point. Every other backend defers it.
        if (impl != QRhi::Null)
            QCOMPARE(calls, 0);
        pd->endPaint();
        rhi->endOffscreenFrame();

        QCOMPARE(calls, 1);
        QCOMPARE(grabbed.size(), expectedSize);
        if (impl != QRhi::Null) {
            QVERIFY(testColor(grabbed, 1, 1, Qt::black));
            QVERIFY(testColor(grabbed, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        }
    }

    // Multiple grabs pending at the same time must each invoke their own
    // callback. Completing one must not disturb the ones still outstanding.
    {
        QList<int> completed;
        QList<QImage> grabbed;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        for (int i = 1; i <= 3; ++i) {
            pd->grabCanvas(canvas, &context, [&completed, &grabbed, i](const QImage &image) {
                completed.append(i);
                grabbed.append(image.copy());
            });
        }
        pd->endPaint();
        rhi->endOffscreenFrame();

        // The order in which simultaneous readbacks complete is up to the
        // backend, some of them run their completion callbacks in reverse.
        std::sort(completed.begin(), completed.end());
        QCOMPARE(completed, QList<int>({ 1, 2, 3 }));

        // All three grabbed the same canvas, so each must have gotten its own
        // copy of the same contents.
        QCOMPARE(grabbed.size(), 3);
        for (const QImage &image : std::as_const(grabbed)) {
            QCOMPARE(image.size(), expectedSize);
            if (impl != QRhi::Null) {
                QVERIFY(testColor(image, 1, 1, Qt::black));
                QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
            }
        }
    }

    // Grabbing a null canvas is a no-op apart from the warning.
    {
        int calls = 0;
        QCanvasOffscreenCanvas nullCanvas;
        QVERIFY(nullCanvas.isNull());
        QTest::ignoreMessage(QtWarningMsg, "Cannot grab null canvas");
        pd->grabCanvas(nullCanvas, &context, [&calls](const QImage &) { ++calls; });
        QCOMPARE(calls, 0);
    }
}

// Not a Q_OBJECT, none of this needs moc, but it has to be a QObject to serve
// as a context object.
class GrabReceiver : public QObject
{
public:
    void onGrabbed(const QImage &image)
    {
        ++calls;
        grabbed = image.copy();
    }

    int calls = 0;
    QImage grabbed;
};

void tst_CanvasRhiRendering::canvasGrabContext_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasGrabContext()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd && painter);

    QCanvasOffscreenCanvas canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    const QSize expectedSize(RT_WIDTH, RT_HEIGHT);

    // A context object that outlives the readback changes nothing.
    {
        QObject context;
        int calls = 0;
        QImage grabbed;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        pd->grabCanvas(canvas, &context, [&calls, &grabbed](const QImage &image) {
            ++calls;
            grabbed = image.copy();
        });
        pd->endPaint();
        rhi->endOffscreenFrame();

        QCOMPARE(calls, 1);
        QCOMPARE(grabbed.size(), expectedSize);
        if (impl != QRhi::Null) {
            QVERIFY(testColor(grabbed, 1, 1, Qt::black));
            QVERIFY(testColor(grabbed, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        }
    }

    // Cancelling a grab that is still pending needs a backend that actually
    // defers the readback. Null completes it right away, within grabCanvas().
    if (impl != QRhi::Null) {
        // Destroying the context object before the readback completes cancels
        // the grab, the callback is not invoked.
        {
            auto context = std::make_unique<QObject>();
            int calls = 0;
            rhi->beginOffscreenFrame(&cb);
            pd->resetForNewFrame();
            pd->beginPaint(cb, rt->rt);
            pd->grabCanvas(canvas, context.get(), [&calls](const QImage &) { ++calls; });
            pd->endPaint();
            context.reset();
            rhi->endOffscreenFrame();

            QCOMPARE(calls, 0);
        }

        // Cancelling one grab must leave the others alone.
        {
            auto context = std::make_unique<QObject>();
            QObject survivingContext;
            int cancelledCalls = 0;
            QList<int> completed;
            QList<QImage> grabbed;
            rhi->beginOffscreenFrame(&cb);
            pd->resetForNewFrame();
            pd->beginPaint(cb, rt->rt);
            auto surviving = [&completed, &grabbed](int id) {
                return [&completed, &grabbed, id](const QImage &image) {
                    completed.append(id);
                    grabbed.append(image.copy());
                };
            };
            pd->grabCanvas(canvas, &survivingContext, surviving(1));
            pd->grabCanvas(canvas, context.get(),
                           [&cancelledCalls](const QImage &) { ++cancelledCalls; });
            pd->grabCanvas(canvas, &survivingContext, surviving(3));
            pd->endPaint();
            context.reset();
            rhi->endOffscreenFrame();

            QCOMPARE(cancelledCalls, 0);
            std::sort(completed.begin(), completed.end());
            QCOMPARE(completed, QList<int>({ 1, 3 }));

            // Cancelling the middle grab must not corrupt what the other two get.
            QCOMPARE(grabbed.size(), 2);
            for (const QImage &image : std::as_const(grabbed)) {
                QCOMPARE(image.size(), expectedSize);
                QVERIFY(testColor(image, 1, 1, Qt::black));
                QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
            }
        }

        // The same, with a member function pointer, cancelled by destroying
        // the receiver.
        {
            auto receiver = std::make_unique<GrabReceiver>();
            rhi->beginOffscreenFrame(&cb);
            pd->resetForNewFrame();
            pd->beginPaint(cb, rt->rt);
            pd->grabCanvas(canvas, receiver.get(), &GrabReceiver::onGrabbed);
            pd->endPaint();
            QCOMPARE(receiver->calls, 0);
            receiver.reset();
            rhi->endOffscreenFrame();
        }
    }

    // A member function pointer, with the receiver acting as the context.
    {
        GrabReceiver receiver;
        pd->grabCanvas(canvas, &receiver, &GrabReceiver::onGrabbed);
        QCOMPARE(receiver.calls, 1);
        QCOMPARE(receiver.grabbed.size(), expectedSize);
        if (impl != QRhi::Null) {
            QVERIFY(testColor(receiver.grabbed, 1, 1, Qt::black));
            QVERIFY(testColor(receiver.grabbed, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        }
    }
}

void tst_CanvasRhiRendering::canvasRenderMipMap_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRenderMipMap()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderMipMap_part1");
#endif

    QCanvasOffscreenCanvas canvas;
    QVERIFY(canvas.isNull());
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT), 1, QCanvasOffscreenCanvas::Flag::MipMaps);
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    QVERIFY(canvas.texture());
    QCOMPARE(canvas.texture()->pixelSize().width(), RT_WIDTH);
    QCOMPARE(canvas.texture()->pixelSize().height(), RT_HEIGHT);
    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
    }

#ifdef FRAME_CAPTURE
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderMipMap_part2");
#endif

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    QCanvasImage canvasImage;
    QVERIFY(canvasImage.isNull());
    // request addImage to generate the mimap sequence
    canvasImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat | QCanvasPainter::ImageFlag::GenerateMipmaps);
    QVERIFY(!canvasImage.isNull());
    drawCircleInCenter(painter, canvasImage);
    pd->endPaint();
    rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    // won't verify here that the linear mipmap filtering works, since the result is the same anyway
    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), rt->tex);
        int x = RT_WIDTH / 2;
        int y = RT_HEIGHT / 2;
        // the distorted circle in the middle
        QVERIFY(testColor(image, x, y, Qt::red));
        // black between columns and rows
        QVERIFY(testColor(image, x - 50, y, Qt::black));
        // the distorted circle to the left
        QVERIFY(testColor(image, x - 100, y, Qt::red));
    }
}

void tst_CanvasRhiRendering::canvasRenderHqStroking_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRenderHqStroking()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderHqStroking");
#endif

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd && painter);

    QCanvasOffscreenCanvas canvas;
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    // this triggers using the stencil buffer
    painter->setHighQualityStroking(true);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        int greenCount = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qGreen(image.pixel(x, y)) > 250)
                    ++greenCount;
            }
        }
        // ca. 2068 green pixels, the rest is either black or red
        QCOMPARE_GT(greenCount, 2000);
    }

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif
}

void tst_CanvasRhiRendering::canvasRenderPathGroups_data()
{
    rhiTestData();
}

static QCanvasPath makePathOne()
{
    QCanvasPath path(64, 128);
    path.reserve(128);
    path.reserve(64, 160);

    path.moveTo(10.f, 10.f);
    path.lineTo(100.f, 10.f);
    path.lineTo(100.f, 80.f);
    path.lineTo(10.f, 80.f);
    path.closePath();

    path.moveTo(QPointF(120.f, 10.f));
    path.lineTo(QPointF(200.f, 10.f));
    path.lineTo(QPointF(200.f, 80.f));
    path.closePath();

    path.moveTo(10.f, 120.f);
    path.bezierCurveTo(10.f, 90.f, 80.f, 90.f, 80.f, 120.f);

    path.moveTo(QPointF(100.f, 120.f));
    path.bezierCurveTo(QPointF(100.f, 90.f), QPointF(180.f, 90.f), QPointF(180.f, 120.f));

    path.moveTo(10.f, 160.f);
    path.quadraticCurveTo(55.f, 130.f, 100.f, 160.f);

    path.moveTo(QPointF(120.f, 160.f));
    path.quadraticCurveTo(QPointF(165.f, 130.f), QPointF(210.f, 160.f));

    path.moveTo(10.f, 200.f);
    path.arcTo(10.f, 250.f, 70.f, 250.f, 35.f);

    path.moveTo(QPointF(120.f, 200.f));
    path.arcTo(QPointF(120.f, 250.f), QPointF(180.f, 250.f), 35.f);

    path.moveTo(260.f, 225.f);
    path.arc(230.f, 225.f, 30.f, 0.f, float(M_PI));
    path.arc(330.f, 225.f, 30.f, 0.f, float(M_PI),
             QCanvasPainter::PathWinding::CounterClockWise,
             QCanvasPainter::PathConnection::NotConnected);
    path.arc(QPointF(430.f, 225.f), 30.f, 0.f, 2.f * float(M_PI),
             QCanvasPainter::PathWinding::ClockWise,
             QCanvasPainter::PathConnection::NotConnected);

    path.rect(10.f, 280.f, 80.f, 50.f);
    path.rect(QRectF(110.f, 280.f, 80.f, 50.f));
    path.roundRect(210.f, 280.f, 80.f, 50.f, 10.f);
    path.roundRect(QRectF(310.f, 280.f, 80.f, 50.f), 10.f);
    path.roundRect(10.f, 350.f, 80.f, 50.f, 5.f, 10.f, 15.f, 20.f);
    path.roundRect(QRectF(110.f, 350.f, 80.f, 50.f), 5.f, 10.f, 15.f, 20.f);
    path.ellipse(260.f, 375.f, 40.f, 22.f);
    path.ellipse(QRectF(310.f, 353.f, 80.f, 44.f));

    path.setPathWinding(QCanvasPainter::PathWinding::CounterClockWise);
    path.circle(80.f, 460.f, 50.f);
    path.beginHoleSubPath(); // ClockWise
    path.circle(80.f, 460.f, 30.f);
    path.beginSolidSubPath();  // CounterClockWise
    path.circle(80.f, 460.f, 12.f);

    return path;
}

static QCanvasPath makePathTwo()
{
    QCanvasPath path;
    path.moveTo(50.f, 40.f);
    path.lineTo(90.f, 20.f);
    path.lineTo(80.f, 38.f);
    path.lineTo(120.f, 38.f);
    path.lineTo(120.f, 42.f);
    path.lineTo(80.f, 42.f);
    path.lineTo(90.f, 60.f);
    path.closePath();

    path.moveTo(QPointF(160.f, 20.f));
    path.bezierCurveTo(QPointF(130.f, 20.f), QPointF(130.f, 60.f), QPointF(160.f, 60.f));
    path.bezierCurveTo(QPointF(190.f, 60.f), QPointF(190.f, 20.f), QPointF(220.f, 20.f));

    path.moveTo(240.f, 60.f);
    path.quadraticCurveTo(270.f, 10.f, 300.f, 60.f);

    path.moveTo(QPointF(320.f, 60.f));
    path.lineTo(QPointF(320.f, 25.f));
    path.arcTo(QPointF(320.f, 20.f), QPointF(325.f, 20.f), 5.f);
    path.lineTo(QPointF(375.f, 20.f));
    path.arcTo(QPointF(380.f, 20.f), QPointF(380.f, 25.f), 5.f);
    path.lineTo(QPointF(380.f, 60.f));

    path.arc(430.f, 40.f, 20.f, 0.f, 2.f * float(M_PI),
            QCanvasPainter::PathWinding::ClockWise,
            QCanvasPainter::PathConnection::NotConnected);

    path.moveTo(510.f, 40.f);
    path.arc(510.f, 40.f, 20.f, 0.4f, 2.f * float(M_PI) - 0.4f,
            QCanvasPainter::PathWinding::ClockWise,
            QCanvasPainter::PathConnection::Connected);
    path.closePath();

    path.setPathWinding(QCanvasPainter::PathWinding::CounterClockWise);
    path.arc(QPointF(570.f, 40.f), 22.f, 0.f, 2.f * float(M_PI),
            QCanvasPainter::PathWinding::CounterClockWise,
            QCanvasPainter::PathConnection::NotConnected);
    path.beginHoleSubPath();
    path.circle(QPointF(570.f, 40.f), 10.f);
    path.beginSolidSubPath();

    path.rect(10.f, 100.f, 60.f, 40.f);
    path.rect(QRectF(90.f, 100.f, 60.f, 40.f));

    path.roundRect(170.f, 100.f, 60.f, 40.f, 8.f);
    path.roundRect(QRectF(250.f, 100.f, 60.f, 40.f), 8.f);

    path.roundRect(330.f, 100.f, 60.f, 40.f, 0.f, 12.f, 0.f, 12.f);
    path.roundRect(QRectF(410.f, 100.f, 60.f, 40.f), 12.f, 0.f, 12.f, 0.f);

    path.ellipse(510.f, 120.f, 35.f, 18.f);
    path.ellipse(QRectF(555.f, 102.f, 50.f, 36.f));

    path.circle(640.f, 120.f, 18.f);
    path.circle(QPointF(680.f, 120.f), 18.f);

    // star
    {
        const float cx = 60.f, cy = 220.f, r1 = 40.f, r2 = 18.f;
        const int points = 5;
        for (int i = 0; i < points; ++i) {
            float outerAngle = float(i) * 2.f * float(M_PI) / points - float(M_PI_2);
            float innerAngle = outerAngle + float(M_PI) / points;
            QPointF outer(cx + r1 * std::cos(outerAngle), cy + r1 * std::sin(outerAngle));
            QPointF inner(cx + r2 * std::cos(innerAngle), cy + r2 * std::sin(innerAngle));
            if (i == 0)
                path.moveTo(outer);
            else
                path.lineTo(outer);
            path.lineTo(inner);
        }
        path.closePath();
    }

    // spiral
    {
        const float cx = 200.f, cy = 220.f;
        float r = 5.f;
        path.moveTo(cx + r, cy);
        for (int i = 0; i < 6; ++i) {
            float a0 = float(i) * float(M_PI_2);
            float a1 = a0 + float(M_PI_2);
            float r0 = r + float(i) * 6.f;
            float r1 = r + float(i + 1) * 6.f;
            float k = 0.5523f; // cubic bezier circle approximation constant
            path.bezierCurveTo(
                cx + r0 * std::cos(a0) - r0 * k * std::sin(a0),
                cy + r0 * std::sin(a0) + r0 * k * std::cos(a0),
                cx + r1 * std::cos(a1) + r1 * k * std::sin(a1),
                cy + r1 * std::sin(a1) - r1 * k * std::cos(a1),
                cx + r1 * std::cos(a1),
                cy + r1 * std::sin(a1));
        }
    }

    // heart
    {
        const float cx = 380.f, cy = 240.f, w = 35.f;
        path.moveTo(cx, cy + w * 0.25f);
        path.quadraticCurveTo(cx - w, cy - w * 0.5f, cx - w * 0.5f, cy - w);
        path.quadraticCurveTo(cx,      cy - w * 0.8f, cx,             cy - w * 0.3f);
        path.quadraticCurveTo(cx,      cy - w * 0.8f, cx + w * 0.5f, cy - w);
        path.quadraticCurveTo(cx + w,  cy - w * 0.5f, cx,             cy + w * 0.25f);
        path.closePath();
    }

    return path;
}

void tst_CanvasRhiRendering::canvasRenderPathGroups()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderPathGroups");
#endif

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd && painter);

    QCanvasOffscreenCanvas canvas;
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);

    QCanvasPath pathLeft = makePathOne();
    QCanvasPath pathRight = makePathTwo();

    int group = 123;

    cb->debugMarkMsg("First frame: drawing path one and two with group 123");

    auto draw = [&](float lineWidth1, float lineWidth2, float aa1, float aa2) {
        painter->setAntialias(aa1);
        painter->setFillStyle(Qt::green);
        painter->fill(pathLeft, group);
        painter->setStrokeStyle(Qt::red);
        painter->setLineWidth(lineWidth1);
        painter->stroke(pathLeft, group);

        painter->translate(RT_WIDTH / 2 - 100, 0);
        painter->setAntialias(aa2);
        painter->fill(pathRight, group);
        painter->setLineWidth(lineWidth2);
        painter->stroke(pathRight, group);
    };

    draw(4, 4, 1, 1);

    pd->endPaint();
    rhi->endOffscreenFrame();

    int redCount = 0;
    int greenCount = 0;
    auto updateColorCounts = [&](const QImage &image) {
        redCount = 0;
        greenCount = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qGreen(image.pixel(x, y)) > 250)
                    ++greenCount;
                if (qRed(image.pixel(x, y)) > 250)
                    ++redCount;
            }
        }
    };

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 18910
        QVERIFY(redCount > 18000);
        QVERIFY(redCount < 20000);
        // 70314
        QVERIFY(greenCount > 70000);
        QVERIFY(greenCount < 71000);
    }

    // Another frame, use the same path group. This should trigger full reuse of the path group vertex/index buffers.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Second frame: drawing path one and two again with group 123");
    draw(4, 4, 1, 1);
    pd->endPaint();
    rhi->endOffscreenFrame();
    // Leave verification to the third frame

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Third frame: drawing path one and two again with group 123");
    draw(4, 4, 1, 1);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 18910
        QVERIFY(redCount > 18000);
        QVERIFY(redCount < 20000);
        // 70314
        QVERIFY(greenCount > 70000);
        QVERIFY(greenCount < 71000);
    }

    // Now we rendered three frames with path group 123 and no change in the QCanvasPath or relevant states.
    // Now change the stroke width for one of the two paths (but not for the other), which internally should
    // generate another version of that path with the different stroke width.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Fourth frame: drawing path one with larger stroke width, path two is same as before, group is still 123");
    draw(8, 4, 1, 1); // different stroke width for the left path!
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 34534
        QVERIFY(redCount > 34000);
        QVERIFY(redCount < 35000);
        // 63392
        QVERIFY(greenCount > 63000);
        QVERIFY(greenCount < 64000);
    }

    // Fifth frame: like frame 4, but disable antialiasing.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Fifth frame: drawing like in fourth, but with AA disabled");
    draw(8, 4, 0, 0);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 45045
        QVERIFY(redCount > 44000);
        QVERIFY(redCount < 46000);
        // 68868
        QVERIFY(greenCount > 68000);
        QVERIFY(greenCount < 70000);
    }

    // Sixth frame: like frame 4, but higher AA for the right path.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Sixth frame: drawing like in fourth, but AA increased for right path");
    draw(8, 4, 1, 5);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 25659
        QVERIFY(redCount > 25000);
        QVERIFY(redCount < 26000);
        // 54775
        QVERIFY(greenCount > 54000);
        QVERIFY(greenCount < 55000);
    }

    // Seventh frame: clear() one of the paths and add some new elements to it.
    pathRight.clear();
    pathRight.moveTo(20, 20);
    pathRight.lineTo(100, 180);
    pathRight.lineTo(180, 20);
    pathRight.closePath();
    pathRight.moveTo(100, 40);
    pathRight.lineTo(125, 90);
    pathRight.lineTo(75, 90);
    pathRight.closePath();

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Seventh frame: drawing like in first, but the right path is now different");
    draw(4, 4, 1, 1);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 11715
        QVERIFY(redCount > 11000);
        QVERIFY(redCount < 12000);
        // 54499
        QVERIFY(greenCount > 54000);
        QVERIFY(greenCount < 55000);
    }

    // Eighth frame: change the path group.
    group = 124;

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Eighth frame: drawing like in seventh, but the path group is different");
    draw(4, 4, 1, 1);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 11715
        QVERIFY(redCount > 11000);
        QVERIFY(redCount < 12000);
        // 54499
        QVERIFY(greenCount > 54000);
        QVERIFY(greenCount < 55000);
    }

    // Ninth frame: Make a copy, pass in the same local variable (as in, &p is
    // the same) for both paths to fill() and stroke() and see if it does not
    // get confused.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    cb->debugMarkMsg("Ninth frame: like the previous, but paths are copied");
    QCanvasPath p = pathLeft;
    painter->setAntialias(1.0f);
    painter->setFillStyle(Qt::green);
    painter->fill(p, group);
    painter->setStrokeStyle(Qt::red);
    painter->setLineWidth(4.0f);
    painter->stroke(p, group);
    p = pathRight;
    painter->translate(RT_WIDTH / 2 - 100, 0);
    painter->fill(p, group);
    painter->stroke(p, group);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        updateColorCounts(image);
        // 11715
        QVERIFY(redCount > 11000);
        QVERIFY(redCount < 12000);
        // 54499
        QVERIFY(greenCount > 54000);
        QVERIFY(greenCount < 55000);
    }

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif
}

void tst_CanvasRhiRendering::canvasRenderGlyphEviction_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRenderGlyphEviction()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(painter && pd);

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    // The distance-field glyph cache for a given font is a single fixed-size
    // atlas texture. Unused glyph eviction only kicks in once that atlas is
    // full: a glyph that was referenced in the previous frame but not in the
    // current one has its refcount dropped to zero, becomes "unused", and is
    // then evicted to make room for newly requested glyphs.
    //
    // To exercise this we draw a disjoint block of distinct glyphs per frame,
    // so the cumulative number of glyphs far exceeds the atlas capacity while
    // only a small working set is referenced in any single frame. With working
    // eviction every frame - including the last - renders its text. With broken
    // eviction the atlas fills after the first few frames and every subsequent
    // frame renders (almost) nothing.
    //
    // The test assumes an 1024x1024 atlas.

    QFont font;
    font.setPixelSize(22);

    // Collect code points that the active font can actually render, deduped by
    // glyph index, so the test does not depend on the glyph coverage of
    // whatever font happens to be the default on the test machine.
    const QRawFont rawFont = QRawFont::fromFont(font);
    QList<QChar> chars;
    QSet<quint32> seenGlyphs;
    const QList<std::pair<uint, uint>> ranges = {
        { 0x21, 0x7E },     // Basic Latin (printable)
        { 0xA1, 0x17F },    // Latin-1 Supplement + Latin Extended-A
        { 0x180, 0x24F },   // Latin Extended-B
        { 0x370, 0x3FF },   // Greek
        { 0x400, 0x4FF },   // Cyrillic
        { 0x4E00, 0x9FFF }, // CJK Unified Ideographs (only if the font has them)
    };
    for (const auto &range : ranges) {
        for (uint uc = range.first; uc <= range.second; ++uc) {
            if (!rawFont.supportsCharacter(uc))
                continue;
            const QChar ch = QChar(char16_t(uc));
            const QList<quint32> idx = rawFont.glyphIndexesForString(QString(ch));
            if (idx.size() != 1 || idx.first() == 0 || seenGlyphs.contains(idx.first()))
                continue;
            seenGlyphs.insert(idx.first());
            chars.append(ch);
        }
    }

    constexpr int requiredGlyphs = 600;
    if (chars.size() < requiredGlyphs)
        QSKIP("Active font does not provide enough distinct glyphs to overflow the atlas");

    auto drawBlock = [&](int first, int count) {
        painter->setFillStyle(Qt::white);
        painter->setFont(font);
        painter->setTextAlign(QCanvasPainter::TextAlign::Left);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
        const int columns = 32;
        const int cellW = RT_WIDTH / columns;
        const int cellH = 26;
        for (int i = 0; i < count; ++i) {
            const int col = i % columns;
            const int row = i / columns;
            painter->fillText(QString(chars.at(first + i)),
                              float(col * cellW + 2), float(row * cellH + 2));
        }
    };

    auto countWhitePixels = [](const QImage &image) {
        int n = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QRgb p = image.pixel(x, y);
                if (qRed(p) > 200 && qGreen(p) > 200 && qBlue(p) > 200)
                    ++n;
            }
        }
        return n;
    };

    const int blockSize = 50;
    const int blockCount = chars.size() / blockSize;
    QVERIFY(blockCount >= 4);

    int firstBlockPixels = 0;
    int lastBlockPixels = 0;
    for (int b = 0; b < blockCount; ++b) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt, Qt::black);
        drawBlock(b * blockSize, blockSize);
        pd->endPaint();
        rhi->endOffscreenFrame();

        if (impl != QRhi::Null && (b == 0 || b == blockCount - 1)) {
            QImage image = imageFromReadback(rhi.get(), rt->tex);
            const int n = countWhitePixels(image);
            if (b == 0)
                firstBlockPixels = n;
            else
                lastBlockPixels = n;
        }
    }

    if (impl != QRhi::Null) {
        // Sanity check: the first block always fits, so text rendering works at
        // all. If this fails the problem is not eviction.
        QCOMPARE_GT(firstBlockPixels, 500);

        // With broken eviction/reuse this count collapses to something much
        // smaller than 500.
        QCOMPARE_GT(lastBlockPixels, 500);
    }
}

void tst_CanvasRhiRendering::canvasRenderEmojiGlyphEviction_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRenderEmojiGlyphEviction()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(painter && pd);

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    // Same idea as canvasRenderGlyphEviction(), but for color (emoji) glyphs,
    // which live in the separate RGBA atlas of QCRhiColorGlyphCache. That atlas
    // grows on demand up to 2048x2048; a large pixel size is used so it fills
    // after a couple hundred distinct glyphs rather than many thousands.

    QFont font;
    font.setPixelSize(90);

    // Resolve the color font the platform substitutes for emoji, so the set of
    // usable glyphs does not depend on the machine's emoji coverage.
    auto resolveEmojiFont = [&](char32_t probe) -> QRawFont {
        QTextLayout layout(QString::fromUcs4(&probe, 1), font);
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        const QList<QGlyphRun> runs = layout.glyphRuns();
        return runs.isEmpty() ? QRawFont() : runs.first().rawFont();
    };

    const QRawFont emojiFont = resolveEmojiFont(U'\U0001F600');
    if (!emojiFont.isValid())
        QSKIP("No emoji font available on this platform");

    // Emoji-presentation code points (color without a VS16 selector), deduped
    // by glyph index in the resolved color font.
    QList<char32_t> emojis;
    QSet<quint32> seenGlyphs;
    const QList<std::pair<char32_t, char32_t>> ranges = {
        { 0x1F300, 0x1F5FF }, // Miscellaneous Symbols and Pictographs
        { 0x1F600, 0x1F64F }, // Emoticons
        { 0x1F680, 0x1F6FF }, // Transport and Map Symbols
        { 0x1F900, 0x1F9FF }, // Supplemental Symbols and Pictographs
        { 0x1FA70, 0x1FAFF }, // Symbols and Pictographs Extended-A
    };
    for (const auto &range : ranges) {
        for (char32_t cp = range.first; cp <= range.second; ++cp) {
            if (!emojiFont.supportsCharacter(cp))
                continue;
            const QList<quint32> idx = emojiFont.glyphIndexesForString(QString::fromUcs4(&cp, 1));
            if (idx.size() != 1 || idx.first() == 0 || seenGlyphs.contains(idx.first()))
                continue;
            seenGlyphs.insert(idx.first());
            emojis.append(cp);
        }
    }

    // At 90px the atlas holds on the order of ~400 glyphs, so this many is
    // enough to overflow it and force several frames' worth of eviction.
    constexpr int requiredGlyphs = 600;
    if (emojis.size() < requiredGlyphs)
        QSKIP("Emoji font does not provide enough distinct color glyphs to overflow the atlas");

    constexpr int cell = 100;
    const int columns = RT_WIDTH / cell;
    constexpr int blockSize = 40;

    auto drawBlock = [&](int first, int count) {
        painter->setFillStyle(Qt::white);
        painter->setFont(font);
        painter->setTextAlign(QCanvasPainter::TextAlign::Left);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
        for (int i = 0; i < count; ++i) {
            const int col = i % columns;
            const int row = i / columns;
            const char32_t cp = emojis.at(first + i);
            painter->fillText(QString::fromUcs4(&cp, 1),
                              float(col * cell + 8), float(row * cell + 8));
        }
    };

    // Emoji carry saturated palette colors; the black background and any white
    // foreground/outline fallbacks are (near-)grey and excluded.
    auto countColorfulPixels = [](const QImage &image) {
        int n = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QRgb p = image.pixel(x, y);
                const int mx = qMax(qRed(p), qMax(qGreen(p), qBlue(p)));
                const int mn = qMin(qRed(p), qMin(qGreen(p), qBlue(p)));
                if (mx > 60 && (mx - mn) > 40)
                    ++n;
            }
        }
        return n;
    };

    const int blockCount = emojis.size() / blockSize;
    QVERIFY(blockCount >= 4);

    int firstBlockPixels = 0;
    int lastBlockPixels = 0;
    for (int b = 0; b < blockCount; ++b) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt, Qt::black);
        drawBlock(b * blockSize, blockSize);
        pd->endPaint();
        rhi->endOffscreenFrame();

        if (impl != QRhi::Null && (b == 0 || b == blockCount - 1)) {
            QImage image = imageFromReadback(rhi.get(), rt->tex);
            const int n = countColorfulPixels(image);
            if (b == 0)
                firstBlockPixels = n;
            else
                lastBlockPixels = n;
        }
    }

    if (impl != QRhi::Null) {
        // If even the first block does not render in color, the platform is not
        // producing color emoji and there is nothing to test.
        if (firstBlockPixels < 500)
            QSKIP("Platform did not render color emoji; cannot test eviction");

        // With broken eviction/reuse this count collapses to something much
        // smaller than 500.
        QCOMPARE_GT(lastBlockPixels, 500);
    }
}

void tst_CanvasRhiRendering::multiPathRender_data()
{
    rhiTestData();
}

// Strokes and fills of several paths are drawn as one triangle strip per
// pass. They must render the same as drawing each path separately.
void tst_CanvasRhiRendering::multiPathRender()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QCPainterEngine *engine = QCanvasPainterPrivate::get(painter)->m_renderer->engine();

    // Subpaths with strips of different lengths: open, closed, round caps
    // and joins, curves. They don't overlap.
    auto addSubpath = [](QCanvasPath &path, int i) {
        const float x = 60 + (i % 4) * 300;
        const float y = 60 + (i / 4) * 220;
        switch (i % 4) {
        case 0:
            path.moveTo(x, y);
            path.lineTo(x + 200, y + 40);
            break;
        case 1:
            path.moveTo(x, y);
            path.lineTo(x + 100, y + 120);
            path.lineTo(x + 200, y);
            break;
        case 2:
            path.rect(x, y, 200, 120);
            break;
        case 3:
            path.circle(x + 80, y + 70, 60);
            break;
        }
    };
    // Only the closed subpaths are filled, as filling a line has
    // differences between convex and non-convex fills.
    auto isClosed = [](int i) { return i % 4 >= 2; };
    constexpr int subpathCount = 8;
    QCanvasPath merged;
    QCanvasPath mergedClosed;
    QList<QCanvasPath> separate(subpathCount);
    for (int i = 0; i < subpathCount; ++i) {
        addSubpath(merged, i);
        if (isClosed(i))
            addSubpath(mergedClosed, i);
        addSubpath(separate[i], i);
    }

    struct Result {
        QImage image;
        int fillDrawCalls = 0;
        int strokeDrawCalls = 0;
        int fillTriangles = 0;
        int strokeTriangles = 0;
    };
    auto render = [&](bool together, bool antialias, bool highQuality, bool cached) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setAntialias(antialias ? 1.0 : 0.0);
        painter->setHighQualityStroking(highQuality);
        painter->setLineWidth(16);
        painter->setLineCap(QCanvasPainter::LineCap::Round);
        painter->setLineJoin(QCanvasPainter::LineJoin::Round);
        painter->setStrokeStyle(QColor(255, 255, 255, 160));
        painter->setFillStyle(QColor(0, 128, 255));
        auto fill = [&](const QCanvasPath &path, int group) {
            if (cached) {
                painter->fill(path, group);
            } else {
                painter->beginPath();
                painter->addPath(path);
                painter->fill();
            }
        };
        auto stroke = [&](const QCanvasPath &path, int group) {
            if (cached) {
                painter->stroke(path, group);
            } else {
                painter->beginPath();
                painter->addPath(path);
                painter->stroke();
            }
        };
        if (together) {
            fill(mergedClosed, 0);
            stroke(merged, 1);
        } else {
            for (int i = 0; i < subpathCount; ++i) {
                if (isClosed(i))
                    fill(separate[i], 2 + 2 * i);
                stroke(separate[i], 3 + 2 * i);
            }
        }
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        engine->syncDebugCounters();
        Result result;
        if (impl != QRhi::Null)
            result.image = imageFromReadback(rhi.get(), rt->tex);
        result.fillDrawCalls = engine->debugCounters().fillDrawCallCount;
        result.strokeDrawCalls = engine->debugCounters().strokeDrawCallCount;
        result.fillTriangles = engine->debugCounters().fillTriangleCount;
        result.strokeTriangles = engine->debugCounters().strokeTriangleCount;
        return result;
    };

    // Without antialiasing, the fills have no strips
    for (bool antialias : { true, false }) {
        for (bool highQuality : { false, true }) {
            for (bool cached : { false, true }) {
                const Result together = render(true, antialias, highQuality, cached);
                const Result separately = render(false, antialias, highQuality, cached);

                // One draw for each stroke pass, instead of one for each subpath:
                // 1 without and 3 with high quality stroking, which has no
                // antialiasing pass without antialiasing
                const int strokePasses = highQuality ? (antialias ? 3 : 2) : 1;
                QCOMPARE(together.strokeDrawCalls, strokePasses);
                QCOMPARE(separately.strokeDrawCalls, subpathCount * strokePasses);
                // A fill with many subpaths has the stencil, antialiasing, and
                // cover passes. The separate fills are convex, with a draw for
                // the fill and for the antialiasing of each. Without
                // antialiasing, there are no antialiasing draws.
                QCOMPARE(together.fillDrawCalls, antialias ? 3 : 2);
                QCOMPARE(separately.fillDrawCalls, (subpathCount / 2) * (antialias ? 2 : 1));
                // The degenerate triangles joining the strips are not counted,
                // and the cover quad of the fill adds 2 triangles
                QCOMPARE(together.strokeTriangles, separately.strokeTriangles);
                QCOMPARE(together.fillTriangles, separately.fillTriangles + 2);

                if (impl == QRhi::Null)
                    continue;
                // The merged fill uses the non-convex fill path, while the
                // separate fills are convex, so their antialiased edges come
                // from different draws. Blending them under the translucent
                // strokes rounds differently between GPUs. Allow those small
                // differences; a missing subpath or a stray triangle between
                // strips differs much more, and over many pixels.
                constexpr int maxAllowedDiff = 16;
                constexpr int smallDiff = 4;
                constexpr int maxPixelsOverSmallDiff = RT_WIDTH * RT_HEIGHT / 1000;
                int maxDiff = 0;
                int pixelsOverSmallDiff = 0;
                for (int y = 0; y < RT_HEIGHT; ++y) {
                    const QRgb *a =
                            reinterpret_cast<const QRgb *>(together.image.constScanLine(y));
                    const QRgb *b =
                            reinterpret_cast<const QRgb *>(separately.image.constScanLine(y));
                    for (int x = 0; x < RT_WIDTH; ++x) {
                        const int diff = std::max({ qAbs(qRed(a[x]) - qRed(b[x])),
                                                    qAbs(qGreen(a[x]) - qGreen(b[x])),
                                                    qAbs(qBlue(a[x]) - qBlue(b[x])) });
                        maxDiff = std::max(maxDiff, diff);
                        if (diff > smallDiff)
                            ++pixelsOverSmallDiff;
                    }
                }
                const QString message =
                        QStringLiteral("antialias %1 highQuality %2 cached %3 maxDiff %4 "
                                       "pixelsOverSmallDiff %5")
                                .arg(antialias).arg(highQuality).arg(cached).arg(maxDiff)
                                .arg(pixelsOverSmallDiff);
                QVERIFY2(maxDiff <= maxAllowedDiff, qPrintable(message));
                QVERIFY2(pixelsOverSmallDiff <= maxPixelsOverSmallDiff, qPrintable(message));
                // The subpaths are there: the line stroke, the rect and circle fills
                QVERIFY(qGray(together.image.pixel(160, 80)) > 100);
                QVERIFY(qBlue(together.image.pixel(760, 120)) > 200);
                QVERIFY(qBlue(together.image.pixel(1040, 130)) > 200);
            }
        }
    }
}

void tst_CanvasRhiRendering::cachedPathVariantsUpdate_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::cachedPathVariantsUpdate()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    // A path in a path group is cached for each set of properties that it is
    // drawn with, here two stroke widths and two antialiasing widths of a fill.
    // When the path changes, drawing it with the first properties updates their
    // data, and drawing it with the second properties must then update theirs
    // as well, not draw the data of the earlier path.
    QCanvasPath strokePath;
    strokePath.moveTo(50, 50);
    strokePath.lineTo(450, 50);
    QCanvasPath fillPath;
    fillPath.rect(50, 300, 150, 100);
    auto render = [&]() {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setStrokeStyle(Qt::white);
        painter->setFillStyle(Qt::white);
        for (int variant = 0; variant < 2; ++variant) {
            painter->resetTransform();
            painter->translate(variant * 600, 0);
            painter->setLineWidth(variant ? 20 : 4);
            painter->stroke(strokePath, 1);
            painter->setAntialias(variant ? 4 : 1);
            painter->fill(fillPath, 1);
        }
        painter->setAntialias(1);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        return impl != QRhi::Null ? imageFromReadback(rhi.get(), rt->tex) : QImage();
    };
    auto isOn = [](const QImage &image, int x, int y) { return qGray(image.pixel(x, y)) > 200; };

    QImage image = render();
    if (!image.isNull()) {
        for (int x : { 0, 600 }) {
            QVERIFY(isOn(image, x + 250, 50));
            QVERIFY(!isOn(image, x + 450, 120));
            QVERIFY(isOn(image, x + 125, 350));
            QVERIFY(!isOn(image, x + 375, 350));
        }
    }

    strokePath.lineTo(450, 150);
    fillPath.rect(300, 300, 150, 100);
    image = render();
    if (!image.isNull()) {
        for (int x : { 0, 600 }) {
            QVERIFY2(isOn(image, x + 250, 50), qPrintable(QString::number(x)));
            QVERIFY2(isOn(image, x + 450, 120), qPrintable(QString::number(x)));
            QVERIFY2(isOn(image, x + 125, 350), qPrintable(QString::number(x)));
            QVERIFY2(isOn(image, x + 375, 350), qPrintable(QString::number(x)));
        }
    }

    // The path changes again, with the same number of commands
    strokePath.clear();
    strokePath.moveTo(50, 50);
    strokePath.lineTo(450, 50);
    strokePath.lineTo(250, 150);
    fillPath.clear();
    fillPath.rect(50, 300, 150, 100);
    fillPath.rect(300, 300, 150, 50);
    image = render();
    if (!image.isNull()) {
        for (int x : { 0, 600 }) {
            QVERIFY2(!isOn(image, x + 450, 120), qPrintable(QString::number(x)));
            QVERIFY2(isOn(image, x + 350, 100), qPrintable(QString::number(x)));
            QVERIFY2(isOn(image, x + 375, 320), qPrintable(QString::number(x)));
            QVERIFY2(!isOn(image, x + 375, 380), qPrintable(QString::number(x)));
        }
    }
}

void tst_CanvasRhiRendering::cachedStrokeScale_data()
{
    rhiTestData();
}

// The stroke of a cached path is expanded in the units of the path, and the
// vertex shader scales it. The width of the line and the antialiasing must
// still be what the stroke has without a path group under the same transform.
void tst_CanvasRhiRendering::cachedStrokeScale()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QCPainterEngine *engine = QCanvasPainterPrivate::get(painter)->m_renderer->engine();

    QCanvasPath path;
    path.moveTo(10, 10);
    path.lineTo(70, 10);
    path.lineTo(70, 40);
    path.lineTo(10, 40);
    path.closePath();
    path.moveTo(10, 10);
    path.lineTo(70, 40);

    auto render = [&](qreal scale, qreal width, qreal antialias, bool cached) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setAntialias(antialias);
        painter->resetTransform();
        painter->translate(20, 20);
        painter->scale(scale);
        painter->setLineWidth(width);
        painter->setLineCap(QCanvasPainter::LineCap::Butt);
        painter->setLineJoin(QCanvasPainter::LineJoin::Miter);
        painter->setStrokeStyle(Qt::white);
        if (cached) {
            painter->stroke(path, 0);
        } else {
            painter->beginPath();
            painter->addPath(path);
            painter->stroke();
        }
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        engine->syncDebugCounters();
        return impl != QRhi::Null ? imageFromReadback(rhi.get(), rt->tex) : QImage();
    };
    // The amount of ink, as the sum of the gray values
    auto ink = [](const QImage &image) {
        qreal sum = 0;
        for (int y = 0; y < image.height(); ++y) {
            const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            for (int x = 0; x < image.width(); ++x)
                sum += qGray(line[x]);
        }
        return sum / 255.0;
    };

    // The cached geometry is made at the first scale and used at the others,
    // when it does not depend on the scale. With antialiasing it does, as the
    // antialiasing is sized in device pixels, so it is made for the scale
    // rounded to a power of two. 1.5 and 0.7 are in between, where the
    // antialiasing is off by up to sqrt(2) but the width of the line is right.
    for (qreal antialias : { 0.0, 1.0 }) {
        for (qreal scale : { 1.0, 2.0, 0.5, 4.0, 1.5, 0.7 }) {
            // A hairline (one device pixel without the transform) and a line
            // that is scaled with the transform
            for (qreal width : { 1.0 / scale, 3.0 }) {
                const QImage cached = render(scale, width, antialias, true);
                const QImage uncached = render(scale, width, antialias, false);
                if (impl == QRhi::Null)
                    continue;
                const qreal cachedInk = ink(cached);
                const qreal uncachedInk = ink(uncached);
                const QString message = QStringLiteral("antialias %1 scale %2 width %3: ink %4, "
                                                       "without the path group %5")
                                                .arg(antialias).arg(scale).arg(width)
                                                .arg(cachedInk).arg(uncachedInk);
                QVERIFY2(uncachedInk > 50, qPrintable(message));
                // Thin lines are sensitive to the rounding of the vertices, but a
                // wrong width is at least 30 % off. With antialiasing between the
                // powers of two the fringe is not the right width, which is most
                // of the ink of a hairline.
                const bool exact = antialias == 0.0
                                   || qFuzzyCompare(std::exp2(std::round(std::log2(scale))), scale);
                QVERIFY2(qAbs(cachedInk - uncachedInk) <= (exact ? 0.1 : 0.4) * uncachedInk,
                         qPrintable(message));
            }
        }
    }

    // The data of other scales of a path is removed, so that zooming does not
    // grow the cache
    int firstSize = -1;
    for (qreal scale : { 1.0, 2.0, 0.5, 4.0, 1.5, 3.0 }) {
        render(scale, 2.0, 1.0, true);
        const int size = engine->debugCounters().cachedPathVertexDataSize;
        if (firstSize < 0)
            firstSize = size;
        QVERIFY(firstSize > 0);
        QCOMPARE(size, firstSize);
    }

    // The data of the scales that the earlier passes of the frame use is kept,
    // so drawing the path at two scales in the same frame doesn't make the data
    // again at every pass. A later frame that strokes the path at a new scale
    // removes the scales that it doesn't use.
    // Without the data of the widths above, which is not dependent on the scale.
    painter->removePathGroup(0);
    auto renderPasses = [&](std::initializer_list<qreal> scales) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        for (qreal scale : scales) {
            pd->beginPaint(cb, rt->rt);
            painter->setAntialias(1.0);
            painter->resetTransform();
            painter->scale(scale);
            painter->setLineWidth(2.0);
            painter->setStrokeStyle(Qt::white);
            painter->stroke(path, 0);
            pd->endPaint();
        }
        rhi->endOffscreenFrame();
        engine->syncDebugCounters();
        return engine->debugCounters().cachedPathVertexDataSize;
    };
    const int sizeAt1 = renderPasses({ 1.0 });
    const int sizeAt4 = renderPasses({ 4.0 });
    QVERIFY(sizeAt1 > 0 && sizeAt4 > 0);
    QCOMPARE(renderPasses({ 1.0, 4.0 }), sizeAt1 + sizeAt4);
    QCOMPARE(renderPasses({ 1.0, 4.0 }), sizeAt1 + sizeAt4);
    QCOMPARE(renderPasses({ 2.0 }), sizeAt4);

    // A new scale removes only the data of the same style, so alternating
    // between two widths on successive frames keeps the data of both
    painter->removePathGroup(0);
    render(1.0, 2.0, 1.0, true);
    const int sizeOfWidth2 = engine->debugCounters().cachedPathVertexDataSize;
    render(1.0, 4.0, 1.0, true);
    const int sizeOfBoth = engine->debugCounters().cachedPathVertexDataSize;
    QVERIFY(sizeOfWidth2 > 0 && sizeOfBoth > sizeOfWidth2);
    for (qreal width : { 2.0, 4.0, 2.0 }) {
        render(1.0, width, 1.0, true);
        QCOMPARE(engine->debugCounters().cachedPathVertexDataSize, sizeOfBoth);
    }
    // A new scale of one of them replaces only its own data
    render(2.0, 2.0, 1.0, true);
    QCOMPARE(engine->debugCounters().cachedPathVertexDataSize, sizeOfBoth);
    render(2.0, 4.0, 1.0, true);
    QCOMPARE(engine->debugCounters().cachedPathVertexDataSize, sizeOfBoth);

    // When the path changes, the data of each scale is made again. Stroking it
    // at the first scale doesn't make the data of the other scale up to date.
    auto renderScales = [&]() {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setAntialias(1.0);
        painter->setLineWidth(3.0);
        painter->setStrokeStyle(Qt::white);
        for (qreal scale : { 1.0, 4.0 }) {
            painter->resetTransform();
            painter->translate(20, 20);
            painter->scale(scale);
            painter->stroke(path, 0);
        }
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        return impl != QRhi::Null ? imageFromReadback(rhi.get(), rt->tex) : QImage();
    };
    auto isOn = [](const QImage &image, int x, int y) { return qGray(image.pixel(x, y)) > 200; };
    QImage image = renderScales();
    if (!image.isNull()) {
        QVERIFY(!isOn(image, 80, 120));
        QVERIFY(!isOn(image, 200, 420));
    }
    path.moveTo(10, 100);
    path.lineTo(70, 100);
    image = renderScales();
    if (!image.isNull()) {
        QVERIFY(isOn(image, 80, 120));
        QVERIFY(isOn(image, 200, 420));
    }
}

void tst_CanvasRhiRendering::cachedPathTransformedClip_data()
{
    rhiTestData();
}

// The points of cached paths are not transformed, and the clip that is made under a transform
// has to be in the coordinates of the points. The result must be what it is without a path group,
// also the antialiasing of the edges of the clip, which is in device pixels.
void tst_CanvasRhiRendering::cachedPathTransformedClip()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");
    if (impl == QRhi::Null)
        QSKIP("Needs the rendered pixels");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    // The stroke of the box is partly inside the clip. The fill of the larger
    // path covers the clip, so the only edges of the fill are those of the clip.
    QCanvasPath box;
    box.rect(0, 0, 180, 90);
    QCanvasPath cover;
    cover.rect(-50, -50, 400, 300);

    auto render = [&](qreal scale, int group, bool stroke) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->resetTransform();
        painter->translate(100, 80);
        painter->scale(scale);
        painter->rotate(0.3);
        painter->setClipRect(QRectF(10, 10, 200, 100));
        painter->setFillStyle(Qt::white);
        painter->setStrokeStyle(Qt::white);
        painter->setLineWidth(8);
        if (stroke)
            painter->stroke(box, group);
        else
            painter->fill(cover, group);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        return imageFromReadback(rhi.get(), rt->tex);
    };

    // At 4 the width of the antialiasing would be 4 pixels if it was scaled with the path
    for (qreal scale : { 1.5, 4.0 }) {
        for (bool stroke : { false, true }) {
            const QImage uncached = render(scale, -1, stroke);
            const QImage cached = render(scale, stroke ? 1 : 0, stroke);
            QCOMPARE(cached.size(), uncached.size());
            int inside = 0;
            int different = 0;
            for (int y = 0; y < cached.height(); ++y) {
                for (int x = 0; x < cached.width(); ++x) {
                    inside += cached.pixel(x, y) != qRgb(0, 0, 0);
                    different += qAbs(qGray(cached.pixel(x, y)) - qGray(uncached.pixel(x, y))) > 64;
                }
            }
            // Something is drawn, as the clip is where the path is, and it is the same
            const QString message = QStringLiteral("scale %1 stroke %2: %3 pixels, %4 differ")
                                            .arg(scale).arg(stroke).arg(inside).arg(different);
            QVERIFY2(inside > 500, qPrintable(message));
            QVERIFY2(different < 20, qPrintable(message));
        }
    }
}

void tst_CanvasRhiRendering::cachedFillScale_data()
{
    rhiTestData();
}

// The antialiasing fringe of a cached fill is made in the units of the path, and the vertex
// shader scales it. It must still be in device pixels, as it is without a path group.
void tst_CanvasRhiRendering::cachedFillScale()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QCPainterEngine *engine = QCanvasPainterPrivate::get(painter)->m_renderer->engine();

    // A convex path, which has half of the fringe, and a concave one, which is
    // filled with the stencil and has all of it
    QCanvasPath convex;
    convex.rect(0, 0, 60, 30);
    QCanvasPath concave;
    concave.moveTo(0, 0);
    concave.lineTo(60, 0);
    concave.lineTo(30, 15);
    concave.lineTo(60, 30);
    concave.lineTo(0, 30);
    concave.closePath();

    auto render = [&](const QCanvasPath &path, qreal scale, qreal antialias, int group) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setAntialias(antialias);
        painter->resetTransform();
        painter->translate(40, 30);
        painter->scale(scale);
        painter->rotate(0.3);
        painter->setFillStyle(Qt::white);
        painter->fill(path, group);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        engine->syncDebugCounters();
        return impl != QRhi::Null ? imageFromReadback(rhi.get(), rt->tex) : QImage();
    };

    // The cached geometry is made at the first scale and used at the others when
    // it does not depend on the scale, without antialiasing. With antialiasing it
    // is made for the scale rounded to a power of two. At 1.5 and 0.7 in between
    // the fringe is off by up to sqrt(2), but not by the scale.
    for (const QCanvasPath *path : { &convex, &concave }) {
        for (qreal antialias : { 0.0, 1.0 }) {
            for (qreal scale : { 1.0, 4.0, 0.5, 8.0, 1.5, 0.7 }) {
                const QImage cached = render(*path, scale, antialias, 0);
                const QImage uncached = render(*path, scale, antialias, -1);
                if (impl == QRhi::Null)
                    continue;
                int inside = 0;
                int different = 0;
                for (int y = 0; y < cached.height(); ++y) {
                    for (int x = 0; x < cached.width(); ++x) {
                        inside += cached.pixel(x, y) != qRgb(0, 0, 0);
                        different += qAbs(qGray(cached.pixel(x, y)) - qGray(uncached.pixel(x, y))) > 64;
                    }
                }
                const QString message = QStringLiteral("%1 antialias %2 scale %3: %4 pixels, %5 differ")
                                                .arg(path == &convex ? "convex" : "concave")
                                                .arg(antialias).arg(scale).arg(inside).arg(different);
                QVERIFY2(inside > 100, qPrintable(message));
                QVERIFY2(different < 20, qPrintable(message));
            }
        }
        painter->removePathGroup(0);
    }

    // The data of other scales of a path is removed, so that zooming does not
    // grow the cache
    int firstSize = -1;
    for (qreal scale : { 1.0, 2.0, 0.5, 4.0, 1.5, 3.0 }) {
        render(concave, scale, 1.0, 0);
        const int size = engine->debugCounters().cachedPathVertexDataSize;
        if (firstSize < 0)
            firstSize = size;
        QVERIFY(firstSize > 0);
        QCOMPARE(size, firstSize);
    }

    // A new scale removes only the data of the same antialiasing, so
    // alternating between two amounts on successive frames keeps the data of
    // both
    painter->removePathGroup(0);
    render(concave, 1.0, 1.0, 0);
    const int sizeOfOne = engine->debugCounters().cachedPathVertexDataSize;
    render(concave, 1.0, 2.0, 0);
    const int sizeOfBoth = engine->debugCounters().cachedPathVertexDataSize;
    QVERIFY(sizeOfOne > 0 && sizeOfBoth > sizeOfOne);
    for (qreal antialias : { 1.0, 2.0, 1.0 }) {
        render(concave, 1.0, antialias, 0);
        QCOMPARE(engine->debugCounters().cachedPathVertexDataSize, sizeOfBoth);
    }
    render(concave, 2.0, 1.0, 0);
    QCOMPARE(engine->debugCounters().cachedPathVertexDataSize, sizeOfBoth);
}

void tst_CanvasRhiRendering::cachedCurveScale_data()
{
    rhiTestData();
}

// The curves of a cached path are flattened in the units of the path. When the
// data is made for a scale, they must be flattened for it, as without a path
// group, or the segments next to a sharp corner differ, and so does its join.
void tst_CanvasRhiRendering::cachedCurveScale()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    // Curves that meet a line at sharp corners
    QCanvasPath path;
    for (int i = 0; i < 4; ++i) {
        const float x = i * 220.0f;
        path.moveTo(x, 60);
        path.quadraticCurveTo(x + 90, 0, x + 180, 60);
        path.closePath();
        path.moveTo(x, 160);
        path.bezierCurveTo(x, 100, x + 140 + i * 10, 100, x + 140 + i * 10, 160);
        path.closePath();
    }

    auto render = [&](qreal scale, bool stroke, int group) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->resetTransform();
        painter->translate(10, 10);
        painter->scale(scale);
        if (stroke) {
            painter->setStrokeStyle(Qt::white);
            painter->setLineWidth(3);
            painter->stroke(path, group);
        } else {
            painter->setFillStyle(Qt::white);
            painter->fill(path, group);
        }
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        return impl != QRhi::Null ? imageFromReadback(rhi.get(), rt->tex) : QImage();
    };

    for (bool stroke : { false, true }) {
        for (qreal scale : { 1.0, 0.5, 0.25, 2.0 }) {
            const QImage cached = render(scale, stroke, 0);
            const QImage uncached = render(scale, stroke, -1);
            painter->removePathGroup(0);
            if (impl == QRhi::Null)
                continue;
            int inside = 0;
            int different = 0;
            for (int y = 0; y < cached.height(); ++y) {
                for (int x = 0; x < cached.width(); ++x) {
                    inside += cached.pixel(x, y) != qRgb(0, 0, 0);
                    different += qAbs(qGray(cached.pixel(x, y)) - qGray(uncached.pixel(x, y))) > 64;
                }
            }
            const QString message = QStringLiteral("%1 scale %2: %3 pixels, %4 differ")
                                            .arg(stroke ? "stroke" : "fill")
                                            .arg(scale).arg(inside).arg(different);
            qInfo("%s", qPrintable(message));
            QVERIFY2(inside > 100, qPrintable(message));
            QVERIFY2(different < 3, qPrintable(message));
        }
    }
}

// Only affine transforms are supported. The perspective part of a transform is
// ignored, so that it does not apply to some of the drawing and not the rest.
void tst_CanvasRhiRendering::perspectiveTransformIgnored()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi);
    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);

    const QTransform perspective(1, 0.5, 0.002,
                                 -0.5, 1, 0.001,
                                 20, 30, 1.5);
    const QTransform affine(1, 0.5, -0.5, 1, 20, 30);

    painter->setTransform(perspective);
    QCOMPARE(painter->getTransform(), affine);

    painter->resetTransform();
    painter->translate(10, 0);
    painter->transform(perspective);
    QCOMPARE(painter->getTransform(), affine * QTransform::fromTranslate(10, 0));

    // Affine transforms are used as they are
    const QTransform scaled = QTransform(affine).scale(2, 3);
    painter->setTransform(scaled);
    QCOMPARE(painter->getTransform(), scaled);
}

void tst_CanvasRhiRendering::restoreClip_data()
{
    rhiTestData();
}

// restore() restores the clip, also whether there is one and of which kind.
void tst_CanvasRhiRendering::restoreClip()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    const QRectF all(0, 0, RT_WIDTH, RT_HEIGHT);
    const QRectF corner(0, 0, 20, 20);
    // Each sets up the clip and returns whether the center is clipped
    const std::function<bool()> clips[] = {
        // The clip set after save() is removed
        [&] {
            painter->save();
            painter->setClipRect(corner);
            painter->restore();
            return false;
        },
        [&] {
            painter->save();
            painter->rotate(0.1);
            painter->setClipRect(corner);
            painter->restore();
            return false;
        },
        // The clip removed after save() is back
        [&] {
            painter->setClipRect(corner);
            painter->save();
            painter->resetClipping();
            painter->restore();
            return true;
        },
        [&] {
            painter->rotate(0.1);
            painter->setClipRect(corner);
            painter->resetTransform();
            painter->save();
            painter->resetClipping();
            painter->restore();
            return true;
        },
        // The clip changed to the other kind after save() is back
        [&] {
            painter->setClipRect(corner);
            painter->save();
            painter->rotate(0.1);
            painter->setClipRect(all);
            painter->restore();
            return true;
        },
        [&] {
            painter->rotate(0.1);
            painter->setClipRect(corner);
            painter->resetTransform();
            painter->save();
            painter->setClipRect(all);
            painter->restore();
            return true;
        },
    };

    for (const auto &clip : clips) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        const bool centerClipped = clip();
        painter->setFillStyle(Qt::white);
        painter->fillRect(all);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        if (impl == QRhi::Null)
            continue;
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 5, 5, Qt::white));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2,
                          centerClipped ? Qt::black : Qt::white));
    }
}

void tst_CanvasRhiRendering::pathWindingNotKept_data()
{
    rhiTestData();
}

// The winding set with setPathWinding() is for the subpaths of that path. The
// next path starts with the default winding, also when save() and restore()
// are called around filling the earlier path.
void tst_CanvasRhiRendering::pathWindingNotKept()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    for (bool saveAroundFill : { false, true }) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setFillStyle(Qt::white);
        // A path that ends with a hole
        painter->beginPath();
        painter->rect(0, 0, 40, 40);
        painter->setPathWinding(QCanvasPainter::PathWinding::ClockWise);
        painter->rect(10, 10, 20, 20);
        if (saveAroundFill)
            painter->save();
        painter->fill();
        if (saveAroundFill)
            painter->restore();
        // A rect with a hole in the center
        painter->beginPath();
        painter->rect(0, 0, RT_WIDTH, RT_HEIGHT);
        painter->setPathWinding(QCanvasPainter::PathWinding::ClockWise);
        painter->rect(RT_WIDTH / 2 - 50, RT_HEIGHT / 2 - 50, 100, 100);
        painter->fill();
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        if (impl == QRhi::Null)
            continue;
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, RT_WIDTH / 2 - 100, RT_HEIGHT / 2, Qt::white));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::black));
    }
}

void tst_CanvasRhiRendering::clockwiseFillWithoutWindingEnforce_data()
{
    rhiTestData();
}

// Without the winding enforcement the paths keep their direction. A fill that
// goes clockwise must still have its antialiasing fringe outside its edges,
// and a convex one must still be drawn without the stencil, like the same fill
// going counterclockwise. A hole going the other way gets the fringe inside it.
void tst_CanvasRhiRendering::clockwiseFillWithoutWindingEnforce()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QCPainterEngine *engine = QCanvasPainterPrivate::get(painter)->m_renderer->engine();

    // A rect going counterclockwise on the screen, or clockwise when reversed
    auto addRect = [](QCanvasPath *path, const QRectF &r, bool reversed) {
        QList<QPointF> points { r.topLeft(), r.bottomLeft(), r.bottomRight(), r.topRight() };
        if (reversed)
            std::reverse(points.begin(), points.end());
        path->moveTo(points.at(0));
        for (int i = 1; i < points.size(); ++i)
            path->lineTo(points.at(i));
        path->closePath();
    };
    auto makePath = [&](bool withHole, bool clockwise) {
        QCanvasPath path;
        addRect(&path, QRectF(20.3, 20.3, 120.4, 80.4), clockwise);
        if (withHole)
            addRect(&path, QRectF(60.5, 40.5, 40, 40), !clockwise);
        return path;
    };

    struct Result {
        QImage image;
        int fillTriangles;
    };
    auto render = [&](const QCanvasPath &path, int group) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setWindingEnforce(false);
        painter->setFillStyle(Qt::white);
        painter->fill(path, group);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        engine->syncDebugCounters();
        return Result { impl != QRhi::Null ? imageFromReadback(rhi.get(), rt->tex) : QImage(),
                        engine->debugCounters().fillTriangleCount };
    };

    for (bool withHole : { false, true }) {
        for (int group : { -1, 0 }) {
            const QCanvasPath ccwPath = makePath(withHole, false);
            const QCanvasPath cwPath = makePath(withHole, true);
            const Result ccw = render(ccwPath, group);
            const Result cw = render(cwPath, group);
            const QString message = QStringLiteral("hole %1 group %2").arg(withHole).arg(group);
            if (impl != QRhi::Null) {
                int different = 0;
                for (int y = 0; y < ccw.image.height(); ++y) {
                    for (int x = 0; x < ccw.image.width(); ++x) {
                        different += qAbs(qGray(cw.image.pixel(x, y))
                                          - qGray(ccw.image.pixel(x, y))) > 8;
                    }
                }
                QVERIFY2(different == 0, qPrintable(message + QStringLiteral(": %1 pixels differ")
                                                              .arg(different)));
                // The edge is antialiased, most of the pixel at 20 is covered
                const int edge = qGray(cw.image.pixel(20, 60));
                QVERIFY2(edge > 64 && edge < 230, qPrintable(message + QStringLiteral(": edge %1")
                                                                          .arg(edge)));
            }
            // Without the stencil the convex fill has fewer triangles
            QVERIFY2(cw.fillTriangles == ccw.fillTriangles, qPrintable(message));
        }
        painter->removePathGroup(0);
    }
}

void tst_CanvasRhiRendering::shapeDirection_data()
{
    rhiTestData();
}

// Like in the HTML canvas, rect(), roundRect(), ellipse() and circle() start
// at the top-left corner or at the angle 0 and go clockwise. Without the
// winding enforcement and with the nonzero fill rule, a subpath going
// counterclockwise inside them is a hole. The antialiasing fringe must still be
// outside the edges of the shape and inside the hole, without a gap.
void tst_CanvasRhiRendering::shapeDirection()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    const qreal cx = RT_WIDTH / 2;
    const qreal cy = RT_HEIGHT / 2;
    // Adds the shape and returns its start point
    const auto addShape = [&](auto *target, int shape) -> QPointF {
        switch (shape) {
        case 0:
            target->rect(cx - 200, cy - 200, 400, 400);
            return { cx - 200, cy - 200 };
        case 1:
            target->roundRect(cx - 200, cy - 200, 400, 400, 40);
            return { cx - 160, cy - 200 };
        case 2:
            target->roundRect(cx - 200, cy - 200, 400, 400, 40, 30, 20, 10);
            return { cx - 160, cy - 200 };
        case 3:
            target->ellipse(cx, cy, 250, 200);
            return { cx + 250, cy };
        default:
            target->circle(cx, cy, 200);
            return { cx + 200, cy };
        }
    };
    // A square going counterclockwise on the screen
    const auto addHole = [&](auto *target) {
        target->moveTo(cx - 50, cy - 50);
        target->lineTo(cx - 50, cy + 50);
        target->lineTo(cx + 50, cy + 50);
        target->lineTo(cx + 50, cy - 50);
        target->closePath();
    };

    for (int shape = 0; shape < 5; ++shape) {
        // Painter path, QCanvasPath, and QCanvasPath in a path group
        for (int mode = 0; mode < 3; ++mode) {
            QRhiCommandBuffer *cb;
            rhi->beginOffscreenFrame(&cb);
            pd->resetForNewFrame();
            pd->beginPaint(cb, rt->rt);
            painter->setWindingEnforce(false);
            painter->setFillStyle(Qt::white);
            if (mode == 0) {
                painter->beginPath();
                addShape(painter, shape);
                addHole(painter);
                painter->fill();
            } else {
                QCanvasPath path;
                const QPointF start = addShape(&path, shape);
                QCOMPARE(path.positionAt(0), start);
                addHole(&path);
                painter->fill(path, mode == 2 ? 0 : -1);
            }
            pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
            cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
            pd->renderPaint();
            cb->endPass();
            rhi->endOffscreenFrame();
            if (impl == QRhi::Null)
                continue;
            const QImage image = imageFromReadback(rhi.get(), rt->tex);
            const QString message = QString("shape %1, mode %2").arg(shape).arg(mode);
            QVERIFY2(testColor(image, cx - 120, cy, Qt::white), qPrintable(message));
            QVERIFY2(testColor(image, cx, cy, Qt::black), qPrintable(message));
            // The left edges of the shape and the hole are on the pixel
            // boundaries. The pixels next to them on the filled side are
            // covered, and those on the other side are not, so that the fringe
            // is not drawn apart from the edge.
            const int left = shape == 3 ? cx - 250 : cx - 200;
            const int hole = int(cx) - 50;
            for (int x : { left - 2, left - 1, left, left + 1,
                           hole - 2, hole - 1, hole, hole + 1 }) {
                const bool filled = (x >= left && x < hole);
                const int gray = qGray(image.pixel(x, cy));
                QVERIFY2(filled ? gray > 200 : gray < 32,
                         qPrintable(message + QString(": x %1 is %2").arg(x).arg(gray)));
            }
        }
    }
}

void tst_CanvasRhiRendering::tessellationLevelLimit_data()
{
    rhiTestData();
}

// A curve that is not flat enough at the deepest level of subdividing it is
// still drawn, with the segments of that level.
void tst_CanvasRhiRendering::tessellationLevelLimit()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    // A circle so large that its curves need more segments than the deepest
    // level gives. Its top is at the center of the render target.
    const float radius = 1.0e7f;
    QCanvasPath path;
    path.circle(RT_WIDTH / 2, RT_HEIGHT / 2 + radius, radius);

    for (int group : { -1, 0 }) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setFillStyle(Qt::white);
        painter->fill(path, group);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        if (impl == QRhi::Null)
            continue;
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2 - 10, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2 + 10, Qt::white));
        QVERIFY(testColor(image, 10, RT_HEIGHT - 10, Qt::white));
    }
}

void tst_CanvasRhiRendering::zeroSizeCurves_data()
{
    rhiTestData();
}

// A curve of zero size, like those of a circle with the radius 0, ends at its
// start. It is not subdivided, and adds no point, so that it does not make a
// segment of zero length, which would change the joins of a stroke.
void tst_CanvasRhiRendering::zeroSizeCurves()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QCPainterEngine *engine = QCanvasPainterPrivate::get(painter)->m_renderer->engine();

    // The circle is a point and four curves of zero size
    QCanvasPath circle;
    circle.circle(100, 100, 0);
    QCanvasPath point;
    point.moveTo(100, 100);

    // A corner with a curve of zero size, like that of a rounded rectangle
    // with the radius 0 at the corner
    QCanvasPath cornerWithCurve;
    cornerWithCurve.moveTo(0, 0);
    cornerWithCurve.lineTo(100, 0);
    cornerWithCurve.bezierCurveTo(100, 0, 100, 0, 100, 0);
    cornerWithCurve.lineTo(100, 100);
    QCanvasPath corner;
    corner.moveTo(0, 0);
    corner.lineTo(100, 0);
    corner.lineTo(100, 100);

    auto cachedSize = [&](const QCanvasPath &path, bool stroke) {
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        pd->resetForNewFrame();
        pd->beginPaint(cb, rt->rt);
        painter->setLineWidth(10);
        painter->setLineJoin(QCanvasPainter::LineJoin::Round);
        if (stroke)
            painter->stroke(path, 0);
        else
            painter->fill(path, 0);
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
        cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
        pd->renderPaint();
        cb->endPass();
        rhi->endOffscreenFrame();
        engine->syncDebugCounters();
        const int size = engine->debugCounters().cachedPathVertexDataSize;
        painter->removePathGroup(0);
        return size;
    };

    for (bool stroke : { false, true }) {
        QCOMPARE(cachedSize(circle, stroke), cachedSize(point, stroke));
        QCOMPARE(cachedSize(cornerWithCurve, stroke), cachedSize(corner, stroke));
    }
}

#include <tst_qcrhiplumbing.moc>
QTEST_MAIN(tst_CanvasRhiRendering)
