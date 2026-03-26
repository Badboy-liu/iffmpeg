//
// Created by loukas on 2026/3/26.
//
#include <QApplication>

#include "../20_add_filter/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.setWindowTitle("QMainWindow");
    mainWindow.show();
    return app.exec();
}
