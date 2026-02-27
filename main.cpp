#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

extern "C" {
#include <libavformat/avformat.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavutil/pixdesc.h>
}

namespace fs = std::filesystem;

int add_watermark_to_video(const char* input_file, const char* output_file) {
    av_log_set_level(AV_LOG_INFO);

    AVFormatContext *fmt_ctx = nullptr;
    AVFormatContext *output_fmt_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    const AVCodec *encoder = nullptr, *decoder = nullptr;

    AVFilterContext *buffersink_ctx = nullptr;
    AVFilterContext *buffersrc_ctx = nullptr;
    AVFilterGraph *filter_graph = nullptr;

    int video_stream_index = -1;
    AVPacket pkt;
    AVFrame *frame = nullptr, *filt_frame = nullptr;

    // 1. 打开输入文件
    if (avformat_open_input(&fmt_ctx, input_file, nullptr, nullptr) < 0) {
        std::cerr << "Cannot open input file\n";
        return -1;
    }
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "Cannot find stream info\n";
        return -1;
    }

    // // 2. 找到视频流
    // for (int i = 0; i < fmt_ctx->nb_streams; i++) {
    //     if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    //         video_stream_index = i;
    //         break;
    //     }
    // }
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        std::cerr << "No video stream found\n";
        return -1;
    }

    // 3. 打开解码器
    decoder = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    if (!decoder)
    {
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);
    if (avcodec_open2(codec_ctx, decoder, nullptr) < 0)
    {
        return -1;
    }

    // 4. 创建输出文件上下文
    avformat_alloc_output_context2(&output_fmt_ctx, nullptr, nullptr, output_file);
    if (!output_fmt_ctx){
        return -1;
    }

    // 为每个输入流创建输出流
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *in_stream = fmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(output_fmt_ctx, nullptr);
        avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->codecpar->codec_tag = 0;
        // ✅ 设置输出流的时间基
        out_stream->time_base = (AVRational){1, 30};  // 或者从输入复制
    }

    // 视频编码器
    encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!encoder) {
        std::cerr << "H.264 encoder not found\n";
        return -1;
    }

    AVCodecContext *enc_ctx = avcodec_alloc_context3(encoder);
    enc_ctx->framerate = (AVRational){30, 1};
    enc_ctx->time_base = (AVRational){1, 30};
//    stream->time_base = (AVRational){1, 30};
    av_opt_set(enc_ctx->priv_data, "crf", "23", 0);
    av_opt_set(enc_ctx->priv_data, "preset", "veryfast", 0);
    av_opt_set(enc_ctx->priv_data, "tune", "zerolatency", 0); // 可选: 低延迟模式

    enc_ctx->width = codec_ctx->width;
    enc_ctx->height = codec_ctx->height;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
//    enc_ctx->time_base = av_inv_q(av_guess_frame_rate(fmt_ctx, fmt_ctx->streams[video_stream_index], nullptr));

    enc_ctx->gop_size = 12;
    enc_ctx->max_b_frames = 1;
    enc_ctx->bit_rate = 3000000;

    if (output_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;



    if (avcodec_open2(enc_ctx, encoder, nullptr) < 0) {
        std::cerr << "Cannot open encoder\n";
        return -1;
    }


    avcodec_parameters_from_context(output_fmt_ctx->streams[video_stream_index]->codecpar, enc_ctx);
    // 把编码器参数写回到输出对应的视频流
    // 注意：output_fmt_ctx->streams 与 fmt_ctx->streams 索引一一对应（因为我们前面按输入流创建）
    AVStream *out_video_stream = output_fmt_ctx->streams[video_stream_index];
    avcodec_parameters_from_context(out_video_stream->codecpar, enc_ctx);
    out_video_stream->time_base = enc_ctx->time_base;
    printf("out_stream->time_base = %d/%d\n",
           out_video_stream->time_base.num, out_video_stream->time_base.den);
    // 5. 初始化滤镜
    filter_graph = avfilter_graph_alloc();
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");


// 获取输入流的真实 time_base
    AVRational input_time_base = fmt_ctx->streams[video_stream_index]->time_base;
    AVRational input_framerate = av_guess_frame_rate(fmt_ctx, fmt_ctx->streams[video_stream_index], nullptr);
    if (!input_framerate.num || !input_framerate.den) {
        input_framerate = (AVRational){30, 1};
    }
    // 重要：这里传入固定的 time_base（enc_ctx->time_base）以确保滤镜链时间基与编码器一致
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d:frame_rate=%d/%d",
             codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
             input_time_base.num, input_time_base.den,  // ✅ 关键！
             codec_ctx->sample_aspect_ratio.num, codec_ctx->sample_aspect_ratio.den,
             input_framerate.num, input_framerate.den);

    avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, nullptr, filter_graph);
    avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", nullptr, nullptr, filter_graph);
    // 在创建 filter graph 后，设置 sink 的参数
    av_opt_set_bin(buffersink_ctx, "pix_fmts", (uint8_t*)&enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),0);
    av_opt_set_int(buffersink_ctx, "video_width", enc_ctx->width, 0);
    av_opt_set_int(buffersink_ctx, "video_height", enc_ctx->height, 0);

