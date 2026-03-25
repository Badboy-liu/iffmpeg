//
// Created by zql on 2026/3/25.
//

#include "ffmpegwidget.h"



FFmpegWidget::FFmpegWidget(QWidget *parent) : QWidget(parent)
{

    player = new Player(getPath());
    connect(player,SIGNAL(sendQImage(QImage)),this,SLOT(receiveQImage(QImage)));
    connect(player,&Player::finished,player,&Player::deleteLater);


}
FFmpegWidget::~FFmpegWidget()
{
    if(player){
        player->requestInterruption();
        player->quit();
        player->wait();
        delete player;
        player = nullptr;
    }
}
void FFmpegWidget::setUrl(QString url)
{
    player->setUrl(url);
}
void FFmpegWidget::run()
{
    player->start();
}
void FFmpegWidget::stop()
{
    if(player  && player->isRunning()){
        player->requestInterruption();
        player->quit();
        player->wait(100);
    }

}
void FFmpegWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0,0,image);
}

void FFmpegWidget::receiveQImage(const QImage &rImg)
{
    image = rImg.scaled(this->size());
    update();
}
