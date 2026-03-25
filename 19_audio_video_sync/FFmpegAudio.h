//
// Created by zql on 2026/3/25.
//

#ifndef IFFMPEG_FFMPEGAUDIO_H
#define IFFMPEG_FFMPEGAUDIO_H
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

#include <Qt6/QtMultimedia/QAudioFormat>
#include <Qt6/QtMultimedia/QAudioSink>
#include <Qt6/QtCore/QElapsedTimer>
#include <Qt6/QtMultimedia/QMediaDevices>
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


class FFmpegAudio :public QThread
{
public:
    explicit FFmpegAudio(QString url,AVFormatContext* fmtCtx,DemuxThread *_demux,int aIndex,
                     std::atomic<double>* clock);
    ~FFmpegAudio();

    void setUrl(QString url);

    int aIndex = -1;
protected:
    void run();

private:
    std::atomic<double>* audio_clock;
    DemuxThread *_demux;
    const AVCodec* audioCodec = NULL;
    AVCodecContext* audioCodecCtx = NULL;
    // AVPacket* pkt = NULL;
    AVFrame* audioFrame = NULL;
    struct SwrContext* swrCtx = NULL;
    uint8_t* audio_out_buffer = nullptr;
    int out_channels;
    int out_sample_rate;
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    AVFormatContext *fmtCtx;
    int numBytes = -1;

    QAudioOutput* audioOutput;
    QIODevice* streamOut;
    QAudioSink *audioSink;
    QString _url;
};


#endif //IFFMPEG_FFMPEGAUDIO_H