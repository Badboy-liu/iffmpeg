//
// Created by zql on 2026/2/27.
//

#include<iostream>

using namespace std;
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}


int main(int argc,char* arg[]){
    const char* file = "..\\input.mp4";
    int ret = 0;
    int videoStreamIndex = -1;

    AVPacket* avPacket = NULL;
    AVCodecContext *avCodecCtx = NULL;
    AVCodecParameters *avCodecPar = NULL;
    const AVCodec *avCodec = NULL;

    AVFormatContext* formatCtx =  avformat_alloc_context();
    if (avformat_open_input(&formatCtx,file,nullptr,nullptr)!=0)
    {
        printf("Error opening input format context\n");
        return -1;
    }


    if (avformat_find_stream_info(formatCtx,nullptr)<0)
    {
        printf("Error not find  stream\n");
        return -1;
    }


    for (int i = 0;i<formatCtx->nb_streams;i++)
    {
        if (formatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex<0)
    {
        printf("Error not find video stream index\n");
        return -1;
    }

    av_dump_format(formatCtx,0,file,0);

    avCodecPar=formatCtx->streams[videoStreamIndex]->codecpar;

    avCodec = avcodec_find_decoder(avCodecPar->codec_id);
    if (avCodec==nullptr)
    {
        printf("Error in avcodec_find_decoder\n");
        return -1;
    }

    avCodecCtx = avcodec_alloc_context3(avCodec);
    if (avCodecCtx==nullptr)
    {
        printf("Error in avcodec_parameters_to_context\n");
        return -1;
    }
    avcodec_parameters_to_context(avCodecCtx,avCodecPar);


    if (avcodec_open2(avCodecCtx,avCodec,nullptr)<0)
    {
        printf("Error in avcodec_open2\n");
        return -1;
    }

    int i = 0;
    avPacket = av_packet_alloc();

    av_new_packet(avPacket,avCodecCtx->width*avCodecCtx->height);

    while (av_read_frame(formatCtx,avPacket)>=0)
    {
        if (avPacket->stream_index==videoStreamIndex)
        {
            i++;
        }
        av_packet_unref(avPacket);
    }

    printf("there are %d frames int total \n",i);


    av_packet_free(&avPacket);
    avcodec_free_context(&avCodecCtx);
    avformat_close_input(&formatCtx);
    avformat_free_context(formatCtx);

    return 0;
}
