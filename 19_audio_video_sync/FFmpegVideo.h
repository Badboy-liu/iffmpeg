//
// Created by zql on 2026/3/25.
//

#ifndef IFFMPEG_FFMPEGVIDEO_H
#define IFFMPEG_FFMPEGVIDEO_H
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


class FFmpegVideo :public QThread
{
    Q_OBJECT
public:
    explicit FFmpegVideo(QString url,AVFormatContext* fmtCtx,DemuxThread *_demux,int vIndex,
                     std::atomic<double>* clock);
    ~FFmpegVideo();

    void setUrl(QString url);

    bool open_input_file();
    int vIndex =-1;

protected:
    void run();
    signals:
        void sendQImage(QImage);

private:
    const AVCodec         *videoCodec   =NULL;
    AVCodecContext  *videoCodecCtx=NULL;
    SwsContext * swsCtx = nullptr;
    // AVPacket        *pkt          = NULL;
    AVFrame         *yuvFrame     = NULL;
    AVFrame         *rgbFrame     = NULL;
    DemuxThread *_demux;
    struct SwsContext *img_ctx=NULL;
    AVFormatContext *fmtCtx;
    unsigned char *out_buffer=nullptr;
    std::atomic<double>* audio_clock;

    int numBytes = -1;

    QString _url;

};

#endif //IFFMPEG_FFMPEGVIDEO_H