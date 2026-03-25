//
// Created by zql on 2026/3/25.
//

#include "player.h"




Player::Player(const QString& url)
{
    this->setUrl(url);
    avformat_open_input(&fmtCtx, url.toLocal8Bit().data(), nullptr, nullptr);
    avformat_find_stream_info(fmtCtx, nullptr);
    int vIndex = -1;
    int aIndex = -1;
    // 找流
    for(int i=0;i<fmtCtx->nb_streams;i++){
        if(fmtCtx->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
            vIndex = i;
        if(fmtCtx->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
            aIndex = i;
    }

    this->_demux = new DemuxThread(fmtCtx,aIndex,vIndex);
    _demux->start();
    // _demux->wait();
    this->_audio = new FFmpegAudio(url,fmtCtx,_demux,aIndex,&this->audio_clock);
    this->_video = new FFmpegVideo(url,fmtCtx,_demux,vIndex,&this->audio_clock);

    connect(_video, &FFmpegVideo::sendQImage,
        this,   &Player::sendQImage);
}
Player::~Player()
{
}

void Player::setUrl(const QString &u)
{
    url = u;
}

void Player::run()
{





    _audio->start();
    _video->start();

    _audio->wait();
    _video->wait();
    qDebug()<<"play finished";
}