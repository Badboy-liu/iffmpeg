//
// Created by loukas on 2026/3/28.
//


#include <stdio.h>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = nullptr;

    int audioStreamIndex = -1;
    int videoStreamIndex = -1;

    const char *inputFile = "../resources/input/input.mp4";
    const char *outAudioFile = "out.pcm";
    const char *outVideoFile = "out.yuv";


    avdevice_register_all();

    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx,inputFile,nullptr,nullptr)<0) {
        printf("avformat_open_input error\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx,nullptr)<0) {
        printf("avformat_find_stream_info error\n");
        return -1;
    }
    av_dump_format(pFormatCtx,0,inputFile,0);

    audioStreamIndex = av_find_best_stream(pFormatCtx,AVMEDIA_TYPE_AUDIO,
        -1,-1,nullptr,
        0);

    videoStreamIndex = av_find_best_stream(pFormatCtx,AVMEDIA_TYPE_VIDEO,
        -1,-1,nullptr,0);

    if (audioStreamIndex<0) {
        printf("av_find_best_stream error\n");
        return -1;
    }else {
        printf("av_find_best_stream\n");

        const AVCodec* audioCodec = avcodec_find_decoder(pFormatCtx->
            streams[audioStreamIndex]->codecpar->codec_id);

        AVCodecContext *audioCodecCtx = avcodec_alloc_context3(audioCodec);
        if (audioCodecCtx==nullptr) {
            printf("avcodec_alloc_context3 error\n");
            return -1;
        }
        avcodec_parameters_to_context(audioCodecCtx,pFormatCtx->
            streams[audioStreamIndex]->codecpar);

        audioCodecCtx->pkt_timebase = pFormatCtx->streams[audioStreamIndex]->time_base;
        if (avcodec_open2(audioCodecCtx,audioCodec,nullptr)<0) {
            printf("avcodec_open2 error\n");
            return -1;
        }
        AVPacket *packet = av_packet_alloc();

        AVFrame *frame = av_frame_alloc();
        SwrContext *swrContext= swr_alloc();
        AVSampleFormat inSampleFmt = audioCodecCtx->sample_fmt;
        AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;

        int inSampleRate = audioCodecCtx->sample_rate;
        int outSampleRate = 44100;

        uint64_t inChannelLayout = audioCodecCtx->ch_layout.u.mask;
        uint64_t outChannelLayout = AV_CH_LAYOUT_MONO;

        printf("inSampleFmt = %d, inSampleRate = %d, inChannelLayout = %d， name = %s\n", inSampleFmt, inSampleRate,
                (int) inChannelLayout, audioCodec->name);

        AVChannelLayout outLayout;
        av_channel_layout_from_mask(&outLayout, AV_CH_LAYOUT_MONO);

        swr_alloc_set_opts2(&swrContext,&outLayout,outSampleFmt,outSampleRate,
            &audioCodecCtx->ch_layout,inSampleFmt,inSampleRate,0,nullptr);

        swr_init(swrContext);

        // int outChannelNb = av_get_ch
        int outChannels = outLayout.nb_channels;
        int maxSamples = 44100;
        uint8_t **outBuffer = nullptr;
        int outLinesize = 0;
        av_samples_alloc_array_and_samples(
            &outBuffer,
            &outLinesize,
            outChannels,
            maxSamples,
            outSampleFmt,0);
        // uint8_t *outBuffer = (uint8_t *) av_malloc(outSampleRate * audioCodecCtx->ch_layout.nb_channels);

        FILE *outFile = fopen(outAudioFile,"wb+");

        av_seek_frame(pFormatCtx,audioStreamIndex,0,AVSEEK_FLAG_BACKWARD);

        while (av_read_frame(pFormatCtx,packet) >= 0) {
            if (packet->stream_index != audioStreamIndex) {
                av_packet_unref(packet);
                continue;
            }

            int ret = avcodec_send_packet(audioCodecCtx,packet);
            if (ret < 0) {
                printf("avcodec_send_packet error\n");
                av_packet_unref(packet);
                continue;
            }
            while (avcodec_receive_frame(audioCodecCtx,frame) >= 0) {
                int outSamples = swr_convert(swrContext,outBuffer,frame->nb_samples,(const uint8_t**)frame->data,
                    frame->nb_samples);
                if (outSamples <= 0) {
                    continue;
                }
                int outBufferSize = av_samples_get_buffer_size(nullptr,outChannels,
                    outSamples,outSampleFmt,1);

                fwrite(outBuffer[0],1,outBufferSize,outFile);
            }
        }

        av_packet_unref(packet);
        fclose(outFile);
        av_frame_free(&frame);
        av_free(outBuffer);
        swr_free(&swrContext);
        avcodec_free_context(&audioCodecCtx);

    }
    if (videoStreamIndex<0) {
        printf("av_find_best_stream error\n");
        return -1;
    }else {
        printf("av_find_best_stream\n");

        int videoStreamIndex = av_find_best_stream(pFormatCtx,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
        const AVCodecParameters *avCodecParameters =pFormatCtx->streams[videoStreamIndex]->codecpar;
        const AVCodec *videoCodec = avcodec_find_decoder(avCodecParameters->codec_id);
        AVCodecContext *videoCodecCtx = avcodec_alloc_context3(videoCodec);
        avcodec_parameters_to_context(videoCodecCtx,avCodecParameters);

        if (avcodec_open2(videoCodecCtx,videoCodec,nullptr)<0) {
            printf("avcodec_open2 error\n");
            return -1;
        }

        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();
        AVFrame *yuvFrame = av_frame_alloc();

        unsigned char *outBuffer = (unsigned char*)av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, videoCodecCtx->width, videoCodecCtx->height,1) );

            av_image_fill_arrays(yuvFrame->data,yuvFrame->linesize,outBuffer,
                AV_PIX_FMT_YUV420P, videoCodecCtx->width, videoCodecCtx->height,1);

        SwsContext *pImgConvertCtx = sws_getContext(
            videoCodecCtx->width,videoCodecCtx->height,videoCodecCtx->pix_fmt,
            videoCodecCtx->width,videoCodecCtx->height,AV_PIX_FMT_YUV420P,
            SWS_BICUBIC,NULL,NULL,NULL
            );

        printf("width = %d, height = %d, name = %s\n", videoCodecCtx->width,
            videoCodecCtx->height, videoCodec->name);

        FILE *outFile = fopen(outVideoFile,"wb+");

        av_seek_frame(pFormatCtx,videoStreamIndex,0,AVSEEK_FLAG_BACKWARD);

        while (av_read_frame(pFormatCtx,packet) >= 0) {
            if (packet->stream_index != videoStreamIndex) {
                av_packet_unref(packet);
                continue;
            }

            int ret = avcodec_send_packet(videoCodecCtx,packet);
            if (ret < 0) {
                printf("avcodec_send_packet error\n");
                av_packet_unref(packet);
                continue;
            }
            while (avcodec_receive_frame(videoCodecCtx,frame) >= 0) {
                sws_scale(pImgConvertCtx,frame->data,frame->linesize,0,
                    videoCodecCtx->height,yuvFrame->data,yuvFrame->linesize);

                int y_size = videoCodecCtx->width * videoCodecCtx->height;

                fwrite(yuvFrame->data[0],1,y_size,outFile);
                fwrite(yuvFrame->data[1],1,y_size/4,outFile);
                fwrite(yuvFrame->data[2],1,y_size/4,outFile);
                // av_frame_free(&frame);
            }
           av_packet_unref(packet);
        }
        fclose(outFile);
        av_frame_free(&frame);
        av_free(outBuffer);
        av_packet_free(&packet);
        avcodec_free_context(&videoCodecCtx);



    }

    avformat_close_input(&pFormatCtx);



}
