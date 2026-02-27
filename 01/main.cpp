//
// Created by zql on 2026/2/27.
//

#ifndef IFFMPEG_MAIN2_H
#define IFFMPEG_MAIN2_H
// #include "external/include/libavcodec/avcodec.h"

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

class main
{
};

#include<iostream>
using namespace std;



int main(int argc,char* arg[]){
    unsigned codeVer = avcodec_version();
    std::cout<<"codeVer:"<<codeVer<<std::endl;

    int ver_major = codeVer>>16&0xff;
    int ver_minor = codeVer>>8&0xff;
    int ver_micro = codeVer&0xff;

    printf("avcodec version is: %d=%d.%d.%d.\n",codeVer,ver_major,ver_minor,ver_micro);
    return 0;
}

#endif //IFFMPEG_MAIN2_H