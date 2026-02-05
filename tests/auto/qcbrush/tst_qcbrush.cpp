// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <qdebug.h>

#include "qcpainter.h"
#include "qclineargradient.h"
#include "qcradialgradient.h"
#include "qcconicalgradient.h"
#include "qcboxgradient.h"
#include "qcboxshadow.h"
#include "qcgridpattern.h"
#include "qcimagepattern.h"
#include "qcimage.h"

class tst_QCBrush : public QObject
{
    Q_OBJECT

private slots:
    // Brush autotests
    void testEqual();
    void testDataStreams();
    void testDebugs();
    void testTypes();
    void testQVariantConversion();
    void testGradientStops();
};

void tst_QCBrush::testEqual()
{
    QCLinearGradient lg(10, 10, 100, 100);
    QCLinearGradient lg2(10, 10, 100, 100);
    QVERIFY(lg == lg2);
    lg.setColorAt(0, Qt::red);
    lg.setColorAt(0.5f, Qt::blue);
    lg.setColorAt(1, Qt::green);
    QVERIFY(lg != lg2);
    lg2.setStops(lg.stops());
    QVERIFY(lg == lg2);
    auto lg3 = lg2;
    QVERIFY(lg == lg3);

    QCRadialGradient rg1;
    QCRadialGradient rg2;
    QVERIFY(rg1 == rg2);
    rg2.setInnerRadius(12.3f);
    QVERIFY(rg1 != rg2);
    rg1.setInnerRadius(12.3f);
    QVERIFY(rg1 == rg2);
    rg2.setOuterRadius(32.4f);
    QVERIFY(rg1 != rg2);
    rg1.setOuterRadius(rg2.outerRadius());
    QVERIFY(rg1 == rg2);
    rg2.setColorAt(0.0f, Qt::red);
    QVERIFY(rg1 != rg2);
    rg2.setStops({});
    QVERIFY(rg1 == rg2);

    QCImagePattern ip1;
    QCImagePattern ip2;
    QVERIFY(ip1 == ip2);
    ip2.setRotation(12.3f);
    QVERIFY(ip1 != ip2);
    ip1.setRotation(12.3f);
    QVERIFY(ip1 == ip2);
    QCImage img;
    ip2.setImage(img);
    // Still the same as the image is empty.
    QVERIFY(ip1 == ip2);
    ip2.setStartPosition(50, 60);
    QVERIFY(ip1 != ip2);
    ip1.setStartPosition(ip2.startPosition());
    QVERIFY(ip1 == ip2);
    ip2.setImageSize(64, 128);
    QVERIFY(ip1 != ip2);
    ip1.setImageSize(ip2.imageSize());
    QVERIFY(ip1 == ip2);

    QCGridPattern gp1;
    QCGridPattern gp2;
    QVERIFY(gp1 == gp2);
    gp2.setRotation(12.3f);
    QVERIFY(gp1 != gp2);
    gp1.setRotation(12.3f);
    QVERIFY(gp1 == gp2);
    gp2.setStartPosition(50, 60);
    QVERIFY(gp1 != gp2);
    gp1.setStartPosition(gp2.startPosition());
    QVERIFY(gp1 == gp2);
    gp2.setLineColor(QColorConstants::Yellow);
    QVERIFY(gp1 != gp2);
    gp1.setLineColor(gp2.lineColor());
    QVERIFY(gp1 == gp2);

}

