//
// Created by zql on 2026/3/25.
//

#ifndef IFFMPEG_MAINWINDOW_H
#define IFFMPEG_MAINWINDOW_H
#include "../19_audio_video_sync/mainwindow.h"
#include <QMainWindow>

#include "player.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow:public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private
    slots:
    void on_btnPlay_clicked();
    void on_btnStop_clicked();


private:
    Ui::MainWindow *ui;
};


#endif //IFFMPEG_MAINWINDOW_H