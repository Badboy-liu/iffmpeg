//
// Created by loukas on 2026/3/26.
//

#include "ffmpegwidget.h"

FFmpegVideo::FFmpegVideo() {
}

FFmpegVideo::~FFmpegVideo() {
}


void FFmpegVideo::init_variables() {
    formatCtx = avformat_alloc_context();
    packet = av_packet_alloc();
    yuvFrame = av_frame_alloc();
    rgbFrame = av_frame_alloc();

    avformat_network_init();

    runFlag = true;
}

void FFmpegVideo::free_variables() {
    if (packet) av_packet_free(&packet);
    if (yuvFrame) av_frame_free(&yuvFrame);
    if (rgbFrame) av_frame_free(&rgbFrame);
    if (codecCtx) avcodec_free_context(&codecCtx);
    if (formatCtx) avformat_close_input(&formatCtx);
    if (filterGraph) avfilter_graph_free(&filterGraph);
    if (bufSinkCtx)avfilter_free(bufSinkCtx);
    if (bufSrcCtx)avfilter_free(bufSrcCtx);
    runFlag = false;
}

void FFmpegVideo::setUrl(QString url) {
    _url = url;
}

bool FFmpegVideo::open_input_file() {
    init_variables();

    if (avformat_open_input(&formatCtx, _url.toLocal8Bit().data(),NULL,NULL) < 0) {
        printf("Cannot open input file.\n");
        return 0;
    }

    if (avformat_find_stream_info(formatCtx,NULL) < 0) {
        printf("Cannot find any stream in file.\n");
        return 0;
    }

    int streamCnt = formatCtx->nb_streams;
    for (int i = 0; i < streamCnt; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            continue;
        }
    }

    if (videoStreamIndex == -1) {
        printf("Cannot find video stream in file.\n");
        return 0;
    }


    AVCodecParameters *codecPara = formatCtx->streams[videoStreamIndex]->codecpar;

    if (!(codec = avcodec_find_decoder(codecPara->codec_id))) {
        printf("Cannot find decoder for video stream.\n");
        return 0;
    }

    if (!(codecCtx = avcodec_alloc_context3(codec))) {
        printf("Cannot allocate video codec context.\n");
        return 0;
    }

    if (avcodec_parameters_to_context(codecCtx, codecPara) < 0) {
        printf("Cannot set codec parameters to context.\n");
        return 0;
    }

    if (avcodec_open2(codecCtx, codec,NULL) < 0) {
        printf("Cannot open codec.\n");
        return 0;
    }
    videoWidth = codecCtx->width;
    videoHeight = codecCtx->height;

    img_ctx = sws_getContext(codecCtx->width, codecCtx->height,
                             codecCtx->pix_fmt, codecCtx->width, codecCtx->height,
                             AV_PIX_FMT_RGB32, SWS_BICUBIC,
                             nullptr, nullptr, nullptr);

    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, codecCtx->width, codecCtx->height, 1);

    out_buf = (unsigned char *) av_malloc(numBytes * sizeof(unsigned char));

    int res = av_image_fill_arrays(
        rgbFrame->data, rgbFrame->linesize,
        out_buf,AV_PIX_FMT_RGB32, codecCtx->width, codecCtx->height, 1);

    if (res < 0) {
        printf("Cannot allocate buffer.\n");
        return 0;
    }

    if (!initFilter()) {
        printf("Cannot init filter.\n");
        return 0;
    }
    return true;
}