// ✅ 设置时间基和帧率
    av_opt_set_q(buffersink_ctx, "time_base", av_inv_q(enc_ctx->framerate), 0);
    av_opt_set_q(buffersink_ctx, "frame_rate", enc_ctx->framerate, 0);

//    int ret = avfilter_init_str(buffersink_ctx, NULL);
//    if (ret < 0) {
//        fprintf(stderr, "Cannot initialize buffer sink\n");
//        return ret;
//    }

    AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
    av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

    // drawtext
    const char *filter_descr =
            "drawtext="
            "fontfile=C\\\\Windows\\\\Fonts\\\\arial.ttf:"
            "text=MyWatermark:"
            "fontcolor=blue:"
            "fontsize=50:"
            "x=200:"
            "y=200";

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();

    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = nullptr;

    inputs->name        = av_strdup("out");
    inputs->filter_ctx  = buffersink_ctx;
    inputs->pad_idx     = 0;
    inputs->next        = nullptr;

    if (avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, nullptr) < 0) {
        std::cerr << "Cannot parse filter graph\n";
        return -1;
    }

    if (avfilter_graph_config(filter_graph, nullptr) < 0) {
        std::cerr << "Cannot configure filter graph\n";
        return -1;
    }


    // 6. 打开输出文件
    if (!(output_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&output_fmt_ctx->pb, output_file, AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Cannot open output file\n";
            goto fail;
        }
    }
    avformat_write_header(output_fmt_ctx, nullptr);

    // 7. 主循环
    frame = av_frame_alloc();
    filt_frame = av_frame_alloc();


    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_index) {
            avcodec_send_packet(codec_ctx, &pkt);
            while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                av_buffersrc_add_frame(buffersrc_ctx, frame);

                while (av_buffersink_get_frame(buffersink_ctx, filt_frame) == 0) {
                    printf("filt_frame pts = %lld, time_base = %d/%d\n",
                           filt_frame->pts,
                           buffersink_ctx->inputs[0]->time_base.num,
                           buffersink_ctx->inputs[0]->time_base.den);
                    // 将滤镜输出帧的 pts 转换为编码器时间基
                    filt_frame->pts = av_rescale_q(filt_frame->pts,
                                                   fmt_ctx->streams[video_stream_index]->time_base,
                                                   enc_ctx->time_base);
                    AVPacket enc_pkt;
                    av_init_packet(&enc_pkt);
                    enc_pkt.data = nullptr;
                    enc_pkt.size = 0;

                    avcodec_send_frame(enc_ctx, filt_frame);
                    if (avcodec_receive_packet(enc_ctx, &enc_pkt) == 0) {
                        enc_pkt.stream_index = video_stream_index;
                        av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base, out_video_stream->time_base);
                        av_interleaved_write_frame(output_fmt_ctx, &enc_pkt);
                        av_packet_unref(&enc_pkt);
                    }
                    av_frame_unref(filt_frame);
                }
                av_frame_unref(frame);
            }
        } else {
            av_interleaved_write_frame(output_fmt_ctx, &pkt);
        }
        av_packet_unref(&pkt);
    }

    // 刷出缓冲帧
    avcodec_send_frame(enc_ctx, nullptr);
    while (avcodec_receive_packet(enc_ctx, &pkt) == 0) {
        pkt.stream_index = video_stream_index;
        av_packet_rescale_ts(&pkt, enc_ctx->time_base, output_fmt_ctx->streams[video_stream_index]->time_base);
        av_interleaved_write_frame(output_fmt_ctx, &pkt);
        av_packet_unref(&pkt);
    }

    av_write_trailer(output_fmt_ctx);

    success:
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    avcodec_free_context(&codec_ctx);
    avcodec_free_context(&enc_ctx);
    avfilter_graph_free(&filter_graph);
    avformat_close_input(&fmt_ctx);
    if (output_fmt_ctx && !(output_fmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_fmt_ctx->pb);
    avformat_free_context(output_fmt_ctx);
    std::cout << "✅ Watermark added successfully: " << output_file << std::endl;
    return 0;

    fail:
    std::cerr << "❌ Failed\n";
    goto success;
}
// 保存 RGB24 帧为 JPEG
bool save_jpeg_with_ffmpeg(AVFrame* rgb_frame, const std::string& filename) {
    if (!rgb_frame || !rgb_frame->data[0]) return false;

    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!codec) {
        std::cerr << "MJPEG encoder not found\n";
        return false;
    }

    // 1️⃣ 创建编码器上下文并设置宽高、像素格式
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P; // MJPEG常用
    codec_ctx->width = rgb_frame->width;
    codec_ctx->height = rgb_frame->height;
    codec_ctx->time_base = {1, 25};

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        std::cerr << "Failed to open MJPEG codec\n";
        avcodec_free_context(&codec_ctx);
        return false;
    }

    // 2️⃣ 创建目标 YUV frame
    AVFrame* yuv_frame = av_frame_alloc();
