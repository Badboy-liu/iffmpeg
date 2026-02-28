//
// Created by zql on 2026/2/28.
//

#include<iostream>
using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main(int argc,char* arg[]){
    auto file = fopen("result.yuv","w+b");

    if(file==nullptr)
    {
        std::cout<<"Error opening file"<<std::endl;
        return -1;
    }
    char file_path[] = "..\\input.mp4";
    int video_stream_index = 0;
    int ret = 0;

    //需要的变量名并初始化
    AVFormatContext *av_format_context=NULL;
    AVPacket *av_packet =nullptr;
    AVCodecContext *av_codec_content=NULL;
    AVCodecParameters *avCodecPara=NULL;
    const AVCodec *av_codec=NULL;
    AVFrame *yuvFrame = av_frame_alloc();


    av_format_context = avformat_alloc_context();

    if (avformat_open_input(&av_format_context,file_path,nullptr,nullptr)==-1)
    {
        std::cout<<"Error opening file"<<std::endl;
        return -1;
    }

    if (avformat_find_stream_info(av_format_context,nullptr)==-1)
    {
        std::cout<<"Error not find video stream "<<std::endl;
        return -1;
    }

    for (int i=0;i<av_format_context->nb_streams;i++)
    {
        if (av_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index=i;
        }
    }


    if (video_stream_index==-1)
    {
        std::cout<<"Error not find video stream index"<<std::endl;
    }


    av_dump_format(av_format_context,0,file_path,0);

    av_codec = avcodec_find_decoder(av_format_context->streams[video_stream_index]->codecpar->codec_id);

    if (av_codec==nullptr)
    {
        std::cout<<"Error not find codec"<<std::endl;
    }

    av_codec_content = avcodec_alloc_context3(av_codec);
    avcodec_parameters_to_context(av_codec_content,av_format_context->streams[video_stream_index]->codecpar);
    if (av_codec_content==nullptr)
    {
        std::cout<<"Error not find codec content"<<std::endl;
        return -1;
    }

    if (avcodec_open2(av_codec_content,av_codec,nullptr)==-1)
    {
        std::cout<<"Error not open codec content"<<std::endl;
        return -1;
    }
    auto width = av_codec_content->width;
    auto height = av_codec_content->height;

    av_packet = av_packet_alloc();
    printf("pix_fmt = %d\n", av_codec_content->pix_fmt);
    int frameCount= 0;
    while (av_read_frame(av_format_context,av_packet)>=0)
    {
        if (av_packet->stream_index==video_stream_index)
        {
            if (avcodec_send_packet(av_codec_content,av_packet)==0)
            {
                while (avcodec_receive_frame(av_codec_content,yuvFrame)>=0)
                {
                    fwrite(yuvFrame->data[0],1,width*height,file);
                    fwrite(yuvFrame->data[1],1,width*height/4,file);
                    fwrite(yuvFrame->data[2],1,width*height/4,file);
                    printf("save frame %d to file.\n",frameCount++);
                    fflush(file);
                }
            }
        }
    }
    av_packet_unref(av_packet);
    av_packet_free(&av_packet);
    avcodec_free_context(&av_codec_content);
    avformat_close_input(&av_format_context);
    avformat_free_context(av_format_context);
    av_frame_free(&yuvFrame);


    return 0;
}
