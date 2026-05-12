// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BENCHWIDGET_H
#define BENCHWIDGET_H

#include <QCanvasPainterWidget>
#include <QElapsedTimer>
#include <QString>

class BenchWidget : public QCanvasPainterWidget
{
public:
    explicit BenchWidget(Api api);
    void paint(QCanvasPainter *p) override;

private:
    int m_frame = 0;
    int m_fpsFrameCount = 0;
    int m_fps = 0;
    QString m_apiName;
    QElapsedTimer m_fpsTimer;
};

#endif
