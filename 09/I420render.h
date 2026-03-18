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
class I420Render :public QObject,public QOpenGLFunctions_2_0
{
    Q_OBJECT
public:
    I420Render();
    ~I420Render();

    void init();
    void updateTextureInfo(int w,int h);
    void updateTextureData(const YUVData &data);
    void paint();
    void resize(int w,int h);

private:
    QOpenGLShaderProgram m_program;
    QOpenGLTexture *mTexY=nullptr,*mTexU=nullptr,*mTexV=nullptr;

    bool mTextureAlloced = false;
    QVector<QVector2D> vertices;
    QVector<QVector2D> textures;
};


#endif //IFFMPEG_I420RENDER_H