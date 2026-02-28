//
// Created by zql on 2026/2/27.
//

#include<iostream>

using namespace std;

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavdevice/avdevice.h>

#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/ffversion.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

void saveFrame(AVFrame *frame,int width,int height,int iFrame);

int main(int argc,char* arg[]){
    char file[] = "..\\input.mp4";
    int video_stream_index = 0;
    int ret = 0;

    AVFormatContext *avFormatContext = NULL;
    AVPacket *avPacket = NULL;
    AVCodecContext *avCodecContext = NULL;
    const AVCodec *avCodec = NULL;
    AVCodecParameters *avCodecParameters = NULL;
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    avFormatContext = avformat_alloc_context();

    if (avformat_open_input(&avFormatContext,file,nullptr,nullptr)==-1)
    {
        std::cout<<"Couldn't open file"<<std::endl;
        return -1;
    }

    if (avformat_find_stream_info(avFormatContext,nullptr)<0)
    {
        std::cout<<"Couldn't find stream info"<<std::endl;
        return -1;
    }

    for (int i=0;i<avFormatContext->nb_streams;i++)
    {
        const auto stream = avFormatContext->streams[i];
        if (stream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
        }
    }

    if (video_stream_index==-1)
    {
        std::cout<<"Couldn't find video stream"<<std::endl;
        return -1;
    }


    avCodec = avcodec_find_decoder(avFormatContext->streams[video_stream_index]->codecpar->codec_id);
    if (avCodec==nullptr)
    {
        std::cout<<"Couldn't find decoder"<<std::endl;
        return -1;
    }
    avCodecContext = avcodec_alloc_context3(avCodec);
    if (avCodecContext==nullptr)
    {
        std::cout<<"Couldn't allocate context"<<std::endl;
        return -1;
    }
    avcodec_parameters_to_context(
        avCodecContext,
        avFormatContext->streams[video_stream_index]->codecpar
    );

    if (avcodec_open2(avCodecContext,avCodec,nullptr)==-1)
    {
        std::cout<<"Couldn't open codec"<<std::endl;
        return -1;
    }


    SwsContext *img_ctx=sws_getContext(
        avCodecContext->width,avCodecContext->height,avCodecContext->pix_fmt,
        avCodecContext->width,avCodecContext->height,AV_PIX_FMT_RGB32,
        SWS_BICUBIC,NULL,NULL,NULL
        );

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32,avCodecContext->width,avCodecContext->height,1);
    unsigned char* out_buffer = (unsigned char*)av_malloc(numBytes*sizeof(unsigned char));

    int i = 0;
    avPacket =av_packet_alloc();
    av_new_packet(avPacket,avCodecContext->width*avCodecContext->height);


    av_image_fill_arrays(rgbFrame->data,rgbFrame->linesize,out_buffer,AV_PIX_FMT_RGB32,avCodecContext->width,avCodecContext->height,1);

    while (av_read_frame(avFormatContext,avPacket)>=0)
    {
        if (avPacket->stream_index!=video_stream_index)
        {
            continue;
        }
        if (avcodec_send_packet(avCodecContext,avPacket)==0)
        {
            while (avcodec_receive_frame(avCodecContext,yuvFrame)==0)
            {
                if (++i<=101 && i>=90)
                {
                    sws_scale(img_ctx,(const uint8_t* const*)yuvFrame->data,yuvFrame->linesize,0
                        ,yuvFrame->height,rgbFrame->data,rgbFrame->linesize);
                    saveFrame(rgbFrame,avCodecContext->width,avCodecContext->height,i);
                }
            }
        }
        av_packet_unref(avPacket);

    }

    printf("there are %d frames int total \n",i);

    av_packet_free(&avPacket);
    avcodec_free_context(&avCodecContext);
    avformat_close_input(&avFormatContext);
    avformat_free_context(avFormatContext);
    av_frame_free(&rgbFrame);
    av_frame_free(&yuvFrame);

    return 0;
}

void saveFrame(AVFrame *frame,int width,int height,int iframe)
{
    std::cout<<"Saving frame"<<iframe<<std::endl;
    FILE *pFile;
    char filename[100];
    int y;


    sprintf(filename,"frame%d.ppm",iframe);
    pFile = fopen(filename,"wb");

    if (pFile==nullptr)
    {
        printf("pFile is null");
        return;
    }

    fprintf(pFile,"P6\n%d %d\n%d\n",width,height,255);


    for (y=0;y<height;y++)
    {
        fwrite(frame->data[0]+y*frame->linesize[0],1,width*3,pFile);
    }

    fclose(pFile);
}
