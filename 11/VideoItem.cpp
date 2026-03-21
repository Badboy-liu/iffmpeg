//
// Created by zql on 2026/3/17.
//

#include "VideoItem.h"

#include "Qt6/QtQuick/QQuickWindow"
#include <QOpenGLFramebufferObjectFormat>
#include <QRandomGenerator>
#include <QDebug>
#include <QDir>

#include "../constant.h"

class VideoFboItem:public QQuickFramebufferObject::Renderer
{
    public:
    VideoFboItem()
    {

    }
    ~VideoFboItem()
    {

    }
    void render()
    {

       m_render.render(ptr,m_width,m_height);
    }
    QOpenGLFramebufferObject *createFramebufferObject(const QSize& size) override
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }
    void synchronize(QQuickFramebufferObject* item) override
    {

        //qDebug() << "sync called";
        VideoItem* p_item = qobject_cast<VideoItem*>(item);
        if (!p_item) return;

        // 只把最新的有效帧拷贝到渲染线程
        p_item->getFrame(&ptr,&m_width,&m_height);

    }
private:
    NV12Render m_render;
    int m_width;
    int m_height;
    uchar *ptr=nullptr;//视频图像数据指针
};

VideoItem::VideoItem(QQuickItem* parent): QQuickFramebufferObject(parent)
{
    m_decoder = new FFmpegDecoder;
    connect(m_decoder,&FFmpegDecoder::newFrame,this,&VideoItem::update);
    this->m_url = getPath();
}
VideoItem::~VideoItem()
{
    m_timer->stop();
    delete m_timer;
    stop();
    delete m_decoder;
}

QQuickFramebufferObject::Renderer* VideoItem::createRenderer()const
{
    return new VideoFboItem();
}
void VideoItem::getFrame(uchar** ptr,int*w,int*h)
{
    *ptr=m_decoder->getFrame();
    *w =(int)m_decoder->width;
    *h =(int)m_decoder->height;
}



void VideoItem::start()
{
    stop();
    m_decoder->start();
}

void VideoItem::stop()
{
    if (m_decoder->isRunning())
    {
        m_decoder->requestInterruption();
        m_decoder->quit();
        m_decoder->wait(1000);
    }
}

void VideoItem::setUrl(QString url)
{
    if (url.isEmpty()) {
        this->m_url = getPath();
    }else {
        this->m_url=url;
    }

    m_decoder->setUrl(this->m_url);


}


