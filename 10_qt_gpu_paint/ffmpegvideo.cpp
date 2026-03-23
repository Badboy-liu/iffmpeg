//
// Created by zql on 2026/3/19.
//

#include "ffmpegvideo.h"




typedef struct DecodeContext
{
    AVBufferRef *hw_device_ctx = nullptr;
}DecodeContext;


DecodeContext decode_context={};

static enum AVPixelFormat hw_pix_fmt;

static AVBufferRef* hw_device_ctx;

FFmpegVideo::FFmpegVideo()
{

}
FFmpegVideo::~FFmpegVideo()
{

}
void FFmpegVideo::setPath(QString p)
{
    path = p;
}

void FFmpegVideo::ffmpeg_init_variables()
{
  avformat_network_init();
  fmtCtx =avformat_alloc_context();
    pkt =av_packet_alloc();
    yuvFrame =av_frame_alloc();
    rgbFrame =av_frame_alloc();
    nv12Frame =av_frame_alloc();

    initFlag = true;
}

void FFmpegVideo::ffmpeg_free_variables()
{
    if(!pkt) av_packet_free(&pkt);
    if(!yuvFrame) av_frame_free(&yuvFrame);
    if(!rgbFrame) av_frame_free(&rgbFrame);
    if(!nv12Frame) av_frame_free(&nv12Frame);
    if(!videoCodecCtx) avcodec_free_context(&videoCodecCtx);
    if(!fmtCtx) avformat_close_input(&fmtCtx);
}

void print_decoders()
{
    const AVCodec* codec = nullptr;
    void* i = nullptr;

    while ((codec = av_codec_iterate(&i)))
    {
        if (codec->type == AVMEDIA_TYPE_VIDEO)
        {
            qDebug() << codec->name;
        }
    }
}
bool FFmpegVideo::open_input_file()
{
    if(!initFlag)
    {
        ffmpeg_init_variables();
        qDebug()<<"ffmpeg_init_variables";
    }
    // print_decoders();
    // qDebug() << avcodec_configuration();
    enum  AVHWDeviceType type = av_hwdevice_find_type_by_name("cuda");
    if(type == AV_HWDEVICE_TYPE_NONE)
    {
        qDebug()<<"Device type h264_cuvid is not supported";

        while ((type=av_hwdevice_iterate_types(type))!=AV_HWDEVICE_TYPE_NONE)
        {
            qDebug()<<"type name:"<<av_hwdevice_get_type_name(type);
        }

        return false;
    }


    if (avformat_open_input(&fmtCtx,path.toLocal8Bit().data(),nullptr,nullptr))
    {
        qDebug()<<"avformat_open_input fail";
        return false;
    }

    if (avformat_find_stream_info(fmtCtx,nullptr) < 0)
    {
        qDebug()<<"avformat_find_stream_info fail";
        return false;
    }


    videoStreamIndex = av_find_best_stream(fmtCtx,AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (videoStreamIndex<0)
    {
        qDebug()<<"av_find_best_stream fail";
        return false;
    }

    // // 根据编码格式选择 GPU 解码器
    if (fmtCtx->streams[videoStreamIndex]->codecpar->codec_id == AV_CODEC_ID_HEVC)
    {
        videoCodec = avcodec_find_decoder_by_name("hevc_cuvid");
    }
    else if (fmtCtx->streams[videoStreamIndex]->codecpar->codec_id == AV_CODEC_ID_H264)
    {
        videoCodec = avcodec_find_decoder_by_name("h264_cuvid");
    }

    if (!videoCodec)
    {
        qDebug()<<"No CUDA decoder found, fallback to CPU";
        videoCodec = avcodec_find_decoder(fmtCtx->streams[videoStreamIndex]->codecpar->codec_id);
    }

    for (int i=0;;i++)
    {
        const AVCodecHWConfig *config= avcodec_get_hw_config(videoCodec,i);
        if (!config)
        {
            qDebug()<<"avcodec_get_hw_config fail";
            fprintf(stderr, "Decoder %s does not support device type %s.\n",
                    videoCodec->name, av_hwdevice_get_type_name(type));
            return false;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type==type)
        {
            hw_pix_fmt = config->pix_fmt;
            // qDebug()<<"avcodec_get_hw_device_ctx success";
            break;
        }
    }

    if (!(videoCodecCtx = avcodec_alloc_context3(videoCodec)))
    {
        qDebug()<<"avcodec_alloc_context3 fail";
        return false;
    }

    videoStream = fmtCtx->streams[videoStreamIndex];


    if (avcodec_parameters_to_context(videoCodecCtx,videoStream->codecpar)<0)
    {
        qDebug()<<"avcodec_parameters_to_context fail";
        return false;
    }

    videoCodecCtx->get_format = get_hw_format;

    if (hw_decoder_init(videoCodecCtx,type)<0)
    {
        qDebug()<<"hw_decoder_init fail";
        return false;
    }

    if (avcodec_open2(videoCodecCtx,videoCodec,nullptr)<0)
    {
        qDebug()<<"avcodec_open2 fail";
        return false;
    }

    img_ctx = sws_getContext(videoCodecCtx->width,videoCodecCtx->height,
        AV_PIX_FMT_NV12,videoCodecCtx->width,videoCodecCtx->height,AV_PIX_FMT_RGB32,SWS_BICUBIC,nullptr,nullptr,nullptr);

    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32,videoCodecCtx->width,videoCodecCtx->height,1);
    out_buffer = (uint8_t*)av_malloc(numBytes*sizeof(uchar));

    int res = av_image_fill_arrays(rgbFrame->data,rgbFrame->linesize,out_buffer,AV_PIX_FMT_RGB32,videoCodecCtx->width,videoCodecCtx->height,1);
    if(res<0)
    {
        qDebug()<<"av_image_fill_arrays fail";
        return false;
    }
    openFlag = true;
    return true;
}

