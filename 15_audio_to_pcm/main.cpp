//
// Created by loukas on 2026/3/23.
//

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


int main(int argc, char *argv[]) {
    const char infile[] = "../resources/input/input.mp4";
    const char outfile[] = "test.pcm";
    FILE *file = fopen(outfile, "w+b");
    if (file == nullptr) {
        std::cout << "Unable to open file " << infile << std::endl;
        return 0;
    }


    AVFormatContext *fmt_ctx = avformat_alloc_context();
    AVCodecContext* codec_context = nullptr;

    AVPacket *pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    int aStreamIndex = -1;

    if (avformat_open_input(&fmt_ctx, infile, nullptr, nullptr)<0) {
        std::cout << "Unable to open file " << infile << std::endl;
        return 0;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr)<0) {
        std::cout << "Unable to find stream information" << std::endl;
        return 0;
    }

    av_dump_format(fmt_ctx, 0, infile, 0);

    aStreamIndex = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
    if (aStreamIndex < 0) {
        std::cout << "Unable to find stream information" << std::endl;
        return 0;
    }
    AVCodecParameters * aCodePara = fmt_ctx->streams[aStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(aCodePara->codec_id);
    if (codec == nullptr) {
        std::cout << "Codec not found" << std::endl;
        return 0;
    }
    codec_context = avcodec_alloc_context3(codec);
    if (codec_context == nullptr) {
        std::cout << "Unable to allocate codec context" << std::endl;
        return 0;
    }

    if (avcodec_parameters_to_context(codec_context,aCodePara)<0) {
        std::cout << "Unable to set codec parameters to context" << std::endl;
        return 0;
    }

    codec_context->pkt_timebase = fmt_ctx->streams[aStreamIndex]->time_base;

    if (avcodec_open2(codec_context, codec, nullptr)<0) {
        std::cout << "Unable to open codec" << std::endl;
        return 0;
    }

    while (av_read_frame(fmt_ctx,pkt)>=0) {
        if (pkt->stream_index != aStreamIndex) {
            continue;
            av_packet_unref(pkt);
        }

        if (avcodec_send_packet(codec_context, pkt)<0) {
            std::cout << "Unable to send packet" << std::endl;
            continue;
        }

        while (avcodec_receive_frame(codec_context,frame)>=0) {
            if (!av_sample_fmt_is_planar(codec_context->sample_fmt)) {
                continue;
                av_packet_unref(pkt);
            }
            int numBytes = av_get_bytes_per_sample(codec_context->sample_fmt);
            for (int i=0;i<frame->nb_samples;i++) {
                for (int j=0;j<codec_context->ch_layout.nb_channels;j++) {
                    fwrite((char*)frame->data[j]+numBytes*i,1,numBytes,file);
                }
            }
        }
        av_packet_unref(pkt);
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_context);
    avformat_free_context(fmt_ctx);
    fclose(file);

}
