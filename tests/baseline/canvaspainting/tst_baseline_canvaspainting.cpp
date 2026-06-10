// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qbaselinetest.h>
#include <QColor>
#include <QSurfaceFormat>
#include <QOffscreenSurface>
#include <rhi/qrhi.h>

#include <QCanvasPainter>
#include <QCanvasPainterFactory>
#include <QCanvasRhiPaintDriver>
#include <QCanvasOffscreenCanvas>
#include <QCanvasImagePattern>

#include "canvaspainting_cpptests.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#if QT_CONFIG(process)
#include <QtCore/QProcess>
#endif
#include <algorithm>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

// Undefine to disable sending the images to the server.
// In that case the results are saved to png files (result_backend_name.png)
#define USE_SERVER

//#define FRAME_CAPTURE

//#define ONLY_THIS_TEST "testSomeText"

#ifdef FRAME_CAPTURE
#include <QtGui/private/qgraphicsframecapture_p.h>
#endif

static QString offscreenCanvasSuffix = "_OffscreenCanvas";

static quint16 checksumFileOrDir(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable())
        return 0;
    if (fi.isFile()) {
        QFile f(path);
        const bool isBinary = path.endsWith(QLatin1String(".png")) || path.endsWith(QLatin1String(".jpg"));
        if (!f.open(isBinary ? QIODevice::ReadOnly : QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << "Failed to open file" << path << f.errorString();
            return 0;
        }
        return qChecksum(f.readAll());
    }
    if (fi.isDir()) {
        static const QStringList nameFilters = {
            QLatin1String("*.qml"), QLatin1String("*.cpp"),
            QLatin1String("*.png"), QLatin1String("*.jpg")
        };
        quint16 cs = 0;
        for (const QString &item : QDir(fi.filePath()).entryList(nameFilters, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
            cs ^= checksumFileOrDir(path + QLatin1Char('/') + item);
        return cs;
    }
    return 0;
}

class tst_CanvasPainterLancelot : public QObject
{
Q_OBJECT

public:
    tst_CanvasPainterLancelot();

private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();

#ifdef Q_OS_WIN
    void testD3D11_data();
    void testD3D11();
    void testD3D12_data();
    void testD3D12();
#endif

#if QT_CONFIG(vulkan)
    void testVulkan_data();
    void testVulkan();
#endif

#if QT_CONFIG(opengl)
    void testOpenGL_data();
    void testOpenGL();
#endif

#if QT_CONFIG(metal)
    void testMetal_data();
    void testMetal();
#endif

    void testCanvas2D_data();
    void testCanvas2D();

private:
    struct RI { // RenderingInfrastructure
#if QT_CONFIG(opengl)
        std::unique_ptr<QOffscreenSurface> fallbackSurface;
#endif
        std::unique_ptr<QRhi> rhi;
        std::unique_ptr<QRhiTexture> tex;
        std::unique_ptr<QRhiRenderBuffer> ds;
        std::unique_ptr<QRhiTextureRenderTarget> rt;
        std::unique_ptr<QRhiRenderPassDescriptor> rp;
        std::unique_ptr<QCanvasPainterFactory> painterFactory;

        ~RI() {
            painterFactory.reset();
            rp.reset();
            rt.reset();
            ds.reset();
            tex.reset();
            rhi.reset();
#if QT_CONFIG(opengl)
            fallbackSurface.reset();
#endif
        }
    };

    bool createRhi(RI *ri, QRhi::Implementation api);
    void setupTestSuite(const QStringList& blacklist = QStringList());
    void runTestSuite(QRhi::Implementation api, QImage::Format format);
    void paint(const QString &methodName, QCanvasPainter *painter, QImage::Format format);

    void setupCanvas2DTestSuite();
    bool renderAndGrab(const QString &qmlFile, QImage *screenshot, QString *errMsg);

#if QT_CONFIG(vulkan)
    QVulkanInstance m_vulkanInstance;
#endif

#ifdef FRAME_CAPTURE
    std::unique_ptr<QGraphicsFrameCapture> m_cap;
#endif

    bool m_checkersBackground = true;

    QCanvasPainter *m_painter;
    QCanvasOffscreenCanvas m_checkerPatternCanvas;
    QCanvasImage m_checkerPatternImage;
    QCanvasImagePattern m_checkerPattern;

    QCanvasOffscreenCanvas m_offscreenCanvas;

    CanvasPainterLancelotCppTests cppTests;

    QString m_canvas2dSuitePath;
    int m_grabberTimeout;
    int m_canvas2dConsecutiveErrors = 0;
    bool m_canvas2dAborted = false;
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

tst_CanvasPainterLancelot::tst_CanvasPainterLancelot()
{
    int sceneTimeout = qEnvironmentVariableIntValue("LANCELOT_SCENE_TIMEOUT");
    if (!sceneTimeout)
        sceneTimeout = 6000;
    m_grabberTimeout = (sceneTimeout * 4) / 3;
}

void tst_CanvasPainterLancelot::initTestCase()
{
    // Check and setup the environment. We treat failures because of test environment
    // (e.g. script files not found) as just warnings, and not QFAILs, to avoid false negatives
    // caused by environment or server instability

#ifdef USE_SERVER
    QByteArray msg;
    if (!QBaselineTest::connectToBaselineServer(&msg))
        QSKIP(msg);
#endif

#ifdef FRAME_CAPTURE
    m_cap = createFrameCapture();
#endif

#if QT_CONFIG(opengl)
    QSurfaceFormat fmt;
    // not really required since there is no window and we render to a texture
    // and explicitly manage the depth-stencil buffer, but just in case
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
#ifdef Q_OS_MACOS
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
    QSurfaceFormat::setDefaultFormat(fmt);
#endif

#if QT_CONFIG(vulkan)
    m_vulkanInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
    if (!m_vulkanInstance.create())
        qWarning("Failed to create Vulkan instance, will not do testing on Vulkan");
#endif

    {
        QString dataDir = QFINDTESTDATA("../data/.");
        if (dataDir.isEmpty())
            dataDir = QStringLiteral("../data");
        const QFileInfo fi(dataDir);
        if (fi.exists() && fi.isDir() && fi.isReadable())
            m_canvas2dSuitePath = fi.canonicalFilePath();
        else
            qWarning() << "Canvas2D test suite data directory missing or unreadable:" << fi.filePath();
    }
}

void tst_CanvasPainterLancelot::init()
{
#ifdef USE_SERVER
    // This gets called for every row. QSKIP if current item is blacklisted on the baseline server:
    QBASELINE_SKIP_IF_BLACKLISTED;
#endif
}

void tst_CanvasPainterLancelot::cleanupTestCase()
{
#ifdef USE_SERVER
    QBaselineTest::finalizeAndDisconnect();
#endif
}

bool tst_CanvasPainterLancelot::createRhi(RI *ri, QRhi::Implementation api)
{
    QRhi::Flags rhiCreateFlags;
#ifdef FRAME_CAPTURE
    rhiCreateFlags |= QRhi::EnableDebugMarkers;
#endif

    if (api == QRhi::Null) {
        QRhiNullInitParams params;
        ri->rhi.reset(QRhi::create(QRhi::Null, &params, rhiCreateFlags));
    }

#if QT_CONFIG(opengl)
    if (api == QRhi::OpenGLES2) {
        ri->fallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams params;
        params.fallbackSurface = ri->fallbackSurface.get();
        ri->rhi.reset(QRhi::create(QRhi::OpenGLES2, &params, rhiCreateFlags));
    }
#endif

#if QT_CONFIG(vulkan)
    if (api == QRhi::Vulkan) {
        QRhiVulkanInitParams params;
        params.inst = &m_vulkanInstance;
        ri->rhi.reset(QRhi::create(QRhi::Vulkan, &params, rhiCreateFlags));
    }
#endif

#ifdef Q_OS_WIN
    if (api == QRhi::D3D11) {
        QRhiD3D11InitParams params;
        ri->rhi.reset(QRhi::create(QRhi::D3D11, &params, rhiCreateFlags));
    } else if (api == QRhi::D3D12) {
        QRhiD3D12InitParams params;
        ri->rhi.reset(QRhi::create(QRhi::D3D12, &params, rhiCreateFlags));
    }
#endif

#if QT_CONFIG(metal)
    if (api == QRhi::Metal) {
        QRhiMetalInitParams params;
        ri->rhi.reset(QRhi::create(QRhi::Metal, &params, rhiCreateFlags));
    }
#endif

    if (!ri->rhi)
        return false;

    ri->tex.reset(ri->rhi->newTexture(QRhiTexture::RGBA8, QSize(WIDTH, HEIGHT), 1,
                                      QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!ri->tex->create())
        return false;
    ri->ds.reset(ri->rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, QSize(WIDTH, HEIGHT)));
    if (!ri->ds->create())
        return false;
    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ ri->tex.get() });
    rtDesc.setDepthStencilBuffer(ri->ds.get());
    ri->rt.reset(ri->rhi->newTextureRenderTarget(rtDesc));
    ri->rp.reset(ri->rt->newCompatibleRenderPassDescriptor());
    ri->rt->setRenderPassDescriptor(ri->rp.get());
    if (!ri->rt->create())
        return false;

    ri->painterFactory.reset(new QCanvasPainterFactory);
    if (!ri->painterFactory->create(ri->rhi.get()))
        return false;

    return true;
}

void tst_CanvasPainterLancelot::setupTestSuite(const QStringList& blacklist)
{
    QTest::addColumn<QString>("methodNameWithSuffix");
    for (const QString &cppTestKey : cppTests.keys()) {
        if (blacklist.contains(cppTestKey))
            continue;
#ifdef ONLY_THIS_TEST
        if (cppTestKey == QLatin1String(ONLY_THIS_TEST)) {
#endif
            const QString offscreenCanvasTestKey = cppTestKey + offscreenCanvasSuffix;
            QBaselineTest::newRow(cppTestKey.toLatin1()) << cppTestKey;
            QBaselineTest::newRow(offscreenCanvasTestKey.toLatin1()) << offscreenCanvasTestKey;
#ifdef ONLY_THIS_TEST
        }
#endif
    }
}

void tst_CanvasPainterLancelot::runTestSuite(QRhi::Implementation api, QImage::Format format)
{
    QFETCH(QString, methodNameWithSuffix);
    QString methodName = methodNameWithSuffix;
    bool isOffscreen = false;
    if (methodName.endsWith(offscreenCanvasSuffix)) {
        isOffscreen = true;
        methodName = methodName.left(methodName.size() - offscreenCanvasSuffix.size());
    }

    RI ri;
    if (!createRhi(&ri, api))
        QSKIP("Failed to set up rendering infrastructure, skipping test");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), ri.rhi.get());
    startFrameCapture(m_cap.get(), ri.rhi.get(), "render");
