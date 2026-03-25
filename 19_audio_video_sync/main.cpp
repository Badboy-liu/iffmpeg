//
// Created by zql on 2026/3/25.
//
#include<iostream>
using namespace std;
#include <QApplication>
#include "../19_audio_video_sync/mainwindow.h"



int main(int argc,char* arg[]){

    QApplication app(argc,const_cast<char**>(arg));
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg)
     {
         std::cout << msg.toStdString() << std::endl;
     });
    MainWindow w;
    w.setWindowTitle(QObject::tr("Player"));
    w.show();
    return app.exec();
}
