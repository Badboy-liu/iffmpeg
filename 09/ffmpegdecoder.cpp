#include "ffmpegdecoder.h"

FFmpegDecoder::FFmpegDecoder()
{
    fmtCtx = avformat_alloc_context();
    pkt = av_packet_alloc();
    yuvFrame = av_frame_alloc();
    rgbFrame = av_frame_alloc();
    frameBuffer.setCapacity(30);
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

    emit videoInfoReady(w,h);
    // numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,w,h,1);
    // out_buffer = (unsigned char *)av_malloc(numBytes*sizeof(uchar));
    qDebug() << "pix_fmt =" << videoCodecCtx->pix_fmt;
    // SwsContext* sws = sws_getContext(
    // videoCodecCtx->width,
    // videoCodecCtx->height,
    // videoCodecCtx->pix_fmt,
    // videoCodecCtx->width,
    // videoCodecCtx->height,
    // AV_PIX_FMT_YUV420P,
    // SWS_BILINEAR,
    // nullptr, nullptr, nullptr);
    AVFrame* frame420 = av_frame_alloc();

    // av_image_alloc(frame420->data, frame420->linesize,
                   // w, h, AV_PIX_FMT_YUV420P, 1);

    static int frameIndex = 0;
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

                    // Y plane
                    m_yuvData.Y.resize(yuvFrame->linesize[0] * yuvFrame->height);
                    memcpy(m_yuvData.Y.data(), yuvFrame->data[0], m_yuvData.Y.size());

                    // U plane
                    m_yuvData.U.resize(yuvFrame->linesize[1] * (yuvFrame->height / 2));
                    memcpy(m_yuvData.U.data(), yuvFrame->data[1], m_yuvData.U.size());

                    // V plane
                    m_yuvData.V.resize(yuvFrame->linesize[2] * (yuvFrame->height / 2));
                    memcpy(m_yuvData.V.data(), yuvFrame->data[2], m_yuvData.V.size());

                    // 保存 linesize
                    m_yuvData.yLineSize = yuvFrame->linesize[0];
                    m_yuvData.uLineSize = yuvFrame->linesize[1];
                    m_yuvData.vLineSize = yuvFrame->linesize[2];
                    m_yuvData.height = yuvFrame->height;
                    m_yuvData.width = yuvFrame->width;

                    qDebug()<<"m_yuvData.yLineSize"<<m_yuvData.yLineSize;
                    qDebug()<<"m_yuvData.uLineSize"<<m_yuvData.uLineSize;
                    qDebug()<<"m_yuvData.vLineSize"<<m_yuvData.vLineSize;


                    QMutexLocker locker(&m_mutex);
                    frameBuffer.append(m_yuvData);
                    // qDebug() << "newFrame";
                    // QString fileName = QString("frame_%1.yuv").arg(frameIndex++);
                    // QFile file(fileName);
                    // if (file.open(QIODevice::WriteOnly)) {
                    //     file.write(m_yuvData.Y);
                    //     file.write(m_yuvData.U);
                    //     file.write(m_yuvData.V);
                    //     file.close();
                    // }
                    QThread::msleep(33);
                }
            }
            av_packet_unref(pkt);
        }
    }


    qDebug()<<"All video play done";
}


