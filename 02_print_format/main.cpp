//
// Created by zql on 2026/2/27.
//


#include<iostream>

using namespace std;
extern "C" {
#include <libavformat/avformat.h>
}


int main(int argc,char* arg[]){

    AVFormatContext *fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx)
    {
        return -1;
    }

    const char* file = "../resources/input/input.mp4";
    int ret = 0;
    do
    {
        if ((ret = avformat_open_input(&fmt_ctx, file, nullptr, nullptr)) < 0)
        {
            break;
        }

        if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) < 0)
        {
            std::printf("Couldn't find stream information");
            break;
        }
        av_dump_format(fmt_ctx, 0, file, 0);




        // 打印文件名和格式
        printf("Filename: %s, Format: %s\n", fmt_ctx->url, fmt_ctx->iformat->long_name);

        // 打印时长 (转换为秒)
        if (fmt_ctx->duration != AV_NOPTS_VALUE) {
            printf("Duration: %.2f seconds\n", fmt_ctx->duration / (double)AV_TIME_BASE);
        }

        // 遍历所有流
        for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
            AVStream *stream = fmt_ctx->streams[i];
            AVCodecParameters *codec_par = stream->codecpar;

            printf("Stream #%d:\n", i);
            printf("  Type: %s\n", av_get_media_type_string(codec_par->codec_type));
            printf("  Codec: %s\n", avcodec_get_name(codec_par->codec_id));
            printf("  Time Base: %d/%d\n", stream->time_base.num, stream->time_base.den);

            // 如果是视频，打印分辨率
            if (codec_par->codec_type == AVMEDIA_TYPE_VIDEO) {
                printf("  Resolution: %dx%d\n", codec_par->width, codec_par->height);
            }
            // 如果是音频，打印采样率
            else if (codec_par->codec_type == AVMEDIA_TYPE_AUDIO) {
                printf("  Sample Rate: %d Hz\n", codec_par->sample_rate);
            }
        }

        // 读取元数据 (例如标题)
        AVDictionaryEntry *tag = av_dict_get(fmt_ctx->metadata, "title", nullptr, 0);
        if (tag) {
            printf("Title: %s\n", tag->value);
        }

    }
    while (false);

    avformat_free_context(fmt_ctx);
    return 0;
}
