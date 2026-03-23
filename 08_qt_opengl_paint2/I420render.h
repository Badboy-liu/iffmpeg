//
// Created by zql on 2026/3/5.
//

#ifndef IFFMPEG_I420RENDER_H
#define IFFMPEG_I420RENDER_H
#include <Qt6/QtCore/qtmetamacros.h>
#include <Qt6/QtWidgets/QWidget>
#include <Qt6/QtOpenGL/QOpenGLFunctions_2_0>
#include <Qt6/QtOpenGL/QOpenGLShaderProgram>
#include <Qt6/QtOpenGL/QOpenGLTexture>
#include <Qt6/QtOpenGL/QOpenGLBuffer>
#include <Qt6/QtOpenGL/QOpenGLShader>
#include <Qt6/QtOpenGLWidgets/QOpenGLWidget>
#include <Qt6/QtWidgets/QWidget>
#include <Qt6/QtCore/qmutex.h>
#include "ffmpegdecoder.h"
#define ATTRIB_VERTEX 0
#define ATTRIB_TEXTURE 1

class I420Render :public QOpenGLWidget,public QOpenGLFunctions_2_0
{
    Q_OBJECT
public:
    I420Render(QWidget *parent =nullptr);
    ~I420Render();

    void setUrl(QString url);

    void startVideo();
    void stopVideo();

    void initializeGL();
    void resizeGL(int w,int h);
    void paintGL();

private:
    QOpenGLShaderProgram m_program;
    GLuint m_textureUniformY,m_textureUniformU,m_textureUniformV;
    GLuint m_idy,m_idu,m_idv;

    int width,height;
    FFmpegDecoder *decoder;
    uchar* ptr;
};


#endif //IFFMPEG_I420RENDER_H