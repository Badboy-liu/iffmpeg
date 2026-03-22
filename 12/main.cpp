#include "../constant.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <iostream>
#include <cstdio>

int main()
{
    std::string inFilePath  = getPath().toStdString();
    const char* inFile  = inFilePath.c_str();
    const char* outFile = "result.h264";

    FILE* fp_out = fopen(outFile, "wb");

    // ===== 输入 =====
    AVFormatContext* ifmt_ctx = nullptr;
    avformat_open_input(&ifmt_ctx, inFile, nullptr, nullptr);
    avformat_find_stream_info(ifmt_ctx, nullptr);

    int video_index = -1;
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        }
    }

    // ===== 解码 =====
    const AVCodec* dec = avcodec_find_decoder(
        ifmt_ctx->streams[video_index]->codecpar->codec_id);

    AVCodecContext* dec_ctx = avcodec_alloc_context3(dec);
    avcodec_parameters_to_context(dec_ctx,
        ifmt_ctx->streams[video_index]->codecpar);

    avcodec_open2(dec_ctx, dec, nullptr);

    // ===== 编码 H264 =====
    const AVCodec* enc = avcodec_find_encoder(AV_CODEC_ID_H264);
    AVCodecContext* enc_ctx = avcodec_alloc_context3(enc);

    enc_ctx->width  = dec_ctx->width;
    enc_ctx->height = dec_ctx->height;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->time_base = AVRational{1, 25};
    enc_ctx->framerate = AVRational{25,1};

    avcodec_open2(enc_ctx, enc, nullptr);

    // ===== sws =====
    SwsContext* sws = sws_getContext(
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
        enc_ctx->width, enc_ctx->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    AVFrame* frame = av_frame_alloc();
    AVFrame* yuv = av_frame_alloc();

    yuv->format = AV_PIX_FMT_YUV420P;
    yuv->width  = enc_ctx->width;
    yuv->height = enc_ctx->height;
    av_frame_get_buffer(yuv, 32);

    AVPacket* in_pkt  = av_packet_alloc();
    AVPacket* out_pkt = av_packet_alloc();

    int pts = 0;

    // ===== 主循环 =====
    while (av_read_frame(ifmt_ctx, in_pkt) >= 0) {

        if (in_pkt->stream_index != video_index) {
            av_packet_unref(in_pkt);
            continue;
        }

        avcodec_send_packet(dec_ctx, in_pkt);

        while (avcodec_receive_frame(dec_ctx, frame) == 0) {

            sws_scale(sws,
                      frame->data, frame->linesize,
                      0, dec_ctx->height,
                      yuv->data, yuv->linesize);

            yuv->pts = pts++;

            avcodec_send_frame(enc_ctx, yuv);

            while (avcodec_receive_packet(enc_ctx, out_pkt) == 0) {

                // 🔥 直接写裸流
                fwrite(out_pkt->data, 1, out_pkt->size, fp_out);

                av_packet_unref(out_pkt);
            }
        }

        av_packet_unref(in_pkt);
    }

    // ===== flush =====
    avcodec_send_frame(enc_ctx, nullptr);
    while (avcodec_receive_packet(enc_ctx, out_pkt) == 0) {
        fwrite(out_pkt->data, 1, out_pkt->size, fp_out);
        av_packet_unref(out_pkt);
    }

    fclose(fp_out);

    std::cout << "H264 encode finished\n";
    return 0;
}