//
// Created by zql on 2026/3/19.
//

#ifndef IFFMPEG_MAINWINDOW_H
#define IFFMPEG_MAINWINDOW_H
#include <QMainWindow>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* q_widget=nullptr);
    ~MainWindow();
public slots:
    void on_btnPs_clicked();

private:
    Ui::MainWindow *ui;

    bool isPlay = false;
};


#endif //IFFMPEG_MAINWINDOW_H
