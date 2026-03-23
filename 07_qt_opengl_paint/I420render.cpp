#include "I420render.h"

I420Render::I420Render(QWidget* parent)
    : QOpenGLWidget(parent)
{
    decoder = new FFmpegDecoder;
    ptr=nullptr;
    connect(decoder,&FFmpegDecoder::sigFirst,
            this,[=](uchar* p,int w,int h)
    {
        width = w;
        height = h;

        ptr=p;
    },Qt::QueuedConnection);


    connect(decoder,&FFmpegDecoder::newFrame,
            this,[=]()
    {
        update();
    },Qt::QueuedConnection);
}

I420Render::~I420Render()
{
    makeCurrent();

    glDeleteTextures(1,&m_idy);
    glDeleteTextures(1,&m_idu);
    glDeleteTextures(1,&m_idv);

    doneCurrent();
}

void I420Render::setUrl(QString url)
{
    decoder->setUrl(url);
}

void I420Render::startVideo()
{
    decoder->start();
}

void I420Render::initializeGL()
{
    initializeOpenGLFunctions();

    const char *vString =
            "attribute vec4 vertexPosition;"
            "attribute vec2 textureCoordinate;"
            "varying vec2 texture_Out;"
            "void main(void)"
            "{"
            "    gl_Position = vertexPosition;"
            "    texture_Out = textureCoordinate;"
            "}";

    const char *tString =
            "varying vec2 texture_Out;"
            "uniform sampler2D tex_y;"
            "uniform sampler2D tex_u;"
            "uniform sampler2D tex_v;"
            "void main(void)"
            "{"
            "    vec3 yuv;"
            "    vec3 rgb;"
            "    yuv.x = texture2D(tex_y, texture_Out).r;"
            "    yuv.y = texture2D(tex_u, texture_Out).r - 0.5;"
            "    yuv.z = texture2D(tex_v, texture_Out).r - 0.5;"
            "    rgb = mat3(1.0, 1.0, 1.0,"
            "               0.0, -0.39465, 2.03211,"
            "               1.13983, -0.58060, 0.0) * yuv;"
            "    gl_FragColor = vec4(rgb,1.0);"
            "}";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,vString);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,tString);

    m_program.bindAttributeLocation("vertexPosition",ATTRIB_VERTEX);
    m_program.bindAttributeLocation("textureCoordinate",ATTRIB_TEXTURE);

    m_program.link();
    m_program.bind();

    static const GLfloat ver[]={
        -1.0f,-1.0f,
        1.0f,-1.0f,
        -1.0f,1.0f,
        1.0f,1.0f
    };

    static const GLfloat tex[]={
        0.0f,1.0f,
        1.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f
    };

    glVertexAttribPointer(ATTRIB_VERTEX,2,GL_FLOAT,0,0,ver);
    glEnableVertexAttribArray(ATTRIB_VERTEX);

    glVertexAttribPointer(ATTRIB_TEXTURE,2,GL_FLOAT,0,0,tex);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    m_textureUniformY = m_program.uniformLocation("tex_y");
    m_textureUniformU = m_program.uniformLocation("tex_u");
    m_textureUniformV = m_program.uniformLocation("tex_v");

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    // Y
    glGenTextures(1,&m_idy);
    glBindTexture(GL_TEXTURE_2D,m_idy);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    // U
    glGenTextures(1,&m_idu);
    glBindTexture(GL_TEXTURE_2D,m_idu);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    // V
    glGenTextures(1,&m_idv);
    glBindTexture(GL_TEXTURE_2D,m_idv);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glClearColor(0,0,0,1);
}

void I420Render::resizeGL(int w,int h)
{
    if(h<=0) h=1;
    glViewport(0,0,w,h);
}

void I420Render::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if(!ptr || ptr==nullptr || width==0 || height==0)
    {
        return;
    }

    m_program.bind();
    // glClearColor(1,0,0,1);

    // Y
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,m_idy);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,GL_RED,GL_UNSIGNED_BYTE,ptr);
    glUniform1i(m_textureUniformY,0);

    // U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,m_idu);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width/2,height/2,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width/2,height/2,GL_RED,GL_UNSIGNED_BYTE,ptr+width*height);
    glUniform1i(m_textureUniformU,1);

    // V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,m_idv);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width/2,height/2,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width/2,height/2,GL_RED,GL_UNSIGNED_BYTE,ptr+width*height*5/4);
    glUniform1i(m_textureUniformV,2);

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}