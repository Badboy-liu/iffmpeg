//
// Created by zql on 2026/3/16.
//

#ifndef IFFMPEG_I420RENDER2_H
#define IFFMPEG_I420RENDER2_H
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLShaderProgram>

#include "ffmpegdecoder.h"

class I420Render2 :public QOpenGLWidget,public QOpenGLFunctions_2_0
{
    Q_OBJECT
    public:
    I420Render2(QWidget *parent=nullptr);
    ~I420Render2();


    void setUrl(QString url);
    void startVideo();
    void initializeGL() override;

    void resizeGL(int w, int h) override;
    void paintGL() override;

    private:
        QOpenGLShaderProgram m_program;
        QOpenGLBuffer vbo;

    int idY,idU,idV;
    int width,height;

    FFmpegDecoder* decoder;

    uchar* ptr;
};


#endif //IFFMPEG_I420RENDER2_H