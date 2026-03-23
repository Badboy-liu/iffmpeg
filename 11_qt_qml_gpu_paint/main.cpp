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
#include "../constant.h"

int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL); // ⭐⭐⭐ 必加
    qmlRegisterType<VideoItem>("VideoItem",1,0,"VideoItem");
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg){
          std::cout << msg.toStdString() << std::endl;
      });
    // 1. 获取 vcpkg 根目录
    QString vcpkgRoot = getVcpkgPath();

    if (vcpkgRoot.isEmpty()) {
        qCritical() << "无法定位 vcpkg 根目录，程序退出。请确保 'vcpkg' 已添加到系统 PATH 环境变量中。";
        return -1;
    }

    // 2. 动态构建子路径 (使用 QDir 确保跨平台分隔符正确)
    // 假设结构: <root>/installed/x64-windows/debug/...
    // 注意：如果你的 vcpkg 是 release 版本或者混合安装，可能需要去掉 "/debug"
    // 这里严格按照你之前的需求构造 debug 路径
    QString baseInstalled = QDir(vcpkgRoot).filePath("installed/x64-windows");

    // 构造具体目录
    QString binPath       = QDir(baseInstalled).filePath("debug/bin");       // DLLs
    QString pluginPath    = QDir(baseInstalled).filePath("debug/Qt6/plugins"); // Qt Plugins
    QString qmlPath       = QDir(baseInstalled).filePath("debug/Qt6/qml");     // QML Modules
    qDebug() << "Using Vcpkg Root:" << vcpkgRoot;
    qDebug() << "Bin Path:" << binPath;
    qDebug() << "Plugin Path:" << pluginPath;
    qDebug() << "QML Path:" << qmlPath;
    // ❗ 关键：DLL 依赖路径
    qputenv("PATH",QByteArray(binPath.toLocal8Bit()+";") +qgetenv("PATH"));

    QQmlApplicationEngine engine;
    engine.addImportPath(qmlPath);
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