void tst_QCBrush::testDataStreams()
{
    QByteArray data;
    QCGradientStops stops;
    stops << QCGradientStop {0.0f, QColorConstants::Red};
    stops << QCGradientStop {0.5f, QColorConstants::Green};
    stops << QCGradientStop {1.0f, QColorConstants::Blue};

    // QCLinearGradient
    QCLinearGradient lc1(10, 20, 30, 40);
    lc1.setStops(stops);
    QCLinearGradient lc2(10, 20, 30, 40);
    lc2.setStops(stops);
    QCOMPARE(lc1, lc2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << lc1;
    }
    QCLinearGradient lcStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> lcStreamed;
    }
    QCOMPARE(lc1, lcStreamed);

    // QCRadialGradient
    QCRadialGradient rc1(50, 100, 80, 40);
    rc1.setStops(stops);
    QCRadialGradient rc2(rc1);
    QCOMPARE(rc1, rc2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << rc1;
    }
    QCRadialGradient rcStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> rcStreamed;
    }
    QCOMPARE(rc1, rcStreamed);

    // QCConicalGradient
    QCConicalGradient cc1(100, 200, float(M_PI));
    cc1.setStops(stops);
    QCConicalGradient cc2 = cc1;
    QCOMPARE(cc1, cc2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << cc1;
    }
    QCConicalGradient ccStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> ccStreamed;
    }
    QCOMPARE(cc1, ccStreamed);

    // QCBoxGradient
    QCBoxGradient bc1(10, 20, 30, 40, 15, 5);
    bc1.setStops(stops);
    QCBoxGradient bc2 = bc1;
    QCOMPARE(bc1, bc2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << bc1;
    }
    QCConicalGradient bcStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> bcStreamed;
    }
    QCOMPARE(bc1, bcStreamed);

    // QCImagePattern
    QCImage image;
    QRectF rect(10, 20, 30, 40);
    QCImagePattern ip1(image, rect, 0.5f, QColorConstants::Red);
    QCImagePattern ip2 = ip1;
    QCOMPARE(ip1, ip2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << ip1;
    }
    QCImagePattern ipStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> ipStreamed;
    }
    QCOMPARE(ip1, ipStreamed);

    // QCBoxShadow
    QCBoxShadow bs1(10, 20, 30, 40, 15, 5, QColorConstants::Red);
    bs1.setTopLeftRadius(2);
    bs1.setTopRightRadius(3);
    bs1.setBottomLeftRadius(4);
    bs1.setBottomRightRadius(5);
    QCBoxShadow bs2 = bs1;
    QCOMPARE(bs1, bs2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << bs1;
    }
    QCBoxShadow bsStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> bsStreamed;
    }
    QCOMPARE(bs1, bsStreamed);

    // QCGridPattern
    QCGridPattern gp1(10, 20, 30, 40, QColorConstants::Red, QColorConstants::Blue);
    gp1.setFeather(2.0f);
    gp1.setRotation(0.5f);
    gp1.setLineWidth(3.0f);
    QCGridPattern gp2 = gp1;
    QCOMPARE(gp1, gp2);
    {
        QDataStream sw(&data, QIODevice::WriteOnly);
        sw << gp1;
    }
    QCGridPattern gpStreamed;
    {
        QDataStream sr(&data, QIODevice::ReadOnly);
        sr >> gpStreamed;
    }
    QCOMPARE(gp1, gpStreamed);
}

void tst_QCBrush::testDebugs()
{
    QCLinearGradient g1;
    qDebug() << g1;
    QCRadialGradient g2;
    g2.setStartColor(QColorConstants::Red);
    qDebug() << g2;
    QCConicalGradient g3;
    g3.setStartColor(QColorConstants::Red);
    g3.setEndColor(QColorConstants::Blue);
    qDebug() << g3;
    QCBoxGradient g4;
    g4.setColorAt(0.5f, QColorConstants::Green);
    qDebug() << g4;
    QCImagePattern p1;
    qDebug() << p1;
    QCBoxShadow bs1;
    qDebug() << bs1;
    QCGridPattern gp1;
    qDebug() << gp1;
}

void tst_QCBrush::testTypes()
{
    QList<QCBrush *> brushes;
    brushes << new QCBrush();
    brushes << new QCLinearGradient(10, 20, 30, 40);
    brushes << new QCRadialGradient(11, 21, 31, 41);
    brushes << new QCConicalGradient(12, 22, 23);
    brushes << new QCBoxGradient(13, 23, 33, 43, 10);
    QCImage image;
    brushes << new QCImagePattern(image, 14, 24, 34, 44);
    brushes << new QCBoxShadow(51, 52, 53, 54, 21, 22, QColorConstants::Black);
    brushes << new QCGridPattern(61, 62, 63, 64, QColorConstants::Green, QColorConstants::Yellow);

    int gradients = 0;
    int patterns = 0;
    int shadows = 0;
    for (auto *brush : brushes) {
        if (brush->type() == QCBrush::BrushType::Invalid) {
            // Base brush type
        } else if (brush->type() == QCBrush::BrushType::LinearGradient) {
            auto b = static_cast<QCLinearGradient*>(brush);
            QCOMPARE(b->startPosition().x(), 10);
            gradients++;
        } else if (brush->type() == QCBrush::BrushType::RadialGradient) {
            auto b = static_cast<QCRadialGradient*>(brush);
            QCOMPARE(b->centerPosition().x(), 11);
            gradients++;
        } else if (brush->type() == QCBrush::BrushType::ConicalGradient) {
            auto b = static_cast<QCConicalGradient*>(brush);
            QCOMPARE(b->centerPosition().x(), 12);
            gradients++;
        } else if (brush->type() == QCBrush::BrushType::BoxGradient) {
            auto b = static_cast<QCBoxGradient*>(brush);
            QCOMPARE(b->rect().x(), 13);
            gradients++;
        } else if (brush->type() == QCBrush::BrushType::ImagePattern) {
            auto b = static_cast<QCImagePattern*>(brush);
            QCOMPARE(b->startPosition().x(), 14);
            patterns++;
        } else if (brush->type() == QCBrush::BrushType::BoxShadow) {
            auto b = static_cast<QCBoxShadow*>(brush);
            QCOMPARE(b->rect().x(), 51);
            shadows++;
        } else if (brush->type() == QCBrush::BrushType::GridPattern) {
            auto b = static_cast<QCGridPattern*>(brush);
            QCOMPARE(b->startPosition().x(), 61);
            patterns++;
        }
    }
    QCOMPARE(gradients, 4);
    QCOMPARE(patterns, 2);
    QCOMPARE(shadows, 1);
}

