#include "mainwindow.h"
#include "../constant.h"
#include "QtUiTools/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineUrl->setText(getPath());
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnPlay_clicked()
{
    ui->wgtPlayer->setUrl(ui->lineUrl->text());
    ui->wgtPlayer->play();
}

void MainWindow::on_btnStop_clicked()
{
    ui->wgtPlayer->stop();
}
