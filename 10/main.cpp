//
// Created by zql on 2026/3/18.
//

#include<iostream>

#include "../10/mainwindow.h"
using namespace std;
#include <QApplication>


int main(int argc, char* arg[])
{
    QApplication app(argc, arg);
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg)
    {
        std::cout << msg.toStdString() << std::endl;
    });
    MainWindow w;
    w.show();


    return app.exec();
}