//    yuv_frame->color_range = AVCOL_RANGE_MPEG;  // 表示是 TV 范围（有限范围）
//    yuv_frame->colorspace = AVCOL_SPC_BT709;    // 色彩矩阵
    yuv_frame->format = codec_ctx->pix_fmt;
    yuv_frame->width = codec_ctx->width;
    yuv_frame->height = codec_ctx->height;
    int buf_size = av_image_get_buffer_size(codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, 1);
    std::vector<uint8_t> buffer(buf_size);
    av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, buffer.data(),
                         codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, 1);

    // 3️⃣ RGB24 -> YUVJ420P
    SwsContext* sws_ctx = sws_getContext(
            rgb_frame->width, rgb_frame->height, AV_PIX_FMT_RGB24,
            codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            SWS_BICUBIC, nullptr, nullptr, nullptr
    );
    sws_scale(sws_ctx, rgb_frame->data, rgb_frame->linesize, 0, rgb_frame->height,
              yuv_frame->data, yuv_frame->linesize);
    sws_freeContext(sws_ctx);

    // 4️⃣ 编码
    AVPacket* pkt = av_packet_alloc();
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;

    if (avcodec_send_frame(codec_ctx, yuv_frame) < 0 ||
        avcodec_receive_packet(codec_ctx, pkt) < 0) {
        std::cerr << "Failed to encode JPEG\n";
        av_packet_free(&pkt);
        avcodec_free_context(&codec_ctx);
        av_frame_free(&yuv_frame);
        return false;
    }

    // 5️⃣ 写入文件
    std::ofstream ofs(filename, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(pkt->data), pkt->size);
    ofs.close();

    av_packet_unref(pkt);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    av_frame_free(&yuv_frame);

    return true;
}
int extract_audio(const char* input_video, const char* output_audio) {
    AVFormatContext *input_fmt_ctx = nullptr;
    AVFormatContext *output_fmt_ctx = nullptr;
    int audio_stream_index = -1;
    AVPacket pkt;

    // 1. 打开输入文件
    if (avformat_open_input(&input_fmt_ctx, input_video, nullptr, nullptr) < 0) {
        std::cerr << "Could not open input file: " << input_video << std::endl;
        return -1;
    }

    // 2. 获取流信息
    if (avformat_find_stream_info(input_fmt_ctx, nullptr) < 0) {
        std::cerr << "Failed to retrieve input stream information" << std::endl;
        avformat_close_input(&input_fmt_ctx);
        return -1;
    }

    // 3. 查找音频流
    for (int i = 0; i < input_fmt_ctx->nb_streams; i++) {
        if (input_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }

    if (audio_stream_index == -1) {
        std::cerr << "No audio stream found in input file." << std::endl;
        avformat_close_input(&input_fmt_ctx);
        return -1;
    }

    // 4. 创建输出格式上下文（自动推断格式）
    avformat_alloc_output_context2(&output_fmt_ctx, nullptr, nullptr, output_audio);
    if (!output_fmt_ctx) {
        std::cerr << "Could not create output context." << std::endl;
        avformat_close_input(&input_fmt_ctx);
        return -1;
    }

    // 5. 创建新的音频流，并复制编码参数
    AVStream *out_stream = avformat_new_stream(output_fmt_ctx, nullptr);
    AVCodecParameters *in_codecpar = input_fmt_ctx->streams[audio_stream_index]->codecpar;
    if (avcodec_parameters_copy(out_stream->codecpar, in_codecpar) < 0) {
        std::cerr << "Failed to copy codec parameters." << std::endl;
        avformat_free_context(output_fmt_ctx);
        avformat_close_input(&input_fmt_ctx);
        return -1;
    }
    out_stream->time_base = input_fmt_ctx->streams[audio_stream_index]->time_base;

    // 6. 打开输出文件
    AVIOContext *pb = nullptr;
    if (!(output_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&pb, output_audio, AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output file: " << output_audio << std::endl;
            avformat_free_context(output_fmt_ctx);
            avformat_close_input(&input_fmt_ctx);
            return -1;
        }
        output_fmt_ctx->pb = pb;
    }

    // 7. 写入文件头
    if (avformat_write_header(output_fmt_ctx, nullptr) < 0) {
        std::cerr << "Error writing header to output file." << std::endl;
        avio_closep(&output_fmt_ctx->pb);
        avformat_free_context(output_fmt_ctx);
        avformat_close_input(&input_fmt_ctx);
        return -1;
    }

    // 8. 读取包并写入音频流
    while (av_read_frame(input_fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == audio_stream_index) {
            // 调整 PTS/DTS 时间基
            av_packet_rescale_ts(&pkt,
                                 input_fmt_ctx->streams[audio_stream_index]->time_base,
                                 out_stream->time_base);

            pkt.stream_index = 0; // 重映射为输出文件的第0个流

            // 写入音频包
            if (av_interleaved_write_frame(output_fmt_ctx, &pkt) < 0) {
                std::cerr << "Error writing packet to output file." << std::endl;
                av_packet_unref(&pkt);
                break;
            }
        }
        av_packet_unref(&pkt);
    }

    // 9. 写入文件尾
    av_write_trailer(output_fmt_ctx);

    // 10. 清理资源
    avio_closep(&output_fmt_ctx->pb);
    avformat_free_context(output_fmt_ctx);
    avformat_close_input(&input_fmt_ctx);

    std::cout << "Audio extracted successfully: " << output_audio << std::endl;
    return 0;
}
int extract_video(std::string  input_path,std::string output_dir){
    fs::create_directories(output_dir);

    av_log_set_level(AV_LOG_INFO);
    avformat_network_init();

    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, input_path.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Failed to open input\n";
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "Failed to read stream info\n";
        avformat_close_input(&fmt_ctx);
        return -1;
    }
    av_dump_format(fmt_ctx,0,fmt_ctx->url,0);

    int video_stream_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index < 0) {
        std::cerr << "No video stream found\n";
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    const AVCodec* dec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    AVCodecContext* dec_ctx = avcodec_alloc_context3(dec);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);
    avcodec_open2(dec_ctx, dec, nullptr);

    AVFrame* frame = av_frame_alloc();

    AVFrame* rgb_frame = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();
    SwsContext* sws_ctx = nullptr;
    int frame_index = 0, saved = 0;

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_index) {
            avcodec_send_packet(dec_ctx, pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                if (!sws_ctx) {
                    sws_ctx = sws_getContext(
                            frame->width, frame->height, (AVPixelFormat)frame->format,
                            frame->width, frame->height, AV_PIX_FMT_RGB24,
                            SWS_BICUBIC, nullptr, nullptr, nullptr
                    );

                    rgb_frame->format = AV_PIX_FMT_RGB24;
                    rgb_frame->width = frame->width;
                    rgb_frame->height = frame->height;

                    av_frame_get_buffer(rgb_frame, 32);
                }

                sws_scale(
                        sws_ctx, frame->data, frame->linesize, 0, frame->height,
                        rgb_frame->data, rgb_frame->linesize
                );

                std::string frame_type;
                switch (frame->pict_type) {
                    case AV_PICTURE_TYPE_I: frame_type = "I"; break;
                    case AV_PICTURE_TYPE_P: frame_type = "P"; break;
                    case AV_PICTURE_TYPE_B: frame_type = "B"; break;
                    default: frame_type = "O"; break;
                }

                char filename[512];
                sprintf(filename, "%s/frame_%05d_%s.jpg",
                        output_dir.c_str(), frame_index, frame_type.c_str());
                save_jpeg_with_ffmpeg(rgb_frame, filename);
                frame_index++;
                saved++;
            }
        }
        av_packet_unref(pkt);
    }

    std::cout << "Done. Saved " << saved << " frames.\n";

    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_packet_free(&pkt);
    return 0;
}

int main(int argc, char* argv[]) {
    std::string input_path =  "..\\input.mp4";
    std::string output_dir = "..\\output";

//    extract_video(input_path,output_dir);
//    extract_audio(input_path.c_str(), output_dir.append("\\output.aac").c_str());
    // add_watermark_to_video(input_path.c_str(),output_dir.append("\\out.mp4").c_str());
    return 0;
}
//#include <iostream>
//extern "C" {
//#include <libavcodec/avcodec.h>
//}
//
//int main() {
//    std::cout << avcodec_configuration() << std::endl;
//    return 0;
//}