//
// Created by zql on 2026/3/25.
//

#ifndef IFFMPEG_PLAYER_H
#define IFFMPEG_PLAYER_H
#include <Qt6/QtCore/qcompilerdetection.h>

#include "player.h"
#include <Qt6/QtCore/qlogging.h>
#include <Qt6/QtGui/QImage>
#include <Qt6/QtCore/qtmetamacros.h>
#include <Qt6/QtCore/QThread>
#include <Qt6/QtCore/QDebug>
#include <Qt6/QtMultimedia/QAudioOutput>
#include <Qt6/QtCore/QIODevice>
#include <Qt6/QtCore/qobjectdefs.h>
#include <Qt6/QtCore/QString>
#include <Qt6/QtCore/qtclasshelpermacros.h>
#include <Qt6/QtCore/QByteArray>

#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>

#include "demuxthread.h"
#include "FFmpegAudio.h"
#include "FFmpegVideo.h"
#include "../constant.h"
extern "C"{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
}

class Player : public QThread {
    Q_OBJECT
public:
    Player(const QString& url);
    ~Player();
    void setUrl(const QString& url);
    void run() override;

    signals:
        void sendQImage(const QImage& img);
private:
    QString url;

    AVFormatContext *fmtCtx = nullptr;
    FFmpegAudio *_audio;
    FFmpegVideo *_video;
    DemuxThread *_demux;
    std::atomic<double> audio_clock{0};
};


#endif //IFFMPEG_PLAYER_H