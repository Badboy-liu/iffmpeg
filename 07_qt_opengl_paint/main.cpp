#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include <windows.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QObject::tr("Player"));
    w.show();
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg){
        std::cout << msg.toStdString() << std::endl;
    });

    qDebug() << "qDebug test";
    qInfo()  << "qInfo test";
    qDebug() << "Hello World";
    return QApplication::exec();
}