#endif

    QCanvasPainter *painter = ri.painterFactory->painter();
    QCanvasRhiPaintDriver *pd = ri.painterFactory->paintDriver();

    QRhiCommandBuffer *cb;
    QRhiReadbackResult readbackResult;

    ri.rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();

    m_checkerPatternCanvas = painter->createCanvas(QSize(20, 20));
    m_checkerPatternCanvas.setFillColor(Qt::white);
    pd->beginPaint(m_checkerPatternCanvas, cb);
    painter->setFillStyle(QColor::fromRgba(0xffdfdfdf));
    painter->fillRect(0, 0, 10, 10);
    painter->setFillStyle(QColor::fromRgba(0xffdfdfdf));
    painter->fillRect(10, 10, 10, 10);
    pd->endPaint();
    m_checkerPatternImage = painter->addImage(m_checkerPatternCanvas, QCanvasPainter::ImageFlag::Repeat);
    m_checkerPattern = QCanvasImagePattern(m_checkerPatternImage);

    if (!isOffscreen) {
        pd->beginPaint(cb, ri.rt.get(), Qt::white);
    } else {
        m_offscreenCanvas = painter->createCanvas(QSize(WIDTH, HEIGHT));
        m_offscreenCanvas.setFillColor(Qt::transparent);
        pd->beginPaint(m_offscreenCanvas, cb);
    }

    paint(methodName, painter, format);

    if (isOffscreen) {
        painter->reset();
        painter->setStencilClip({}); // internal feature so not included in reset(), but some tests set it

        painter->setStrokeStyle(Qt::red);
        painter->setLineWidth(4);
        painter->strokeRect(0, 0, WIDTH, HEIGHT);
        painter->setFillStyle(Qt::red);
        QFont f;
        f.setPointSize(12);
        f.setBold(true);
        painter->setFont(f);
        painter->setTextAlign(QCanvasPainter::TextAlign::Right);
        painter->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
        painter->fillText("This was drawn onto an offscreen canvas", QRectF(0, 0, WIDTH, HEIGHT));
    }

    pd->endPaint();

    if (isOffscreen) {
        pd->beginPaint(cb, ri.rt.get(), Qt::white);
        painter->drawImage(painter->addImage(m_offscreenCanvas), 0, 0);
        pd->endPaint();
    }

    QRhiResourceUpdateBatch *u = ri.rhi->nextResourceUpdateBatch();
    u->readBackTexture({ ri.tex.get() }, &readbackResult);
    cb->resourceUpdate(u);
    ri.rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    QImage image(reinterpret_cast<const uchar *>(readbackResult.data.constData()),
                readbackResult.pixelSize.width(),
                readbackResult.pixelSize.height(),
                QImage::Format_RGBA8888);
    if (ri.rhi->isYUpInFramebuffer())
        image.flip();
    image = image.convertToFormat(format);

