

#include "../constant.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
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
    AVStream* inStream = ifmt_ctx->streams[video_index];

    AVRational fps = av_guess_frame_rate(ifmt_ctx, inStream, NULL);
    if (fps.num == 0 || fps.den == 0) {
        fps = AVRational{25,1};
    }

    enc_ctx->time_base = av_inv_q(fps);
    enc_ctx->framerate = fps;
    enc_ctx->gop_size = fps.num / fps.den;

    enc_ctx->max_b_frames = 0;       // 先禁用 B 帧（你现在阶段很重要）
    enc_ctx->bit_rate = 400000;      // 随便给个码率


    // libx264 推荐
    av_opt_set(enc_ctx->priv_data, "preset", "veryfast", 0);
    av_opt_set(enc_ctx->priv_data, "tune", "zerolatency", 0);

    if (avcodec_open2(enc_ctx, enc, nullptr) < 0) {
        std::cout << "encoder open failed\n";
        return -1;
    }

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

    int64_t  pts = 0;

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
            // AVRational tb = {1, fps.num};
            // yuv->pts = av_rescale_q(pts++, tb, enc_ctx->time_base);
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