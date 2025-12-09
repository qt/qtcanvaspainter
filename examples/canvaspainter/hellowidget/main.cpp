// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include "canvaswidget.h"

class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    QMdiArea *mdi;
};

MainWindow::MainWindow()
{
    mdi = new QMdiArea;
    setCentralWidget(mdi);

    CanvasWidget *canvasWidget = new CanvasWidget;
    mdi->addSubWindow(canvasWidget)->resize(500, 500);
    canvasWidget->show();

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New widget"), this, [this] {
        CanvasWidget *canvasWidget = new CanvasWidget;
        mdi->addSubWindow(canvasWidget)->resize(500, 500);
        canvasWidget->show();
    });
    fileMenu->addAction(tr("E&xit"), qApp, &QCoreApplication::quit);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.resize(1280, 720);
    mainWindow.show();

    return app.exec();
}
