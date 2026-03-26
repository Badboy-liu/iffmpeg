//
// Created by loukas on 2026/3/26.
//

#ifndef IFFMPEG_MAINWINDOWS_H
#define IFFMPEG_MAINWINDOWS_H
#include <QMainWindow>
#include <qtconfigmacros.h>
#include <QMessageBox>
QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private
    slots:
    void on_btnPlay_clicked();

    void on_btnStop_clicked();
    void on_spinBoxContrast_valueChanged(int c);
    void on_spinBoxLightness_valueChanged(int c);

private:
    Ui::MainWindow *ui;
    int contrast = 5,brightness = 5;

};


#endif //IFFMPEG_MAINWINDOWS_H