AVPixelFormat FFmpegVideo::get_hw_format(AVCodecContext* ctx,const AVPixelFormat* pix_fmts)
{
    Q_UNUSED(ctx);
    const enum AVPixelFormat *p;

    for (p=pix_fmts;*p!=-1;p++)
    {
       if (*p == hw_pix_fmt)
       {
           return *p;
       }
    }
    qDebug()<<"hw_pix_fmt not supported";
    return AV_PIX_FMT_NONE;
}

int FFmpegVideo::hw_decoder_init(AVCodecContext* videoCodecCtx,const AVHWDeviceType type)
{
    int err = 0;
    if ((err =av_hwdevice_ctx_create(&hw_device_ctx,type,nullptr,nullptr,0)) < 0)
    {
        qDebug()<<"av_hwdevice_ctx_create fail";
        return err;
    }
    videoCodecCtx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    return err;
}

void FFmpegVideo::stopThread()
{
    stopFlag = true;
}

void FFmpegVideo::run()
{
    if (!openFlag)
    {
        open_input_file();
    }

    while (av_read_frame(fmtCtx,pkt)>=0)
    {
        if (stopFlag)
        {
            av_packet_unref(pkt);
            break;
        }
        if (pkt->stream_index != videoStreamIndex)
        {
            av_packet_unref(pkt);
            continue;
        }

        if (avcodec_send_packet(videoCodecCtx,pkt) < 0)
        {
            av_packet_unref(pkt);
            continue;
        }

        int ret;
        while ((ret=avcodec_receive_frame(videoCodecCtx,yuvFrame)) >= 0)
        {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
            {
                return;
                av_packet_unref(pkt);
            }else if (ret<0)
            {
                qDebug()<<"avcodec_receive_frame fail";
                exit(1);
            }
            if (yuvFrame->format==videoCodecCtx->pix_fmt)
            {
                if ((ret=av_hwframe_transfer_data(nv12Frame,yuvFrame,0))<0)
                {
                    continue;
                }
            }

            sws_scale(img_ctx,(const uint8_t* const*)nv12Frame->data,
                (const int*)nv12Frame->linesize,
                0,nv12Frame->height,rgbFrame->data,rgbFrame->linesize);

            QImage img(out_buffer,videoCodecCtx->width,videoCodecCtx->height,QImage::Format_RGB32);
            emit sendQImage(img);
            QThread::msleep(30);

        }
        av_packet_unref(pkt);

    }
    qDebug()<<"thread stop now";
}

FFmpegWidget::FFmpegWidget(QWidget* parent):QWidget(parent)
{
    ffmepg = new FFmpegVideo();
    connect(ffmepg,SIGNAL(sendQImage(QImage)),this,SLOT(receiveQImage(QImage)));
    connect(ffmepg,&FFmpegVideo::finished,ffmepg,&FFmpegVideo::deleteLater);
}

FFmpegWidget::~FFmpegWidget()
{
    qDebug()<<"FFmpegWidget destructor";
    if (ffmepg->isRunning())
    {
        stop();
    }
}

void FFmpegWidget::play(QString path)
{
    ffmepg->setPath(path);
    ffmepg->start();
}

void FFmpegWidget::stop()
{
    if (ffmepg->isRunning())
    {
        ffmepg->requestInterruption();
        ffmepg->quit();
        ffmepg->wait(100);
    }
    ffmepg->ffmpeg_free_variables();
}

void FFmpegWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(0,0,img);
}

void FFmpegWidget::receiveQImage(const QImage& image)
{
    img =image.scaled(this->size());
    update();
}