#ifdef USE_SERVER
    QBASELINE_TEST(image);
#else
    const char *backendName = ri.rhi->backendName();
    image.save(QString::asprintf("result_%s_%s.png", backendName, qPrintable(methodNameWithSuffix)));
#endif
}

void tst_CanvasPainterLancelot::paint(const QString &methodName, QCanvasPainter *painter, QImage::Format)
{
    if (m_checkersBackground) {
        painter->save();
        painter->setFillStyle(m_checkerPattern);
        painter->fillRect(0, 0, WIDTH, HEIGHT);
        painter->restore();
    }

    cppTests.run(methodName, painter, QSize(WIDTH, HEIGHT));
}

#ifdef Q_OS_WIN
void tst_CanvasPainterLancelot::testD3D11_data()
{
    setupTestSuite();
}

void tst_CanvasPainterLancelot::testD3D11()
{
    runTestSuite(QRhi::D3D11, QImage::Format_RGBA8888);
}

void tst_CanvasPainterLancelot::testD3D12_data()
{
    setupTestSuite();
}

void tst_CanvasPainterLancelot::testD3D12()
{
    runTestSuite(QRhi::D3D12, QImage::Format_RGBA8888);
}
#endif //  Q_OS_WIN

#if QT_CONFIG(vulkan)
void tst_CanvasPainterLancelot::testVulkan_data()
{
    setupTestSuite();
}

