//
// Created by zql on 2026/3/5.
//

#ifndef IFFMPEG_I420RENDER_H
#define IFFMPEG_I420RENDER_H
#include <Qt6/QtCore/qtmetamacros.h>
#include <Qt6/QtCore/QVector>
#include <Qt6/QtCore/QDir>
#include <Qt6/QtCore/QRandomGenerator>
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
#include <QOpenGLPixelTransferOptions>
class NV12Render :public QObject,public QOpenGLFunctions_2_0
{
    Q_OBJECT
public:
    NV12Render();
    ~NV12Render();

    void render(uchar*p,int width,int height);

private:
    QOpenGLShaderProgram m_program;
    GLuint idY,idUV;

    QOpenGLBuffer vbo;
};


#endif //IFFMPEG_I420RENDER_H