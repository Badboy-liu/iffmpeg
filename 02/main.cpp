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

    const char* file = "..\\input.mp4";
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
    }
    while (false);

    avformat_free_context(fmt_ctx);
    return 0;
}
