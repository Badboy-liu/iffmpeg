//
// Created by zql on 2026/3/19.
//

#ifndef IFFMPEG_FFMPEGVIDEO_H
#define IFFMPEG_FFMPEGVIDEO_H
#include <QThread>
#include <QWidget>
#include <QPainter>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/buffer.h>
#include <libavcodec/avcodec.h>

#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}



class FFmpegVideo : public QThread
{
    Q_OBJECT

public:
    explicit FFmpegVideo();
    ~FFmpegVideo();
    FFmpegVideo(const FFmpegVideo&) = delete;
    FFmpegVideo& operator=(const FFmpegVideo&) = delete;
    void setPath(QString path);
    void ffmpeg_init_variables();
    void ffmpeg_free_variables();
    bool open_input_file();
    static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);

    static int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type);

    void stopThread();

protected:
    void run() override;

signals:
    void sendQImage(const QImage& image);

private:
    AVFormatContext* fmtCtx = nullptr;
    const AVCodec* videoCodec = nullptr;
    AVCodecContext* videoCodecCtx = nullptr;
    AVPacket* pkt;
    AVFrame* yuvFrame;
    AVFrame* rgbFrame;
    AVFrame* nv12Frame;
    AVStream* videoStream;

    uchar *out_buffer;
    struct SwsContext *img_ctx;

    QString path;

    int videoStreamIndex = -1;
    int numBytes = -1;
    int ret = 0;

    bool initFlag = false,openFlag = false,stopFlag=false;
};


class FFmpegWidget:public QWidget
{
    Q_OBJECT
    public:
    explicit FFmpegWidget(QWidget *parent=nullptr);
    ~FFmpegWidget();
    void play(QString path);
    void stop();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void receiveQImage(const QImage& image);

private:
    FFmpegVideo* ffmepg;
    QImage img;
};


#endif //IFFMPEG_FFMPEGVIDEO_H