void tst_CanvasPainterLancelot::testVulkan()
{
    if (!m_vulkanInstance.isValid())
        QSKIP("Failed to initialize Vulkan, skip test");

    runTestSuite(QRhi::Vulkan, QImage::Format_RGBA8888);
}
#endif

#if QT_CONFIG(opengl)
void tst_CanvasPainterLancelot::testOpenGL_data()
{
    setupTestSuite();
}

void tst_CanvasPainterLancelot::testOpenGL()
{
    runTestSuite(QRhi::OpenGLES2, QImage::Format_RGBA8888);
}
#endif

#if QT_CONFIG(metal)
void tst_CanvasPainterLancelot::testMetal_data()
{
    setupTestSuite();
}

void tst_CanvasPainterLancelot::testMetal()
{
    runTestSuite(QRhi::Metal, QImage::Format_RGBA8888);
}
#endif

void tst_CanvasPainterLancelot::setupCanvas2DTestSuite()
{
#ifdef ONLY_THIS_TEST
    QSKIP("Skipping .qml tests in ONLY_THIS_TEST mode.");
    return;
#endif

    QTest::addColumn<QString>("qmlFile");

    if (m_canvas2dSuitePath.isEmpty())
        QSKIP("Canvas2D test suite data directory not found");

    QStringList ignoreItems;
    QFile ignoreFile(m_canvas2dSuitePath + QLatin1String("/Ignore"));
    if (ignoreFile.open(QIODevice::ReadOnly)) {
        while (!ignoreFile.atEnd()) {
            const QByteArray line = ignoreFile.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith('#'))
                ignoreItems += QString::fromLatin1(line);
        }
    }

    QStringList itemFiles;
    QDirIterator it(m_canvas2dSuitePath, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString fp = it.next();
        if (fp.endsWith(QLatin1String(".qml"))) {
            const QString itemName = fp.mid(m_canvas2dSuitePath.length() + 1);
            if (!ignoreItems.contains(itemName))
                itemFiles.append(fp);
        }
    }
    std::sort(itemFiles.begin(), itemFiles.end());

    int numItems = 0;
    for (const QString &filePath : std::as_const(itemFiles)) {
        const QByteArray itemName = filePath.mid(m_canvas2dSuitePath.length() + 1).toLatin1();
        QBaselineTest::newRow(itemName, checksumFileOrDir(filePath)) << filePath;
        numItems++;
    }

    if (!numItems)
        QSKIP("No .qml test files found in " + m_canvas2dSuitePath.toLatin1());
}

