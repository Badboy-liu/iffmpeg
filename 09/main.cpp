//
// Created by zql on 2026/3/17.
//

#include<iostream>
#include <QQuickWindow>
#include <qsgrendererinterface.h>
#include <Qt6/QtGui/qguiapplication.h>
#include <Qt6/QtQml/qqmlapplicationengine.h>
#include <Qt6/QtQml/qqml.h>

#include "VideoItem.h"

int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL); // ⭐⭐⭐ 必加
    QCoreApplication::addLibraryPath("E:/vcpkg/installed/x64-windows/debug/Qt6/plugins");
    qmlRegisterType<VideoItem>("VideoItem",1,0,"VideoItem");
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg){
          std::cout << msg.toStdString() << std::endl;
      });
    // ❗ 关键：DLL 依赖路径
    qputenv("PATH",
        QByteArray("E:/vcpkg/installed/x64-windows/debug/bin;") +
        qgetenv("PATH"));
    QQmlApplicationEngine engine;
    engine.addImportPath("E:/vcpkg/installed/x64-windows/debug/Qt6/qml");
    engine.load(QUrl("qrc:/main.qml"));

    QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreated,
    [](QObject *obj, const QUrl &objUrl) {
        if (!obj) {
            qDebug() << "QML load failed:" << objUrl;
        }
    }
);

    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    return app.exec();
}
