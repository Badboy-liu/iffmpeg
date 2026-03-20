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
#include <Qt6/QtCore/qtmetamacros.h>
#include "Qt6/QtQuick/QQuickWindow"
#include <QOpenGLFramebufferObjectFormat>
#include <QRandomGenerator>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include "ffmpegdecoder.h"
#include "nv12render.h"

class VideoItem:public QQuickFramebufferObject
{
    Q_OBJECT
public:
    VideoItem(QQuickItem *parent = nullptr);
    ~VideoItem();
    void getFrame(uchar**ptr ,int*w,int*h);
    Renderer *createRenderer()const override;
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setUrl(QString url);
    FFmpegDecoder *m_decoder = nullptr;
    QTimer *m_timer = nullptr;
    QString m_url;
};


#endif //IFFMPEG_VIDEOITEM_H