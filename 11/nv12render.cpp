#include "nv12render.h"

NV12Render::NV12Render()
{
    initializeOpenGLFunctions();
    const char* vsrc = R"(
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

            const char* fsrc = R"(
        #version 330 core

        in vec2 textureOut;
        out vec4 fragColor;

        uniform sampler2D textureY;
        uniform sampler2D textureUV;

        void main()
        {
            vec3 yuv;
            vec3 rgb;

            yuv.x = texture(textureY, textureOut.st).r-0.0625;
            yuv.y = texture(textureUV, textureOut.st).r - 0.5;
            yuv.z = texture(textureUV, textureOut.st).g - 0.5;

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

    GLfloat points[]{
        -1.0f,1.0f,
        -1.0f,-1.0f,
        1.0f,1.0f,
        1.0f,-1.0f,

        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        1.0f,0.0f
    };
    vbo.create();
    vbo.bind();
    vbo.allocate(points,sizeof(points));
    GLuint ids[2];
    glGenTextures(2, ids);
    idY = ids[0];
    idUV = ids[1];

}

NV12Render::~NV12Render()
{

}

void NV12Render::render(uchar* p, int width, int height)
{
    if(!p) return;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.bind();
    vbo.bind();
    m_program.enableAttributeArray(0);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(0, GL_FLOAT,0,2,2*sizeof(GLfloat));
    m_program.setAttributeBuffer(1, GL_FLOAT,2*4*sizeof(GLfloat),2,2*sizeof(GLfloat));

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,p);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RG,width >> 1,height >> 1,0,GL_RG,GL_UNSIGNED_BYTE,p + width*height);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_program.setUniformValue("textureY", 1);
    m_program.setUniformValue("textureUV", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program.disableAttributeArray(0);
    m_program.disableAttributeArray(1);
    m_program.release();
}









