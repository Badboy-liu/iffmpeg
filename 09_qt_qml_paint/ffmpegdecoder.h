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
struct YUVData
{
    YUVData()
    {
        Y.reserve(bufferSize);
        U.reserve(bufferSize);
        V.reserve(bufferSize);
    }
    QByteArray Y;
    QByteArray U;
    QByteArray V;
    int yLineSize;
    int uLineSize;
    int vLineSize;
    int height;
    int width;
};
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
    // uchar* getFrame(){
    //     return out_buffer;
    // }
    YUVData getFrame()
    {
        QMutexLocker locker(&m_mutex);

        return frameBuffer.isEmpty() ? YUVData() : frameBuffer.takeLast();
    }
protected:
    void run();

signals:
    void sigFirst(uchar* p,int w,int h);
    void newFrame();
    void videoInfoReady(int w,int h);

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
    QMutex m_mutex;
    YUVData m_yuvData;
    QContiguousCache<YUVData> frameBuffer;
    int w,h;
};



#endif // FFMPEGWIDGET_H
