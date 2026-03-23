//
// Created by zql on 2026/3/17.
//

#ifndef IFFMPEG_VIDEOITEM_H
#define IFFMPEG_VIDEOITEM_H

#include <memory>
#include <Qt6/QtCore/qcompilerdetection.h>
#include <Qt6/QtCore/qobjectdefs.h>
#include <Qt6/QtCore/qstring.h>
#include <Qt6/QtCore/qtclasshelpermacros.h>
#include <Qt6/QtQuick/QQuickItem>
#include <Qt6/QtQuick/QQuickFramebufferObject>
#include <Qt6/QtCore/QTimerEvent>
#include <Qt6/QtCore/qtmetamacros.h>

#include "ffmpegdecoder.h"

class VideoItem:public QQuickFramebufferObject
{
    Q_OBJECT
public:
    VideoItem(QQuickItem *parent = nullptr);
    void timerEvent(QTimerEvent *event)override;
    YUVData getFrame();

    bool infoDirty()const
    {
        return m_infoChanged;
    }
    void makeInfoDirty(bool dirty)
    {
        m_infoChanged = dirty;
    }
public slots:
    void onVideoInfoReady(int width,int height);
    void start();
    void stop();
    void setUrl(QString url);
    public:

    Renderer *createRenderer()const override;
    FFmpegDecoder *m_decoder = nullptr;
    int m_videoWidth;
    int m_videoHeight;
    bool m_infoChanged = false;

};


#endif //IFFMPEG_VIDEOITEM_H