#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <QApplication>
#include <QDebug>

#include <windows.h>
#include <vector>
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
#include <libswresample/swresample.h>

#include <libavformat/avformat.h>
}

#include "whisper.h"
bool decode_audio(const char* filename,std::vector<float>& pcm)
{
    AVFormatContext* fmt=nullptr;

    avformat_open_input(&fmt,filename,nullptr,nullptr);
    if(avformat_open_input(&fmt,filename,nullptr,nullptr)!=0)
    {
        std::cout<<"open file failed\n";
        return false;
    }
    avformat_find_stream_info(fmt,nullptr);

    int audio_index=av_find_best_stream(fmt,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
    if(audio_index < 0)
    {
        std::cout<<"no audio stream\n";
        return false;
    }
    AVCodecParameters* codecpar=fmt->streams[audio_index]->codecpar;
    std::cout<<"input sample rate "<<codecpar->sample_rate<<std::endl;
    const AVCodec* codec=avcodec_find_decoder(codecpar->codec_id);

    AVCodecContext* ctx=avcodec_alloc_context3(codec);

    avcodec_parameters_to_context(ctx,codecpar);

    avcodec_open2(ctx,codec,nullptr);

    SwrContext* swr = swr_alloc();

    AVChannelLayout out_ch_layout;
    av_channel_layout_default(&out_ch_layout, 1); // mono

    AVChannelLayout in_ch_layout = ctx->ch_layout;

    av_opt_set_chlayout(swr, "in_chlayout", &in_ch_layout, 0);
    av_opt_set_chlayout(swr, "out_chlayout", &out_ch_layout, 0);

    av_opt_set_int(swr, "in_sample_rate", ctx->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", 16000, 0);

    av_opt_set_sample_fmt(swr, "in_sample_fmt", ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);


    swr_init(swr);

    AVPacket* pkt=av_packet_alloc();
    AVFrame* frame=av_frame_alloc();

    float buffer[16192];

    while(av_read_frame(fmt,pkt)>=0)
    {
        if(pkt->stream_index==audio_index)
        {
            avcodec_send_packet(ctx,pkt);

            while(avcodec_receive_frame(ctx,frame)==0)
            {
                uint8_t* out[]={ (uint8_t*)buffer };

                int samples=swr_convert(
                    swr,
                    out,
                    frame->nb_samples,
                    (const uint8_t**)frame->data,
                    frame->nb_samples
                );

                pcm.insert(pcm.end(),buffer,buffer+samples);
            }
        }

        av_packet_unref(pkt);
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);

    swr_free(&swr);

    avcodec_free_context(&ctx);
    avformat_close_input(&fmt);

    return true;
}
std::string recognize(std::vector<float>& pcm)
{
    whisper_context* ctx=
        whisper_init_from_file("../100/ggml-base.bin");

    whisper_full_params params=
        whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

    params.n_threads=8;
    params.print_progress=false;
    params.print_special=false;
    params.print_realtime=false;
    params.print_timestamps=true;

    params.language="zh";

    whisper_full(ctx,params,pcm.data(),pcm.size());

    std::string text;

    int n=whisper_full_n_segments(ctx);

    for(int i=0;i<n;i++)
    {
        text+=whisper_full_get_segment_text(ctx,i);
    }

    whisper_free(ctx);

    return text;
}

int main(int argc, char* argv[])
{
    std::vector<float> pcm;

    decode_audio("../100/input.mp4",pcm);
    for(int i=0;i<20;i++)
    {
        std::cout<<pcm[i]<<" ";
    }
    std::cout<<std::endl;
    if(pcm.empty())
    {
        std::cout<<"decode failed\n";
        return 0;
    }

    std::string text=recognize(pcm);

    std::cout<<"识别结果:"<<std::endl;
    std::cout<<text<<std::endl;
}
