#ifndef FFMPEGWIDGET_H
#define FFMPEGWIDGET_H

#include <QImage>
#include <QWidget>
#include <QPaintEvent>
#include <QThread>
#include <QPainter>
#include <QDebug>

#include <string>

extern "C"{
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
}

using namespace std;

class FFmpegDecoder : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegDecoder();
    ~FFmpegDecoder();

    void setUrl(QString url);

    bool open_input_file();
    int width();
    int height();
    uchar* getFrame(){
        return out_buffer;
    }
protected:
    void run();

signals:
    void sigFirst(uchar* p,int w,int h);
    void newFrame();

private:
    AVFormatContext *fmtCtx       =NULL;
    const AVCodec         *videoCodec   =NULL;
    AVCodecContext  *videoCodecCtx=NULL;
    AVPacket        *pkt          = NULL;
    AVFrame         *yuvFrame     = NULL;
    AVFrame         *rgbFrame     = NULL;

    struct SwsContext *img_ctx=NULL;

    unsigned char *out_buffer=nullptr;

    int videoStreamIndex =-1;
    int numBytes = -1;

    QString _url;
    bool isFirst = true;

    int w,h;
};



#endif // FFMPEGWIDGET_H