void tst_CanvasPainterLancelot::testCanvas2D_data()
{
    setupCanvas2DTestSuite();
    m_canvas2dConsecutiveErrors = 0;
    m_canvas2dAborted = false;
}

void tst_CanvasPainterLancelot::testCanvas2D()
{
    if (m_canvas2dAborted)
        QSKIP("System too unstable.");

    QFETCH(QString, qmlFile);

    QImage screenShot;
    QString errorMessage;
    if (renderAndGrab(qmlFile, &screenShot, &errorMessage)) {
        m_canvas2dConsecutiveErrors = 0;
    } else {
        if (++m_canvas2dConsecutiveErrors >= 3 && QBaselineTest::shouldAbortIfUnstable())
            m_canvas2dAborted = true;
        QFAIL(qPrintable(QLatin1String("QML scene grabbing failed: ") + errorMessage));
    }

#ifdef USE_SERVER
    QBASELINE_TEST(screenShot);
#else
    const QByteArray fn = QFileInfo(qmlFile).baseName().toLatin1();
    screenShot.save(QString::asprintf("result_canvas2d_%s.png", fn.constData()));
#endif
}

bool tst_CanvasPainterLancelot::renderAndGrab(const QString &qmlFile, QImage *screenshot, QString *errMsg)
{
#if QT_CONFIG(process)
#if defined(Q_OS_WIN)
    const bool usePipe = false;
#else
    const bool usePipe = true;
#endif
    QProcess grabber;
    grabber.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    const QString cmd = QCoreApplication::applicationDirPath() + QDir::separator()
                        + QLatin1String("canvaspainter_qmlscenegrabber");
    QStringList args;
#if defined(Q_OS_WIN)
    args << QLatin1String("-platform") << QLatin1String("windows:fontengine=freetype");
#elif defined(Q_OS_DARWIN)
    args << QLatin1String("-platform") << QLatin1String("cocoa:fontengine=freetype");
#endif
    const QString tmpfile = usePipe
        ? QLatin1String("-")
        : QString::fromLatin1("%1/canvaspainter-qmlscenegrabber-%2-out.ppm")
              .arg(QDir::tempPath()).arg(QCoreApplication::applicationPid());
    args << qmlFile << QLatin1String("-o") << tmpfile;

    grabber.start(cmd, args, QIODevice::ReadOnly);
    grabber.waitForFinished(m_grabberTimeout);
    if (grabber.state() != QProcess::NotRunning) {
        grabber.terminate();
        grabber.waitForFinished(m_grabberTimeout / 4);
    }

    QImage img;
    const bool res = usePipe ? img.load(&grabber, "ppm") : img.load(tmpfile);
    if (!res || img.isNull()) {
        if (errMsg) {
            *errMsg = QString::fromLatin1("Failed to grab screen. qmlscenegrabber exitcode: %1. Process error: %2.")
                          .arg(grabber.exitCode()).arg(grabber.errorString());
        }
        if (!usePipe)
            QFile::remove(tmpfile);
        return false;
    }
    if (screenshot)
        *screenshot = img;
    if (!usePipe)
        QFile::remove(tmpfile);
    return true;
#else
    Q_UNUSED(qmlFile);
    Q_UNUSED(screenshot);
    if (errMsg)
        *errMsg = QLatin1String("QProcess not available");
    return false;
#endif // QT_CONFIG(process)
}

QBASELINETEST_MAIN(tst_CanvasPainterLancelot);

#include "tst_baseline_canvaspainting.moc"
