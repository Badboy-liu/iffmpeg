//
// Created by zql on 2026/3/25.
//

#include "FFmpegAudio.h"




FFmpegAudio::FFmpegAudio(QString url,AVFormatContext* fmtCtx,DemuxThread *_demux,int aIndex,
                         std::atomic<double>* clock)
{

    this->aIndex = aIndex;
    this->setUrl(url);
    this->fmtCtx = fmtCtx;
    this->_demux = _demux;
    this->audio_clock = clock;



    QAudioFormat audioFormat;
    audioFormat.setSampleRate(44100);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Int16);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();

    if(!device.isFormatSupported(audioFormat)){
        audioFormat = device.preferredFormat();
    }


    audioSink = new QAudioSink(device,audioFormat);
    audioSink->setVolume(1.0);



    // audioCodecCtx = NULL;
    // pkt = av_packet_alloc();
    audioFrame = av_frame_alloc();
    swrCtx = swr_alloc();
}

FFmpegAudio::~FFmpegAudio()
{
    if(streamOut && streamOut->isOpen()){
        audioOutput->destroyed();
        streamOut->close();
    }
    // if(pkt) av_packet_free(&pkt);
    if(audioFrame) av_frame_free(&audioFrame);
    if(audioCodecCtx) avcodec_free_context(&audioCodecCtx);
    if(fmtCtx) avformat_close_input(&fmtCtx);
    if(swrCtx) swr_free(&swrCtx);
    if(audio_out_buffer) av_free(audio_out_buffer);
}

void FFmpegAudio::setUrl(QString url)
{
    _url = url;
}

void FFmpegAudio::run()
{
    // ===== 音频 =====
    streamOut = audioSink->start();
    auto apara = fmtCtx->streams[aIndex]->codecpar;
    auto acodec = avcodec_find_decoder(apara->codec_id);
    audioCodecCtx = avcodec_alloc_context3(acodec);
    avcodec_parameters_to_context(audioCodecCtx,apara);
    avcodec_open2(audioCodecCtx,acodec,nullptr);


    // swr
    swr_alloc_set_opts2(
        &swrCtx,
        &audioCodecCtx->ch_layout,
        AV_SAMPLE_FMT_S16,
        audioCodecCtx->sample_rate,
        &audioCodecCtx->ch_layout,
        audioCodecCtx->sample_fmt,
        audioCodecCtx->sample_rate,
        0,nullptr
    );
    swr_init(swrCtx);


    uint8_t* audio_buf = (uint8_t*)av_malloc(192000);

    // ⭐ 时钟核心
    QElapsedTimer timer;
    timer.start();
    double base_pts = 0;

    while (!isInterruptionRequested())
    {

        bool ok;
        AVPacket* pkt = _demux->audioQ->pop(ok);
        if (!ok) {
            break; // 线程退出
        }
        if (pkt)
        {

            avcodec_send_packet(audioCodecCtx,pkt);

            while(avcodec_receive_frame(audioCodecCtx,audioFrame)==0){



                int len = swr_convert(
                    swrCtx,
                    &audio_buf,
                    44100,
                    (const uint8_t**)audioFrame->data,
                    audioFrame->nb_samples
                );

                int out_size = av_samples_get_buffer_size(
                    nullptr,
                    audioCodecCtx->ch_layout.nb_channels,
                    len,
                    AV_SAMPLE_FMT_S16,
                    1
                );
                // ⭐ 写音频
                while (audioSink->bytesFree() < out_size) {
                    QThread::msleep(1);
                }
                streamOut->write((char*)audio_buf,out_size);


                // ⭐ 正确 pts
                int64_t best_pts = audioFrame->best_effort_timestamp;

                double pts = best_pts * av_q2d(fmtCtx->streams[aIndex]->time_base);

                // ⭐ 更新时钟（关键）
                base_pts = pts;
                timer.restart();

                double clock = base_pts + timer.elapsed() / 1000.0;
                audio_clock->store(clock);


                qDebug()<<"pts="<<pts;
                qDebug()<<"audio_clock="<<audio_clock->load();
                qDebug()<<"streamOut->write()"<<out_size;
            }
        }
    }



}