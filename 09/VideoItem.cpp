//
// Created by zql on 2026/3/17.
//

#include "VideoItem.h"

#include "../09/I420render.h"
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
    void render() override
    {

        // qDebug() << "render called";
        if (!m_inited)
        {
            m_render.init();   // ✅ 正确位置（GL线程）
            m_inited = true;
        }
        if (m_pendingResize)
        {
            m_render.resize(ba.width, ba.height);
            m_render.updateTextureInfo(ba.width, ba.height);
            m_pendingResize = false;
        }

        if (!ba.Y.isEmpty())
        {
            m_render.updateTextureData(ba);
        }

        m_render.paint();
        update(); // 持续刷新
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



        if (!m_window)
        {
            m_window = p_item->window();
        }


        // 只把最新的有效帧拷贝到渲染线程
        YUVData frame = p_item->getFrame();
        if (!frame.Y.isEmpty())
        {
            ba = frame;
            int randomInt = QRandomGenerator::global()->bounded(1000);
            // qDebug()<<"ba--------";
            // QFile file(std::format("frame99-{0}.yuv",randomInt).c_str());
            QDir dir("frame99");
            if (!dir.exists()) {
                dir.mkpath(".");  // 创建目录
            }

            QFile file(dir.filePath(QString("%1.yuv").arg(randomInt)));
            if(file.open(QIODevice::WriteOnly)){
                file.write(ba.Y);
                file.write(ba.U);
                file.write(ba.V);
                file.close();
            }

        }else {
            //qDebug() << "sync called, no YUV frame yet!";
        }
        if (p_item->infoDirty())
        {
            //qDebug() <<"p_item->infoDirty()";
            m_pendingResize = true;
            m_width = p_item->width();
            m_height = p_item->height();
            p_item->makeInfoDirty(false);
        }
    }
private:
    I420Render m_render;
    QQuickWindow *m_window=nullptr;
    YUVData ba;
    bool m_inited=false;
    bool m_pendingResize =false;
    int m_width;
    int m_height;

};
VideoItem::VideoItem(QQuickItem* parent): QQuickFramebufferObject(parent)
{
    m_decoder = new FFmpegDecoder;
    connect(m_decoder,&FFmpegDecoder::videoInfoReady,this,&VideoItem::onVideoInfoReady);
    setFlag(ItemHasContents, true);   // ⭐ 关键
    startTimer(24);
}

void VideoItem::timerEvent(QTimerEvent* event)
{
    update();

}

YUVData VideoItem::getFrame()
{
    return m_decoder->getFrame();
}



void VideoItem::start()
{
    m_decoder->start();
}

void VideoItem::stop()
{
    if (m_decoder->isRunning())
    {
        m_decoder->quit();
        m_decoder->wait(1000);
    }
}

void VideoItem::setUrl(QString url)
{

    if (url.isEmpty()) {
        m_decoder->setUrl(getPath());
    }else {
        m_decoder->setUrl(url);
    }
}

void VideoItem::onVideoInfoReady(int width, int height)
{
    if (m_videoWidth!=width)
    {
        m_videoWidth=width;
        makeInfoDirty(true);
    }
    if (m_videoHeight!=height)
    {
        m_videoHeight=height;
        makeInfoDirty(true);
    }
}
QQuickFramebufferObject::Renderer* VideoItem::createRenderer() const
{
    qDebug() << "createRenderer called";
    return new VideoFboItem();
}
