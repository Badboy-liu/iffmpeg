//
// Created by loukas on 2026/3/23.
//

#include <iostream>
#include <ostream>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libswresample/swresample.h>

}


int main(int argc, char *argv[]) {
    const char *inFileName = "../input.mp4";
    const char *outFileName = "test.pcm";
    FILE *outFile = fopen(outFileName, "w+b");
    if (outFile == NULL) {
        std::cout << "Error opening file " << outFileName << std::endl;
        return -1;
    }

    AVFormatContext *formatCtx = avformat_alloc_context();
    AVCodecContext* codec_context = nullptr;
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    int aStreamIndex = -1;
    if(avformat_open_input(&formatCtx,inFileName,NULL,NULL)<0){
        printf("Cannot open input file.\n");
        return -1;
    }
    if(avformat_find_stream_info(formatCtx,NULL)<0){
        printf("Cannot find any stream in file.\n");
        return -1;
    }

    aStreamIndex = av_find_best_stream(formatCtx,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
    if(aStreamIndex<0) {
        printf("Cannot find audio stream in file.\n");
        return -1;
    }
    AVCodecParameters* codecPara = formatCtx->streams[aStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecPara->codec_id);
    if(codec==NULL) {
        printf("Cannot find a valid codec.\n");
        return -1;
    }

    codec_context = avcodec_alloc_context3(codec);
    if (codec==NULL) {
        printf("Cannot allocate the codec context.\n");
        return -1;
    }
    if (avcodec_parameters_to_context(codec_context,codecPara)<0) {
        printf("Cannot initialize the codec context.\n");
        return -1;
    }
    codec_context->pkt_timebase = formatCtx->streams[aStreamIndex]->time_base;
    if (avcodec_open2(codec_context,codec,NULL)<0) {
        printf("Cannot open audio codec.\n");
        return -1;
    }
    AVChannelLayout out_channel_layout = codec_context->ch_layout;

    enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;
    int out_sample_rate = codec_context->sample_rate;
    int out_channels = codec_context->ch_layout.nb_channels;


    SwrContext *swr_ctx = nullptr;
    swr_alloc_set_opts2(&swr_ctx,
        &out_channel_layout,AV_SAMPLE_FMT_S16,out_sample_rate,
        &out_channel_layout,codec_context->sample_fmt,codec_context->sample_rate,0,nullptr);

    swr_init( swr_ctx );

    while(av_read_frame(formatCtx,pkt)>=0) {
        if (pkt->stream_index!=aStreamIndex) {
            av_packet_unref(pkt);
            continue;
        }
        if (avcodec_send_packet(codec_context,pkt)<0) {
            av_packet_unref(pkt);
            continue;
        }
        if (avcodec_receive_frame(codec_context,frame)<0) {
            av_packet_unref(pkt);
            continue;
        }

        uint8_t *dst_data = nullptr;
        int dst_linesize = 0;

        av_samples_alloc(
            &dst_data,
            &dst_linesize,
            out_channels,
            frame->nb_samples,
            AV_SAMPLE_FMT_S16,
            0
        );

        // 构造二维数组（swr需要）
        uint8_t *out[] = { dst_data };

        int len = swr_convert(swr_ctx, out,
                              frame->nb_samples, (const uint8_t **) frame->data, frame->nb_samples);
        if (len < 0) {
            std::cout << "Error when converting audio frame" << std::endl;
            continue;
        }

        int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, out_channels,
            len, out_format, 1);
        fwrite(dst_data, 1, dst_bufsize, outFile);
        av_packet_unref(pkt);
        av_free(dst_data);
    }
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_context);
    avformat_free_context(formatCtx);

    fclose(outFile);

}
