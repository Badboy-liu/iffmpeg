//
// Created by zql on 2026/3/24.
//

#include<iostream>
#include <QThread>
#include <Qt6/QtCore/QCoreApplication.h>
#include <Qt6/QtCore/qstring.h>
#include <Qt6/QtCore/qDebug.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

}
#include "Qt6/QtMultimedia/QAudioOutput"
#include "Qt6/QtMultimedia/QAudioFormat"
#include "Qt6/QtMultimedia/QMediaDevices"
#include "Qt6/QtMultimedia/QAudioDevice"
#include "Qt6/QtMultimedia/QAudioSink"
using namespace std;


#define MAX_AUDIO_FRAME_SIZE 192000

int main(int argc,char* arg[]){
    QString url  = "../resources/input/input.mp4";
    QCoreApplication a(argc, arg);
    QAudioOutput *audioOutput;

    QAudioFormat audioFormat;
    audioFormat.setSampleRate(44100);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Int16);


    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (!device.isFormatSupported(audioFormat))
    {
        audioFormat = device.preferredFormat();
    }

    QAudioSink *audioSink = new QAudioSink(device,audioFormat);
    audioSink->setVolume(1.0);

    QIODevice *streamOut = audioSink->start();

    AVFormatContext *formatCtx = avformat_alloc_context();
    AVCodecContext *codecContext = nullptr;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    int audioIndex = -1;

    if (avformat_open_input(&formatCtx,url.toLocal8Bit().data(),nullptr,nullptr)<0)
    {
        qDebug()<<"could not open input";
        return -1;
    }

    if (avformat_find_stream_info(formatCtx,nullptr)<0)
    {
        qDebug()<<"could not find stream info";
        return -1;
    }
    audioIndex = av_find_best_stream(formatCtx,AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audioIndex < 0)
    {
        qDebug()<<"could not find audio stream";
        return -1;
    }

    AVCodecParameters *codeParam = formatCtx->streams[audioIndex]->codecpar;

    const AVCodec *codec = avcodec_find_decoder(codeParam->codec_id);

    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext,codeParam);

    if (avcodec_open2(codecContext,codec,0)<0)
    {
        qDebug()<<"could not open codec";
        return -1;
    }

    SwrContext *swr = swr_alloc();

    AVChannelLayout outLayout;

    av_channel_layout_default(&outLayout,audioFormat.channelCount());

    av_opt_set_chlayout(swr,"out_chlayout",&outLayout,0);
    av_opt_set_sample_fmt(swr,"out_sample_fmt",AV_SAMPLE_FMT_S16,0);
    av_opt_set_int(swr,"out_sample_rate",audioFormat.sampleRate(),0);


    av_opt_set_chlayout(swr,"in_chlayout",&codecContext->ch_layout,0);
    av_opt_set_sample_fmt(swr,"in_sample_fmt",codecContext->sample_fmt,0);
    av_opt_set_int(swr,"in_sample_rate",codecContext->sample_rate,0);


    swr_init(swr);

    uint8_t *outBuf = nullptr;
    int out_linesize = 0;

    av_samples_alloc(&outBuf,&out_linesize,audioFormat.channelCount(),MAX_AUDIO_FRAME_SIZE,AV_SAMPLE_FMT_S16,0);


    while (av_read_frame(formatCtx,packet)>=0)
    {
        if (packet->stream_index!=audioIndex)
        {
            av_packet_unref(packet);
            continue;
        }

        if (avcodec_send_packet(codecContext,packet)<0)
        {
            qDebug()<<"could not send packet";
            return -1;
        }

        while (avcodec_receive_frame(codecContext,frame)>=0)
        {
            int len = swr_convert(swr,&outBuf,MAX_AUDIO_FRAME_SIZE,frame->data,frame->nb_samples);
            if (len<=0)continue;

            int out_size = av_samples_get_buffer_size(nullptr,audioFormat.channelCount(),len,AV_SAMPLE_FMT_S16,1);


            while (audioSink->bytesFree()<out_size)
            {
                QThread::msleep(5);
            }

            streamOut->write((char*)outBuf,out_size);
            av_frame_unref(frame);
        }
        av_packet_unref(packet);
    }

    QThread::sleep(1);

    av_free(outBuf);
    swr_free(&swr);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatCtx);
    return 0;
}
