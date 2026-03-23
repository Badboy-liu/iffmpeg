#include "ffmpegdecoder.h"


typedef struct DecodeContext
{
    AVBufferRef *hw_device_ctx = nullptr;
}DecodeContext;


DecodeContext decode_context={};

static enum AVPixelFormat hw_pix_fmt;

static AVBufferRef* hw_device_ctx;

FFmpegDecoder::FFmpegDecoder()
{
    fmtCtx = avformat_alloc_context();
    pkt = av_packet_alloc();
    yuvFrame = av_frame_alloc();
    rgbFrame = av_frame_alloc();
    nv12Frame = av_frame_alloc();
}

FFmpegDecoder::~FFmpegDecoder()
{
    if(!pkt) av_packet_free(&pkt);
    if(!yuvFrame) av_frame_free(&yuvFrame);
    if(!rgbFrame) av_frame_free(&rgbFrame);
    if(!videoCodecCtx) avcodec_free_context(&videoCodecCtx);
    // if(!videoCodecCtx) avcodec_close(videoCodecCtx);
    if(!fmtCtx) avformat_close_input(&fmtCtx);
}

void FFmpegDecoder::setUrl(QString url)
{
    path = url;
}

int FFmpegDecoder::hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
    int err = 0;
    if ((err =av_hwdevice_ctx_create(&hw_device_ctx,type,nullptr,nullptr,0))<0)
    {
        qDebug()<<"Failed to create HW device.";
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    return err;
}

enum AVPixelFormat FFmpegDecoder::get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
{
    Q_UNUSED(ctx);
    const enum AVPixelFormat *p;
    for (p= pix_fmts; *p!=-1; ++p)
    {
        if (*p == hw_pix_fmt)
        {
            return *p;
        }

    }
    qDebug()<<"No pixel format found.";
    return AV_PIX_FMT_NONE;
}

bool FFmpegDecoder::open_input_file()
{
    if(path.isEmpty()) return 0;
    avformat_network_init();
    AVHWDeviceType type = av_hwdevice_find_type_by_name("cuda");
    if(avformat_open_input(&fmtCtx,path.toLocal8Bit().data(),NULL,NULL)!=0){
        printf("Cannot open input file.\n");
        return 0;
    }

    if(avformat_find_stream_info(fmtCtx,NULL)<0){
        printf("Cannot find any stream in file.\n");
        return 0;
    }

    ret = av_find_best_stream(fmtCtx,AVMEDIA_TYPE_VIDEO,-1,-1,(const AVCodec**)&videoCodec,0);

    videoStreamIndex = ret;
    AVCodecParameters *videoCodecPara = fmtCtx->streams[videoStreamIndex]->codecpar;
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
    if(!(videoCodecCtx = avcodec_alloc_context3(videoCodec))){
        printf("Cannot find valid decode codec context.\n");
        return 0;
    }
    videoStream = fmtCtx->streams[videoStreamIndex];

    if(avcodec_parameters_to_context(videoCodecCtx,videoCodecPara)<0){
        printf("Cannot initialize parameters.\n");
        return 0;
    }
    if (hw_decoder_init(videoCodecCtx,type)<0)
    {
        return 0;
    }

    if(avcodec_open2(videoCodecCtx,videoCodec,NULL)<0){
        printf("Cannot open codec.\n");
        return 0;
    }


    return true;
}

void FFmpegDecoder::run()
{
    if(!open_input_file()){
        qDebug()<<"Please open video file first.";
        return;
    }

    width = videoCodecCtx->width;
    height = videoCodecCtx->height;
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32,videoCodecCtx->width,videoCodecCtx->height,1);
    out_buffer = (uint8_t*)av_malloc(numBytes*sizeof(uchar));



    while (av_read_frame(fmtCtx, pkt) >= 0)
    {
        if (pkt->stream_index == videoStreamIndex)
        {
            if (avcodec_send_packet(videoCodecCtx, pkt) >= 0)
            {
                int ret;
                while ((ret = avcodec_receive_frame(videoCodecCtx, yuvFrame)) >= 0)
                {
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        return;
                    else if (ret < 0)
                    {
                        fprintf(stderr, "Error during decoding\n");
                        exit(1);
                    }

                  if (yuvFrame->format==videoCodecCtx->pix_fmt)
                  {
                      if (av_hwframe_transfer_data(nv12Frame,yuvFrame,0)<0)
                      {
                          continue;
                      }
                  }
                    int bytes = 0;

                    for (int i=0;i<height;i++)
                    {
                        memcpy(out_buffer+bytes,nv12Frame->data[0]+nv12Frame->linesize[0]*i,width);
                        bytes+=width;
                    }
                    int uv = height>>1;
                    for (int i=0;i<uv;i++)
                    {
                        memcpy(out_buffer+bytes,nv12Frame->data[1]+nv12Frame->linesize[1]*i,width);
                        bytes+=width;
                    }
                    emit newFrame();
                    QThread::msleep(33);
                }
            }
            av_packet_unref(pkt);
        }
    }


    qDebug()<<"All video play done";
}


