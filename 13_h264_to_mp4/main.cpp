//
// Created by zql on 2026/3/23.
//

#include<iostream>
#include <locale>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/timestamp.h>
#include <libavcodec/bsf.h>
}

using namespace std;



int main(int argc,char* arg[]){
    int frame_index = 0;

    int inVStreamIndex = -1,outVStreamIndex = -1;
    const char* inVFileName = "../result.h264";
    const char* outFileName = "result.mp4";

    AVFormatContext *inFormatCtx = nullptr,*outFormatCtx = nullptr;
    AVCodecParameters *codecParameters = nullptr;
    AVStream *outStream = nullptr;
    const AVCodec *outCodec = nullptr;
    AVCodecContext *outCodecCtx = nullptr;
    AVCodecParameters *outCodecParameters = nullptr;
    AVStream *inStream = nullptr;
    AVPacket *pkt = av_packet_alloc();

    do
    {
       if (avformat_open_input(&inFormatCtx,inVFileName,nullptr,nullptr)<0)
       {
           cout<<"Couldn't open file"<<endl;
           return -1;
       }
        if (avformat_find_stream_info(inFormatCtx,nullptr)<0)
        {
            cout<<"Couldn't find stream info"<<endl;
            return -1;
        }

        inVStreamIndex = av_find_best_stream(inFormatCtx,AVMediaType::AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
        if (inVStreamIndex<0)
        {
            cout<<"Couldn't find stream index"<<endl;
            return -1;
        }


        printf("===============Input information========>\n");

        av_dump_format(inFormatCtx,0,inVFileName,0);

        printf("===============Input information========>\n");


        inStream = inFormatCtx->streams[inVStreamIndex];
        codecParameters = inFormatCtx->streams[inVStreamIndex]->codecpar;
        if (avformat_alloc_output_context2(&outFormatCtx,nullptr,nullptr,outFileName)<0)
        {
            cout<<"Couldn't allocate output context"<<endl;
            return -1;
        }

        if (avio_open(&outFormatCtx->pb,outFileName,AVIO_FLAG_READ_WRITE)<0)
        {
            cout<<"Couldn't open output file"<<endl;
            return -1;
        }

        outStream = avformat_new_stream(outFormatCtx,nullptr);
        if (!outStream)
        {
            cout<<"Couldn't allocate stream"<<endl;
            return -1;
        }
        // outStream->time_base.den = 25;
        // outStream->time_base.num = 1;
        // outVStreamIndex = outStream->index;
        //
        // outCodec = avcodec_find_encoder(codecParameters->codec_id);
        //
        // if (!outCodec)
        // {
        //     cout<<"Couldn't find encoder"<<endl;
        //     return -1;
        // }
        //
        // outCodecCtx = avcodec_alloc_context3(outCodec);
        // if (!outFormatCtx)
        // {
        //     cout<<"Couldn't allocate context"<<endl;
        //     return -1;
        // }
        // outCodecParameters = outFormatCtx->streams[outStream->index]->codecpar;
        //
        // if (avcodec_parameters_copy(outCodecParameters,codecParameters)<0)
        // {
        //     cout<<"Couldn't copy codec parameters"<<endl;
        //     return -1;
        // }
        // if (avcodec_parameters_to_context(outCodecCtx,outCodecParameters)<0)
        // {
        //     cout<<"Couldn't build codec context"<<endl;
        //     return -1;
        // }
        // outCodecCtx->time_base.den = 25;
        // outCodecCtx->time_base.num = 1;
        //
        // if (avcodec_open2(outCodecCtx,outCodec,0)<0)
        // {
        //     cout<<"Couldn't open outCodecCtx"<<endl;
        //     return -1;
        // }
        //
        // av_dump_format(outFormatCtx,0,outFileName,1);
        //
        //
        // if (avformat_write_header(outFormatCtx,nullptr)<0)
        // {
        //     cout<<"Couldn't write header"<<endl;
        //     return -1;
        // }
        // inStream = inFormatCtx->streams[inVStreamIndex];


        // ⭐关键：直接拷贝参数（不要 encoder）
        avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);

        AVRational fps = av_guess_frame_rate(inFormatCtx, inStream, NULL);

        if (fps.num == 0 || fps.den == 0) {
            fps = AVRational{25, 1}; // fallback
        }

        AVRational time_base = av_inv_q(fps); // 1/fps
        outStream->time_base = time_base;
        // 打开输出文件
        if (!(outFormatCtx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&outFormatCtx->pb, outFileName, AVIO_FLAG_WRITE) < 0) {
                cout << "open output failed\n";
                return -1;
            }
        }

        // // ⭐关键：bitstream filter（补 SPS/PPS）
        // const AVBitStreamFilter *bsf = av_bsf_get_by_name("extract_extradata");
        // AVBSFContext *bsfCtx = nullptr;
        // av_bsf_alloc(bsf, &bsfCtx);
        // avcodec_parameters_copy(bsfCtx->par_in, inStream->codecpar);
        // av_bsf_init(bsfCtx);

        // 写头
        if (avformat_write_header(outFormatCtx, nullptr) < 0) {
            cout << "write header failed\n";
            return -1;
        }

        // AVRational time_base = {1, 25};
        // outStream->time_base = time_base;
        while (av_read_frame(inFormatCtx,pkt)>=0)
        {
            if (pkt->stream_index!=inVStreamIndex)
            {
                av_packet_unref(pkt);
                continue;
            }

                // ⭐核心：手动生成时间戳
                pkt->pts = frame_index;
                pkt->dts = frame_index;
                pkt->duration = 1;
                frame_index++;

                av_packet_rescale_ts(pkt,time_base,outStream->time_base);
                pkt->stream_index = outStream->index;
                pkt->pos = -1;

                if (av_interleaved_write_frame(outFormatCtx, pkt) < 0)
                {
                    cout << "write packet failed\n";
                    return -1;
                }
                av_packet_unref(pkt);

        }
        av_write_trailer(outFormatCtx);
    }
    while(0);

    av_packet_unref(pkt);
    avformat_close_input(&outFormatCtx);
    avcodec_free_context(&outCodecCtx);
    avformat_free_context(outFormatCtx);
    avformat_close_input(&inFormatCtx);
    avformat_free_context(inFormatCtx);
    // avio_close(inFormatCtx->pb);

    return 0;
}
