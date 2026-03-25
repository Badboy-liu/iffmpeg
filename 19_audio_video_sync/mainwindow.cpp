//
// Created by zql on 2026/3/25.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../constant.h"

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineUrl = new QLineEdit(getPath());
    // this->player = new Player(getPath());
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_btnPlay_clicked()
{
    ui->wgtPlayer->setUrl(ui->lineUrl->text());
    ui->wgtPlayer->run();
}
void MainWindow::on_btnStop_clicked()
{
    ui->wgtPlayer->stop();

}
