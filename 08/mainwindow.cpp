#include "mainwindow.h"
#include "QtUiTools/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnPlay_clicked()
{
    ui->openGLWidget->setUrl(ui->lineUrl->text());
    ui->openGLWidget->startVideo();
}

void MainWindow::on_btnStop_clicked()
{
    ui->openGLWidget->startVideo();
}
