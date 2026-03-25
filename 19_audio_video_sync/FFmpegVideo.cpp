//
// Created by zql on 2026/3/25.
//

#include "FFmpegVideo.h"

FFmpegVideo::FFmpegVideo(QString url, AVFormatContext* fmtCtx,DemuxThread *_demux,int vIndex,
                     std::atomic<double>* clock)
{
    this->setUrl(url);
    this->fmtCtx = fmtCtx;
    this->_demux = _demux;
    this->vIndex = vIndex;
    this->audio_clock = clock;
}
FFmpegVideo::~FFmpegVideo(){
    if(videoCodecCtx){
        avcodec_free_context(&videoCodecCtx);
    }

    if(yuvFrame){
        av_frame_free(&yuvFrame);
    }

    if(rgbFrame){
        av_frame_free(&rgbFrame);
    }

    if(swsCtx){
        sws_freeContext(swsCtx);
    }

    if(out_buffer){
        av_free(out_buffer);
    }
}
void FFmpegVideo::setUrl(QString url)
{
    _url = url;
}

void FFmpegVideo::run()
{
    // ===== 视频 =====
    auto vpara = fmtCtx->streams[vIndex]->codecpar;
    auto vcodec = avcodec_find_decoder(vpara->codec_id);
    this->videoCodecCtx = avcodec_alloc_context3(vcodec);
    avcodec_parameters_to_context(this->videoCodecCtx,vpara);
    avcodec_open2(this->videoCodecCtx,vcodec,nullptr);

    swsCtx = sws_getContext(
        videoCodecCtx->width,videoCodecCtx->height,videoCodecCtx->pix_fmt,
        videoCodecCtx->width,videoCodecCtx->height,AV_PIX_FMT_RGB32,
        SWS_BILINEAR,nullptr,nullptr,nullptr
    );

    // AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* rgb = av_frame_alloc();

    int numBytes = av_image_get_buffer_size(
        AV_PIX_FMT_RGB32,
        videoCodecCtx->width,
        videoCodecCtx->height,
        1
    );

    uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
    av_image_fill_arrays(rgb->data,rgb->linesize,
                         buffer,AV_PIX_FMT_RGB32,
                         videoCodecCtx->width,videoCodecCtx->height,1);


    qDebug() << QMediaDevices::audioOutputs()[0].description();
    while (!isInterruptionRequested())
    {
        bool ok;
        AVPacket* pkt = _demux->videoQ->pop(ok);
        if (!ok) {
            break; // 线程退出
        }
        if (pkt)
        {

            avcodec_send_packet(videoCodecCtx,pkt);

            while(avcodec_receive_frame(videoCodecCtx,frame)==0){


                int64_t best_pts = frame->best_effort_timestamp;
                double pts = best_pts * av_q2d(fmtCtx->streams[vIndex]->time_base);

                double audio = audio_clock->load();
                double diff = pts - audio;

                // ⭐ 丢帧
                if (diff < -0.05) {
                    av_frame_unref(frame);
                    continue;
                }

                // ⭐ 等待
                if (diff > 0.01) {
                    int delay = diff * 1000;
                    if (delay > 50) delay = 50;
                    QThread::msleep(delay);
                }



                sws_scale(swsCtx,
                          frame->data,frame->linesize,
                          0,videoCodecCtx->height,
                          rgb->data,rgb->linesize);

                QImage img(buffer,
                           videoCodecCtx->width,
                           videoCodecCtx->height,
                           QImage::Format_RGB32);

                emit sendQImage(img);
                qDebug()<<"sendQImage";

            }
            av_packet_unref(pkt);

        }
    }




}