void tst_QCBrush::testQVariantConversion()
{
    QCLinearGradient g1(10, 20, 30, 40);
    g1.setStartColor(QColorConstants::Blue);
    g1.setEndColor(QColorConstants::Red);
    QVariant vg1(g1);
    QCLinearGradient g2 = qvariant_cast<QCLinearGradient>(vg1);
    QCOMPARE(g1, g2);

    QCImage image;
    QCImagePattern i1(image, 14, 24, 34, 44);
    i1.setRotation(0.5f);
    QVariant vi1(i1);
    QCImagePattern i2 = qvariant_cast<QCImagePattern>(vi1);
    QCOMPARE(i1, i2);

    QCBoxShadow s1(10, 20, 30, 40);
    s1.setColor(QColorConstants::Blue);
    QVariant vs1(s1);
    QCBoxShadow s2 = qvariant_cast<QCBoxShadow>(vs1);
    QCOMPARE(s1, s2);
}

void tst_QCBrush::testGradientStops()
{
    // Test stop values
    QCLinearGradient g1;
    QCOMPARE(g1.stops().size(), 0);
    g1.setStartColor(QColorConstants::Blue);
    g1.setColorAt(0.0, QColorConstants::Blue);
    g1.setColorAt(0.0f, QColorConstants::Blue);
    QCOMPARE(g1.stops().size(), 1);
    g1.setEndColor(QColorConstants::Blue);
    g1.setColorAt(1.0, QColorConstants::Blue);
    g1.setColorAt(1.0f, QColorConstants::Blue);
    QCOMPARE(g1.stops().size(), 2);
    g1.setColorAt(0.5f, QColorConstants::Blue);
    g1.setColorAt(0.05f, QColorConstants::Blue);
    g1.setColorAt(0.95f, QColorConstants::Blue);
    QCOMPARE(g1.stops().size(), 5);
    g1.setColorAt(0.5f, QColorConstants::Blue);
    g1.setColorAt(0.05f, QColorConstants::Blue);
    g1.setColorAt(0.95f, QColorConstants::Blue);
    QCOMPARE(g1.stops().size(), 5);

    // Test stop order
    QCLinearGradient g2;
    g2.setColorAt(0.0f, QColorConstants::Black);
    g2.setColorAt(0.5f, QColorConstants::Red);
    g2.setColorAt(0.95f, QColorConstants::Green);
    g2.setColorAt(1.0f, QColorConstants::Black);
    QCOMPARE(g2.stops().size(), 4);
    QCOMPARE(g2.stops().at(1).color, QColorConstants::Red);
    QCOMPARE(g2.stops().at(2).color, QColorConstants::Green);
    QCLinearGradient g3;
    g3.setColorAt(1.0f, QColorConstants::Black);
    g3.setColorAt(0.95f, QColorConstants::Green);
    g3.setColorAt(0.5f, QColorConstants::Red);
    g3.setColorAt(0.0f, QColorConstants::Black);
    QCOMPARE(g3.stops().size(), 4);
    QCOMPARE(g3.stops().at(1).color, QColorConstants::Red);
    QCOMPARE(g3.stops().at(2).color, QColorConstants::Green);
    QCLinearGradient g4;
    g4.setColorAt(0.5f, QColorConstants::Red);
    g4.setColorAt(1.0f, QColorConstants::Black);
    g4.setColorAt(0.0f, QColorConstants::Black);
    g4.setColorAt(0.95f, QColorConstants::Green);
    QCOMPARE(g4.stops().size(), 4);
    QCOMPARE(g4.stops().at(1).color, QColorConstants::Red);
    QCOMPARE(g4.stops().at(2).color, QColorConstants::Green);

    // Test setting all stops
    QCLinearGradient lg;
    lg.setStops({});
    QCOMPARE(lg.stops().size(), 0);
    lg.setStops({{0.5f, QColorConstants::Black}});
    QCOMPARE(lg.stops().size(), 1);
    lg.setStops({ {0.0f, QColorConstants::Red},
                  {1.0f, QColorConstants::Green},
                  {1.0f, QColorConstants::Blue} });
    QCOMPARE(lg.stops().size(), 3);
    lg.setStops({});
    QCOMPARE(lg.stops().size(), 0);
}

QTEST_MAIN(tst_QCBrush)
#include <tst_qcbrush.moc>
