#include "I420render.h"

I420Render::I420Render()
{
    mTexY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTexY->setFormat(QOpenGLTexture::R8_UNorm);
    mTexY->setMinificationFilter(QOpenGLTexture::Nearest);
    mTexY->setMagnificationFilter(QOpenGLTexture::Nearest);
    mTexY->setWrapMode(QOpenGLTexture::ClampToEdge);

    mTexU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTexU->setFormat(QOpenGLTexture::R8_UNorm);
    mTexU->setMinificationFilter(QOpenGLTexture::Nearest);
    mTexU->setMagnificationFilter(QOpenGLTexture::Nearest);
    mTexU->setWrapMode(QOpenGLTexture::ClampToEdge);

    mTexV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTexV->setFormat(QOpenGLTexture::R8_UNorm);
    mTexV->setMinificationFilter(QOpenGLTexture::Nearest);
    mTexV->setMagnificationFilter(QOpenGLTexture::Nearest);
    mTexV->setWrapMode(QOpenGLTexture::ClampToEdge);
}

I420Render::~I420Render()
{

}
void I420Render::init()
{
    initializeOpenGLFunctions();
    const char *vsrc = R"(
#version 330 core

layout(location = 0) in vec4 vertexIn;
layout(location = 1) in vec2 textureIn;

out vec2 textureOut;

void main()
{
    gl_Position = vertexIn;
    textureOut = textureIn;
}
)";

    const char *fsrc = R"(
#version 330 core

in vec2 textureOut;
out vec4 fragColor;

uniform sampler2D textureY;
uniform sampler2D textureU;
uniform sampler2D textureV;

void main()
{
    vec3 yuv;
    vec3 rgb;

    yuv.x = texture(textureY, textureOut).r;
    yuv.y = texture(textureU, textureOut).r - 0.5;
    yuv.z = texture(textureV, textureOut).r - 0.5;

    rgb = mat3(
        1.0,      1.0,      1.0,
        0.0,     -0.3455,   1.779,
        1.4075,  -0.7169,   0.0
    ) * yuv;

    fragColor = vec4(rgb, 1.0);
}
)";

    m_program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,vsrc);
    m_program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,fsrc);
    m_program.bindAttributeLocation("vertexIn",0);
    m_program.bindAttributeLocation("textureIn",1);
    m_program.link();
    m_program.bind();

    vertices << QVector2D(-1.0f,1.0f)
             << QVector2D(1.0f,1.0f)
             << QVector2D(-1.0f,-1.0f)
             << QVector2D(1.0f,-1.0f);

    textures << QVector2D(0.0f,1.f)
             << QVector2D(1.0f,1.0f)
             << QVector2D(0.0f,0.0f)
             << QVector2D(1.0f,0.0f);
    mTexY->create();
    mTexU->create();
    mTexV->create();
}

void I420Render::updateTextureInfo(int w, int h)
{

    mTexY ->setSize(w,h);
    mTexY->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    mTexY->setFormat(QOpenGLTexture::R8_UNorm);
    mTexY->setWrapMode(QOpenGLTexture::ClampToEdge);
    mTexY->setMinificationFilter(QOpenGLTexture::Linear);
    mTexY->setMagnificationFilter(QOpenGLTexture::Linear);


    mTexU ->setSize(w/2,h/2);
    mTexU->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    mTexU->setFormat(QOpenGLTexture::R8_UNorm);
    mTexU->setWrapMode(QOpenGLTexture::ClampToEdge);
    mTexU->setMinificationFilter(QOpenGLTexture::Linear);
    mTexU->setMagnificationFilter(QOpenGLTexture::Linear);

    mTexV ->setSize(w/2,h/2);
    mTexV->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    mTexV->setFormat(QOpenGLTexture::R8_UNorm);
    mTexV->setWrapMode(QOpenGLTexture::ClampToEdge);
    mTexV->setMinificationFilter(QOpenGLTexture::Linear);
    mTexV->setMagnificationFilter(QOpenGLTexture::Linear);

    mTextureAlloced = true;

}

void I420Render::updateTextureData(const YUVData& data)
{
    if (data.Y.size()<=0 || data.U.size()<=0||data.V.size()<=0)
    {
        return;
    }

    QOpenGLPixelTransferOptions options;
    options.setImageHeight(data.height);

    options.setRowLength(data.yLineSize);
    mTexY->setData(QOpenGLTexture::Red,QOpenGLTexture::UInt8,data.Y.data(),&options);


    options.setImageHeight(data.height / 2);
    options.setRowLength(data.uLineSize);
    mTexU->setData(QOpenGLTexture::Red,QOpenGLTexture::UInt8,data.U.data(),&options);

    options.setRowLength(data.vLineSize);
    options.setImageHeight(data.height / 2);
    mTexV->setData(QOpenGLTexture::Red,QOpenGLTexture::UInt8,data.V.data(),&options);
}

void I420Render::paint()
{
    qDebug() << "render frame" << mTextureAlloced;

    //
    // if (!mTextureAlloced )
    // {
    //     return;
    // }
    // // qDebug()<<"ba--------";
    // // QFile file(std::format("frame99-{0}.yuv",randomInt).c_str());
    //
    //
    // glClearColor(0.0f,0.0f,0.0f,1.0f);
    // glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    // glDisable(GL_DEPTH_TEST);
    //
    // m_program.bind();
    // m_program.enableAttributeArray("vertexIn");
    // m_program.setAttributeArray("vertexIn",vertices.constData());
    // m_program.enableAttributeArray("textureIn");
    // m_program.setAttributeArray("textureIn",textures.constData());
    //
    // // m_program.enableAttributeArray(0);
    // // m_program.setAttributeArray(0, vertices.constData());
    // //
    // // m_program.enableAttributeArray(1);
    // // m_program.setAttributeArray(1, textures.constData());
    //
    // glActiveTexture(GL_TEXTURE0);
    // mTexY->bind();
    //
    //
    // glActiveTexture(GL_TEXTURE1);
    // mTexU->bind();
    //
    // glActiveTexture(GL_TEXTURE2);
    // mTexV->bind();
    //
    // m_program.setUniformValue("textureY",0);
    // m_program.setUniformValue("textureU",1);
    // m_program.setUniformValue("textureV",2);
    // glDrawArrays(GL_QUADS, 0, 4);
    // m_program.disableAttributeArray("vertexIn");
    // m_program.disableAttributeArray("textureIn");
    // // m_program.disableAttributeArray(0);
    // // m_program.disableAttributeArray(1);
    // m_program.release();

    if(!mTextureAlloced) return;

    m_program.bind();
    m_program.enableAttributeArray(0);
    m_program.setAttributeArray(0, vertices.constData());
    m_program.enableAttributeArray(1);
    m_program.setAttributeArray(1, textures.constData());

    glActiveTexture(GL_TEXTURE0);
    mTexY->bind();
    glActiveTexture(GL_TEXTURE1);
    mTexU->bind();
    glActiveTexture(GL_TEXTURE2);
    mTexV->bind();

    m_program.setUniformValue("textureY", 0);
    m_program.setUniformValue("textureU", 1);
    m_program.setUniformValue("textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program.disableAttributeArray(0);
    m_program.disableAttributeArray(1);
    m_program.release();

}

void I420Render::resize(int w, int h)
{
    glViewport(0, 0, w, h);
}




