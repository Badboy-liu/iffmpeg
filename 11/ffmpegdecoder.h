#ifndef FFMPEGWIDGET_H
#define FFMPEGWIDGET_H

#include <QImage>
#include <QWidget>
#include <QPaintEvent>
#include <QThread>
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <qmutex.h>

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
const static  int bufferSize = 1280*720;

class FFmpegDecoder : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegDecoder();
    ~FFmpegDecoder();
    bool open_input_file();
    void setUrl(QString url);

    static enum AVPixelFormat get_hw_format(AVCodecContext*ctx,const enum AVPixelFormat *pix_fmts);
    static int hw_decoder_init(AVCodecContext *ctx,const enum AVHWDeviceType type);

    uchar* getFrame(){
        return out_buffer;
    }

protected:
    void run() override;

signals:
    void newFrame();

public:
    QString path;
    uchar* out_buffer;
    int ret = 0;
    int width = 0,height = 0;
    AVFormatContext *fmtCtx       =NULL;
    const AVCodec         *videoCodec   =NULL;
    AVCodecContext  *videoCodecCtx=NULL;
    AVPacket        *pkt          = NULL;
    AVFrame         *yuvFrame     = NULL;
    AVFrame         *rgbFrame     = NULL;
    AVFrame         *nv12Frame     = NULL;
    AVStream         *videoStream     = NULL;
    int videoStreamIndex = -1;
    struct SwsContext *img_ctx=NULL;
    int numBytes;
};



#endif // FFMPEGWIDGET_H
