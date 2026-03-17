#include "ffmpegdecoder.h"

FFmpegDecoder::FFmpegDecoder()
{
    fmtCtx = avformat_alloc_context();
    pkt = av_packet_alloc();
    yuvFrame = av_frame_alloc();
    rgbFrame = av_frame_alloc();
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
    _url = url;
}

int FFmpegDecoder::width()
{
    return w;
}

int FFmpegDecoder::height()
{
    return h;
}

bool FFmpegDecoder::open_input_file()
{
    if(_url.isEmpty()) return 0;

    if(avformat_open_input(&fmtCtx,_url.toLocal8Bit().data(),NULL,NULL)<0){
        printf("Cannot open input file.\n");
        return 0;
    }

    if(avformat_find_stream_info(fmtCtx,NULL)<0){
        printf("Cannot find any stream in file.\n");
        return 0;
    }

    int streamCnt=fmtCtx->nb_streams;
    for(int i=0;i<streamCnt;i++){
        if(fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoStreamIndex = i;
            continue;
        }
    }

    if(videoStreamIndex==-1){
        printf("Cannot find video stream in file.\n");
        return 0;
    }

    AVCodecParameters *videoCodecPara = fmtCtx->streams[videoStreamIndex]->codecpar;

    if(!(videoCodec = avcodec_find_decoder(videoCodecPara->codec_id))){
        printf("Cannot find valid decode codec.\n");
        return 0;
    }

    if(!(videoCodecCtx = avcodec_alloc_context3(videoCodec))){
        printf("Cannot find valid decode codec context.\n");
        return 0;
    }

    if(avcodec_parameters_to_context(videoCodecCtx,videoCodecPara)<0){
        printf("Cannot initialize parameters.\n");
        return 0;
    }
    if(avcodec_open2(videoCodecCtx,videoCodec,NULL)<0){
        printf("Cannot open codec.\n");
        return 0;
    }
    // img_ctx = sws_getContext(videoCodecCtx->width,
    //                          videoCodecCtx->height,
    //                          videoCodecCtx->pix_fmt,
    //                          videoCodecCtx->width,
    //                          videoCodecCtx->height,
    //                          AV_PIX_FMT_RGB32,
    //                          SWS_BICUBIC,NULL,NULL,NULL);
    //
    // numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32,videoCodecCtx->width,videoCodecCtx->height,1);
    // out_buffer = (unsigned char *)av_malloc(numBytes*sizeof(unsigned char));
    //
    // int res = av_image_fill_arrays(
    //             rgbFrame->data,rgbFrame->linesize,
    //             out_buffer,AV_PIX_FMT_RGB32,
    //             videoCodecCtx->width,videoCodecCtx->height,1);
    // if(res<0){
    //     qDebug()<<"Fill arrays failed.";
    //     return 0;
    // }

    return true;
}

void FFmpegDecoder::run()
{
    if(!open_input_file()){
        qDebug()<<"Please open video file first.";
        return;
    }
    w = videoCodecCtx->width;
    h = videoCodecCtx->height;
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,w,h,1);
    out_buffer = (unsigned char *)av_malloc(numBytes*sizeof(uchar));
    qDebug() << "pix_fmt =" << videoCodecCtx->pix_fmt;
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
                    if (isFirst)
                    {
                        FILE *fp = fopen("first.yuv","wb");
                        fwrite(out_buffer,1,numBytes,fp);
                        fclose(fp);
                        isFirst = false;
                        emit sigFirst(out_buffer, w, h);
                    }

                    int bytes = 0;
                    for (int i = 0; i < h; i++)
                    {
                        memcpy(out_buffer + bytes, yuvFrame->data[0] + yuvFrame->linesize[0] * i, w);
                        bytes += w;
                    }
                    int u = h >> 1;
                    for (int i = 0; i < u; i++)
                    {
                        memcpy(out_buffer + bytes, yuvFrame->data[1] + yuvFrame->linesize[1] * i, w / 2);
                        bytes += w / 2;
                    }
                    for (int i = 0; i < u; i++)
                    {
                        memcpy(out_buffer + bytes, yuvFrame->data[2] + yuvFrame->linesize[2] * i, w / 2);
                        bytes += w / 2;
                    }

                    qDebug() << "newFrame";;
                    emit newFrame();
                    QThread::msleep(24);
                }
            }
            av_packet_unref(pkt);
        }
    }


    qDebug()<<"All video play done";
}


