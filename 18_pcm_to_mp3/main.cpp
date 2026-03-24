//
// Created by loukas on 2026/3/24.
//

#include <iostream>
#include <ostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libswresample/swresample.h>
}


int main(int argc, char *argv[]) {
    AVFormatContext *formatCtx= nullptr;




    const char*inFileName="../test.pcm";
    const char*outFileName="output.mp3";

    FILE *inFile = fopen(inFileName, "rb");
    if (!inFile) {
        printf("Cannot open input file\n");
        return -1;
    }

    int ret = 0;

    if (avformat_alloc_output_context2(&formatCtx,nullptr,nullptr,outFileName)<0) {
        std::cout<<"Could not allocate memory"<<std::endl;
        return -1;
    }


    const AVCodec * codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
    AVStream *outStream = avformat_new_stream(formatCtx,nullptr);
    if (!outStream) {
        std::cout<<"Could not allocate stream"<<std::endl;
        return -1;
    }
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);



    codecCtx->codec_id = AV_CODEC_ID_MP3;
    codecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    codecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    codecCtx->sample_rate = 44100;
    codecCtx->bit_rate = 128000;

    codecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    codecCtx->time_base = AVRational{1,codecCtx->sample_rate};
    outStream->time_base=codecCtx->time_base;

    if (avcodec_open2(codecCtx,codec,0)<0) {
        std::cout<<"Could not open codec"<<std::endl;
        return -1;
    }

    avcodec_parameters_from_context(outStream->codecpar,codecCtx);

    if (avio_open(&formatCtx->pb,outFileName,AVIO_FLAG_WRITE)<0) {
        std::cout<<"Could not open output file"<<std::endl;
        return -1;
    }


    avformat_write_header(formatCtx,nullptr);

    AVFrame *frame= av_frame_alloc();

    frame->nb_samples = codecCtx->frame_size;
    frame->format = codecCtx->sample_fmt;
    frame->ch_layout=codecCtx->ch_layout;

    av_frame_get_buffer(frame,0);


    SwrContext * swrCtx = swr_alloc();
    AVChannelLayout inLayout;
    av_channel_layout_default(&inLayout,1);

    swr_alloc_set_opts2(
        &swrCtx,
        &codecCtx->ch_layout,
        codecCtx->sample_fmt,
        codecCtx->sample_rate,
        &inLayout,
        AV_SAMPLE_FMT_S16,
        44100,0,nullptr);


    swr_init(swrCtx);

    int in_nb_samples= codecCtx->frame_size;
    int in_buffer_size= in_nb_samples*1*sizeof(int16_t);

    uint8_t *in_buffer= (uint8_t*)av_malloc(in_buffer_size);

    uint8_t **convert_data = nullptr;

    av_samples_alloc_array_and_samples(
        &convert_data,
        nullptr,2,codecCtx->frame_size,codecCtx->sample_fmt,0);


    AVPacket *packet =av_packet_alloc();
    int64_t pts = 0;


    while (true) {
        size_t read_size = fread(in_buffer,1,in_buffer_size,inFile);
        if (read_size<=0) {
            break;
        }
        // ⭐补齐最后一帧
        if (read_size < in_buffer_size) {
            memset(in_buffer + read_size, 0, in_buffer_size - read_size);
        }

        const uint8_t *in_data[1] = {in_buffer};

        swr_convert(swrCtx,
            convert_data,
            codecCtx->frame_size,
            in_data,
            in_nb_samples);


        int per_channel_size = codecCtx->frame_size
        *av_get_bytes_per_sample(codecCtx->sample_fmt);

        for (int ch = 0; ch < codecCtx->ch_layout.nb_channels; ch++) {
            memcpy(frame->data[ch],convert_data[ch],per_channel_size);
        }

        frame->pts = pts;
        pts+=codecCtx->frame_size;

        int ret = avcodec_send_frame(codecCtx,frame);


        while (ret >= 0) {
            ret = avcodec_receive_packet(codecCtx,packet);
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
                std::cout<<"EOF"<<std::endl;
                break;
            }
            av_packet_rescale_ts(packet,codecCtx->time_base,outStream->time_base);
            packet->stream_index=outStream->index;
            av_write_frame(formatCtx,packet);
            av_packet_unref(packet);
        }
    }
    avcodec_send_frame(codecCtx,nullptr);
    while (avcodec_receive_packet(codecCtx,packet)==0) {
        av_packet_rescale_ts(packet,codecCtx->time_base,outStream->time_base);
        packet->stream_index=outStream->index;
        av_write_frame(formatCtx,packet);
        av_packet_unref(packet);
    }
    av_write_trailer(formatCtx);

    fclose(inFile);

    av_free(in_buffer);

    av_freep(&convert_data[0]);
    av_freep(&convert_data);
    av_packet_free(&packet);
    av_frame_free(&frame);
    swr_free(&swrCtx);
    avcodec_free_context(&codecCtx);
    avio_close(formatCtx->pb);
    avformat_free_context(formatCtx);
    printf("encode finished\n");
}
