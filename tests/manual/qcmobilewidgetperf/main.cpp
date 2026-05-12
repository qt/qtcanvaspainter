// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "benchwidget.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QList>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QtGlobal>

namespace {

struct ApiEntry
{
    QRhiWidget::Api api;
    const char *label;
};

QList<ApiEntry> supportedApis()
{
    QList<ApiEntry> apis;
#if QT_CONFIG(opengl)
    apis.append({ QRhiWidget::Api::OpenGL, "OpenGL" });
#endif
#if QT_CONFIG(vulkan)
    apis.append({ QRhiWidget::Api::Vulkan, "Vulkan" });
#endif
#ifdef Q_OS_WIN
    apis.append({ QRhiWidget::Api::Direct3D11, "Direct3D 11" });
    apis.append({ QRhiWidget::Api::Direct3D12, "Direct3D 12" });
#endif
#ifdef Q_OS_DARWIN
    apis.append({ QRhiWidget::Api::Metal, "Metal" });
#endif
    apis.append({ QRhiWidget::Api::Null, "Null" });
    return apis;
}

QRhiWidget::Api defaultApi()
{
#ifdef Q_OS_DARWIN
    return QRhiWidget::Api::Metal;
#elif defined(Q_OS_WIN)
    return QRhiWidget::Api::Direct3D11;
#else
    return QRhiWidget::Api::OpenGL;
#endif
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qputenv("QSG_INFO", "1");

    const QList<ApiEntry> apis = supportedApis();
    const QRhiWidget::Api preselect = defaultApi();

    QRhiWidget::Api chosen = preselect;
    {
        // The dialog is scoped so it is destroyed before BenchWidget is created.
        // On Android, QAndroidPlatformWindow picks SurfaceView (vsync-throttled
        // via SurfaceFlinger) only when the BenchWidget's platform window is the
        // sole window on the screen at initialize() time. If the dialog's window
        // is still around, BenchWidget falls back to TextureView, whose
        // SurfaceTexture-backed EGL surface does not block on display vsync, and
        // the measured fps no longer reflects the panel refresh rate.
        QDialog dialog;
        dialog.setWindowTitle(QStringLiteral("cpwidgettest - select graphics API"));
        auto *layout = new QVBoxLayout(&dialog);
        layout->addWidget(new QLabel(QStringLiteral("Select the QRhi backend to use:")));

        auto *group = new QButtonGroup(&dialog);
        for (int i = 0; i < apis.size(); ++i) {
            auto *button = new QRadioButton(QString::fromLatin1(apis[i].label));
            layout->addWidget(button);
            group->addButton(button, i);
            if (apis[i].api == preselect)
                button->setChecked(true);
        }
        if (group->checkedId() < 0 && !apis.isEmpty())
            group->button(0)->setChecked(true);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted)
            return 0;

        const int idx = group->checkedId();
        chosen = idx >= 0 ? apis[idx].api : preselect;
    }

    BenchWidget widget(chosen);
    widget.resize(800, 600);
    widget.setWindowTitle("cpwidgettest");
    widget.show();

    return QCoreApplication::exec();
}
