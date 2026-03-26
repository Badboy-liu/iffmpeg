//
// Created by loukas on 2026/3/26.
//

#ifndef IFFMPEG_FFMPEGWIDGET_H
#define IFFMPEG_FFMPEGWIDGET_H
#include <QThread>
#include <QWidget>
#include <QPainter>

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

#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include <libswscale/swscale.h>

#include <libavformat/avformat.h>
}
class FFmpegVideo:public QThread {
    Q_OBJECT
    public:
    FFmpegVideo();
    ~FFmpegVideo();

    void updateEQ(float contrast, float brightness);

    void init_variables();
    void free_variables();

    void setUrl(QString url);
    bool open_input_file();

    bool initFilter();

    void setCL(int c,int l);
protected:
    void run() override;

signals:
    void sendImg(QImage);

private:
    AVFormatContext *formatCtx = nullptr;
    const AVCodec *codec = nullptr;
    AVCodecContext *codecCtx = nullptr;
    AVPacket* packet = nullptr;
    AVFrame* yuvFrame = nullptr;
    AVFrame* rgbFrame = nullptr;
    AVFilterContext *eqCtx = nullptr;
    AVFilterContext* bufSinkCtx = nullptr;
    AVFilterContext* bufSrcCtx = nullptr;
    AVFilterGraph* filterGraph = nullptr;


    QString filterDescr="eq=contrast=1:brightness=0";

    QString _url;

    struct SwsContext *img_ctx = nullptr;
    unsigned char*out_buf = nullptr;
    int videoWidth,videoHeight;
    int videoStreamIndex = -1;
    int numBytes = -1;

    bool runFlag = true;

    int ret = -1;

};
class FFmpegWidget : public QWidget {
    Q_OBJECT

public:
    explicit FFmpegWidget(QWidget *parent = 0);

    ~FFmpegWidget();

    void play(QString url);

    void stop();

    void setFilterDescr(int c, int b);

protected:
    void paintEvent(QPaintEvent *);


private slots:
    void receiveQImage(const QImage &image);

private:
    FFmpegVideo *ffmpeg;
    QImage img;

};


#endif //IFFMPEG_FFMPEGWIDGET_H