bool FFmpegVideo::initFilter() {
    AVFilter *bufSrc = (AVFilter *) avfilter_get_by_name("buffer");
    AVFilter *bufSink = (AVFilter *) avfilter_get_by_name("buffersink");

    AVFilterInOut *outFilter = avfilter_inout_alloc();
    AVFilterInOut *inFilter = avfilter_inout_alloc();

    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE,};


    filterGraph = avfilter_graph_alloc();

    if (!outFilter || !inFilter || !filterGraph) {
        ret = AVERROR(ENOMEM);
    }
    AVRational tb = formatCtx->streams[videoStreamIndex]->time_base;


    QString qargs = QString(
                "video_size=%1x%2:pix_fmt=%3:time_base=%4/%5"
            )
            .arg(codecCtx->width)
            .arg(codecCtx->height)
            .arg(codecCtx->pix_fmt)
            .arg(tb.num)
            .arg(tb.den);
    QByteArray argsBA = qargs.toUtf8();

    // char * args =qargs.toLocal8Bit().data();

    int ret = avfilter_graph_create_filter(
        &bufSrcCtx, bufSrc,
        "in",
        argsBA.constData(), nullptr,
        filterGraph);


    if (ret < 0) {
        printf("Cannot create filter.\n");
        return false;
    }

    ret = avfilter_graph_create_filter(
        &bufSinkCtx, bufSink,
        "out", nullptr, nullptr, filterGraph);

    if (ret < 0) {
        printf("Cannot create filter.\n");
        return false;
    }

    outFilter->name = av_strdup("in");
    outFilter->filter_ctx = bufSrcCtx;
    outFilter->pad_idx = 0;
    outFilter->next = nullptr;

    inFilter->name = av_strdup("out");
    inFilter->filter_ctx = bufSinkCtx;
    inFilter->pad_idx = 0;
    inFilter->next = nullptr;

    auto s = filterDescr.toUtf8();
    ret = avfilter_graph_parse_ptr(
        filterGraph, s.constData(),
        &inFilter,
        &outFilter,
        nullptr);

    if (ret < 0) {
        printf("Cannot parse filter.\n");
        return false;
    }


    if (avfilter_graph_config(filterGraph, nullptr) < 0) {
        printf("Cannot parse filter configuration.\n");
        return false;
    }

    // 在 parse + config 之后
    for (unsigned i = 0; i < filterGraph->nb_filters; i++) {
        AVFilterContext *ctx = filterGraph->filters[i];
        if (strcmp(ctx->filter->name, "eq") == 0) {
            eqCtx = ctx;
            break;
        }
    }
    return true;
}
void FFmpegVideo::updateEQ(float contrast, float brightness) {
    if (!eqCtx) return;

    av_opt_set_double(eqCtx, "contrast", contrast, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_double(eqCtx, "brightness", brightness, AV_OPT_SEARCH_CHILDREN);
}
void FFmpegVideo::setCL(int c, int l) {
    c = c < 1 ? 1 : c;
    c = c > 9 ? 9 : c;

    l = l < 1 ? 1 : l;
    l = l > 9 ? 9 : l;

    float cf = 1 + ((float) (c - 5)) / 10.0;
    float lf = 1 + ((float) (l - 5)) / 10.0;
    filterDescr = QString(
        "format=pix_fmts=yuv420p,colorspace=bt709:iall=bt709:fast=1,eq=contrast=%1:brightness=%2"
    ).arg(cf).arg(lf);
    updateEQ(cf, lf);
}

void FFmpegVideo::run() {
    if (!open_input_file()) {
        qDebug() << "Please open file first.";
        return;
    }
    AVFrame *filterFrame = av_frame_alloc();

    while (av_read_frame(formatCtx, packet) >= 0) {
        if (!runFlag) {
            break;
        }
        if (packet->stream_index != videoStreamIndex) {
            av_packet_unref(packet);
            continue;
        }
        if (avcodec_send_packet(codecCtx, packet) < 0) {
            printf("Cannot send packet.\n");
            continue;
        }

        while ((ret = avcodec_receive_frame(codecCtx, yuvFrame)) >= 0) {
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                return;
            } else if (ret < 0) {
                printf("Cannot receive frame.\n");
                exit(0);
            }

            if (av_buffersrc_add_frame_flags(bufSrcCtx, yuvFrame,
                                             AV_BUFFERSRC_FLAG_KEEP_REF)) {
                av_log(nullptr,AV_LOG_ERROR, "Could not get buffersrc frame.");
            }

            if (av_buffersink_get_frame(bufSinkCtx, filterFrame) < 0) {
                printf("Cannot get buffersink frame.\n");
                continue;
            }
            sws_scale(
                img_ctx, filterFrame->data, filterFrame->linesize,
                0, codecCtx->height, rgbFrame->data, rgbFrame->linesize
            );

            QImage img(out_buf, codecCtx->width, codecCtx->height,
                       QImage::Format_RGB32);

            emit sendImg(img);
            QThread::msleep(30);
        }
        av_packet_unref(packet);
    }
}


FFmpegWidget::FFmpegWidget(QWidget *parent) : QWidget(parent) {
    ffmpeg = new FFmpegVideo;
    connect(ffmpeg,SIGNAL(sendImg(QImage)), this,SLOT(receiveQImage(QImage)));
    connect(ffmpeg, &FFmpegVideo::finished, this, &FFmpegVideo::deleteLater);
}

FFmpegWidget::~FFmpegWidget() {
    if (ffmpeg->isRunning()) {
        stop();
    }
}

void FFmpegWidget::play(QString url) {
    stop();
    ffmpeg->setUrl(url);
    ffmpeg->start();
}

void FFmpegWidget::stop() {
    if (ffmpeg->isRunning()) {
        ffmpeg->requestInterruption();
        ffmpeg->quit();
        ffmpeg->wait(100);
    }
    ffmpeg->free_variables();
    img.fill(Qt::black);
}

void FFmpegWidget::setFilterDescr(int c, int b) {
    ffmpeg->setCL(c, b);
}


void FFmpegWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), img);
}


void FFmpegWidget::receiveQImage(const QImage &image) {
    img = image.scaled(this->size());
    update();
}
