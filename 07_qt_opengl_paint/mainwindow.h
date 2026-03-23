//
// Created by zql on 2026/3/2.
//


#ifndef IQT_MAINWINDOW_H
#define IQT_MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
class MainWindow:public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnPlay_clicked();

    void on_btnStop_clicked();


private:
    Ui::MainWindow *ui;
};


#endif //IQT_MAINWINDOW_H