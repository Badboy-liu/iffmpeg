//
// Created by loukas on 2026/3/28.
//
extern "C"
{
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

#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
}

int open_codec_context(int *streamIndex,AVFormatContext *&ofmtCtx,
    AVFormatContext *ifmtCtx,AVMediaType type) {
    AVStream *outStream = nullptr,*inStream = nullptr;
    int ret = -1,index = -1;
    index = av_find_best_stream(ifmtCtx,type,-1,-1,nullptr,0);

    if (index<0) {
        printf("can't find %s stream in input file\n",av_get_media_type_string(type));
        return ret;
    }
    inStream =ifmtCtx->streams[index];

    outStream = avformat_new_stream(ofmtCtx,nullptr);

    if (!outStream) {
        printf("failed to allocate output stream\n");
        return ret;
    }

    ret = avcodec_parameters_copy(outStream->codecpar,inStream->codecpar);
    if (ret<0) {
        printf("failed to copy codec parametes\n");
        return ret;
    }

    outStream->codecpar->codec_tag = 0;
    *streamIndex = index;
    return 0;
}
int main(int argc, char *argv[]) {
    AVFormatContext *ifmtCtx = NULL, *ofmtCtxAudio = NULL, *ofmtCtxVideo = NULL;
    AVPacket *packet;
    int videoIndex = -1, audioIndex = -1;
    int ret = 0;

    char inFilename[128] = "../input.mp4";
    char outFilenameAudio[128] = "output.aac";
    char outFilenameVideo[128] = "output.h265";

    avdevice_register_all();

    ret = avformat_open_input(&ifmtCtx,inFilename,0,0);
    if (ret <0) {
        printf("can't open input file\n");
        return 0;
    }

    ret = avformat_find_stream_info(ifmtCtx,nullptr);
    if (ret<0) {
        printf("can't retrieve input stream information\n");
        return 0;
    }
    avformat_alloc_output_context2(&ofmtCtxVideo,nullptr,nullptr,outFilenameVideo);
    if (!ofmtCtxVideo) {
        printf("can't create video output context");
        return 0;
    }
    avformat_alloc_output_context2(&ofmtCtxAudio,nullptr,nullptr,outFilenameAudio);
    if (!ofmtCtxAudio) {
            printf("can't create audio output context");
        return 0;
    }

    ret = open_codec_context(&videoIndex,ofmtCtxVideo,ifmtCtx,AVMEDIA_TYPE_VIDEO);
    if (ret<0) {
        printf("can't decode video context\n");
        return 0;
    }

    ret = open_codec_context(&audioIndex,ofmtCtxAudio,ifmtCtx,AVMEDIA_TYPE_AUDIO);
    if (ret<0) {
        printf("can't decode audio context\n");
        return 0;
    }
    //Dump Format------------------
    printf("\n==============Input Video=============\n");
    av_dump_format(ifmtCtx, 0, inFilename, 0);
    printf("\n==============Output Video============\n");
    av_dump_format(ofmtCtxVideo, 0, outFilenameVideo, 1);
    printf("\n==============Output Audio============\n");
    av_dump_format(ofmtCtxAudio, 0, outFilenameAudio, 1);
    printf("\n======================================\n");

    if (!(ofmtCtxVideo->oformat->flags& AVFMT_NOFILE)) {
        if (avio_open(&ofmtCtxVideo->pb,outFilenameVideo,AVIO_FLAG_WRITE)<0) {
            printf("can't open output file: %s\n", outFilenameVideo);
            return 0;
        }
    }

    if (!(ofmtCtxAudio->oformat->flags& AVFMT_NOFILE)) {
        if (avio_open(&ofmtCtxAudio->pb,outFilenameAudio,AVIO_FLAG_WRITE)<0) {
            printf("can't open output file: %s\n", outFilenameAudio);
            return 0;
        }
    }

    if (avformat_write_header(ofmtCtxVideo,nullptr)<0) {
        printf("Error occurred when opening video output file\n");
        return 0;
    }

    if (avformat_write_header(ofmtCtxAudio,nullptr)<0) {
        printf("Error occurred when opening audio output file\n");
        return 0;
    }

    packet = av_packet_alloc();
    while (true) {
        AVFormatContext *ofmtCtx;
        AVStream *instream,*outStream;

        if (av_read_frame(ifmtCtx,packet)<0) {
            break; // ❗ 必须 break
        }
        instream = ifmtCtx->streams[packet->stream_index];

        if (packet->stream_index == videoIndex) {
            outStream = ofmtCtxVideo->streams[0];
            ofmtCtx= ofmtCtxVideo;
        }else if (packet->stream_index == audioIndex) {
            outStream = ofmtCtxAudio->streams[0];
            ofmtCtx = ofmtCtxAudio;
        }else {
            continue;
        }
        packet->pts = av_rescale_q_rnd(packet->pts,instream->time_base,outStream->time_base,
            (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet->dts = av_rescale_q_rnd(packet->dts,instream->time_base,outStream->time_base,
            (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

        packet->duration = av_rescale_q(packet->duration,instream->time_base,outStream->time_base);
        packet->pos = -1;
        packet->stream_index = 0;

        if (av_interleaved_write_frame(ofmtCtx,packet)<0) {
            printf("Error muxing packet\n");
            av_packet_unref(packet);
            break;
        }
        av_packet_unref(packet);
    }

    av_write_trailer(ofmtCtxVideo);
    av_write_trailer(ofmtCtxAudio);


    avformat_close_input(&ifmtCtx);

    if (ofmtCtxVideo && !(ofmtCtxVideo->oformat->flags& AVFMT_NOFILE)) {
        avio_close(ofmtCtxVideo->pb);
    }
    if (ofmtCtxAudio && !(ofmtCtxAudio->oformat->flags& AVFMT_NOFILE)) {
        avio_close(ofmtCtxAudio->pb);
    }
    avformat_free_context(ofmtCtxVideo);
    avformat_free_context(ofmtCtxAudio);
}
