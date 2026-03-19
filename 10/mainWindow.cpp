//
// Created by zql on 2026/3/19.
//

#include "MainWindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* q_widget):QMainWindow(q_widget),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_btnPs_clicked()
{
    QString url = ui->lineUrl->text().trimmed();
    if (url.isEmpty())
    {
        QMessageBox::information(this, tr("Information"),"Please input url",QMessageBox::Ok);
        return;
    }
    ui->widget->play(url);
}