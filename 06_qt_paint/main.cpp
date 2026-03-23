#include <QApplication>
#include <QDebug>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QObject::tr("Player"));
    w.show();
    qDebug() << "Hello World";
    return QApplication::exec();
}
