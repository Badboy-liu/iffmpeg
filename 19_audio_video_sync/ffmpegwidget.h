//
// Created by zql on 2026/3/25.
//

#ifndef IFFMPEG_FFMPEGWIDGET_H
#define IFFMPEG_FFMPEGWIDGET_H
#include <QImage>
#include <QWidget>
#include <QPaintEvent>
#include <QThread>
#include <QPainter>
#include <QDebug>
#include <QTime>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QTimer>
#include <QTest>
#include <QAudioDevice>
#include <QMediaDevices>
#include <string>
#include <QAudioSink>


#include "demuxthread.h"
#include "player.h"

extern "C" {
#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libavutil/mem.h>

#include <libswscale/swscale.h>

#include <libavformat/avformat.h>
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
}

#define MAX_AUDIO_FRAME_SIZE 192000

using namespace std;





class FFmpegWidget : public QWidget
{
    Q_OBJECT
    public:
    explicit FFmpegWidget(QWidget *parent = nullptr);
    ~FFmpegWidget();

    void setUrl(QString url);
    void run();
    void stop();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void receiveQImage(const QImage &rImg);
private:

    QImage image;
    Player* player;

};






#endif //IFFMPEG_FFMPEGWIDGET_H
