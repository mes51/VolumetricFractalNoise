#include "GLRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace gl;

GLRenderer::GLRenderer(std::shared_ptr<GLContext> context, std::shared_ptr<GLShaderProgram> shader)
{
    this->context = context;
    this->shader = shader;
}

GLRenderer::~GLRenderer()
{
    DeleteBuffer();
}

void GLRenderer::ResizeBuffer(long width, long height)
{
    DeleteBuffer();

    glGenFramebuffers(1, &this->frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);

    glGenTextures(1, &this->outputTexture);
    glBindTexture(GL_TEXTURE_2D, this->outputTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->outputTexture, 0);

    this->width = width;
    this->height = height;
}

boolean GLRenderer::IsNeedResize(long width, long height)
{
    return this->width != width || this->height != height;
}

void GLRenderer::DeleteBuffer()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (this->frameBuffer)
    {
        glDeleteFramebuffers(1, &this->frameBuffer);
        this->frameBuffer = 0;
    }
    if (this->outputTexture)
    {
        glDeleteTextures(1, &this->outputTexture);
        this->outputTexture = 0;
    }
}

void GLRenderer::SetContext()
{
    this->context->SetContext();
    this->shader->UseProgram();

    glViewport(0, 0, this->width, this->height);
    glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
    glBindTexture(GL_TEXTURE_2D, this->outputTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    this->shader->BindFragDataLocation("color", 0);
}

long GLRenderer::GetTotalPixels()
{
    return this->width * this->height;
}

void GLRenderer::ReadPixels(gl::GLenum format, void * buffer)
{
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, this->width, this->height, GL_RGBA, format, buffer);
}

void GLFilterRenderer::ResizeBuffer(long width, long height)
{
    GLRenderer::ResizeBuffer(width, height);

    // x, y, z, u, v
    float vertices[] = {
        -1.0F, -1.0F, 0.0F, 0.0F, 0.0F,
        1.0F, -1.0F, 0.0F, 1.0F, 0.0F,
        1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
        -1.0F, 1.0F, 0.0F, 1.0F, 1.0F
    };

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(GLFilterShaderProgram::PositionLocation);
    glEnableVertexAttribArray(GLFilterShaderProgram::UVLocation);
    glVertexAttribPointer(GLFilterShaderProgram::PositionLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(GLFilterShaderProgram::UVLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void GLFilterRenderer::DeleteBuffer()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (this->vao)
    {
        glDeleteVertexArrays(1, &this->vao);
        this->vao = 0;
    }
    if (this->vbo)
    {
        glDeleteBuffers(1, &this->vbo);
        this->vbo = 0;
    }

    GLRenderer::DeleteBuffer();
}

void GLFilterRenderer::SetContext()
{
    GLRenderer::SetContext();

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBindVertexArray(this->vao);
}

void GLFilterRenderer::Draw()
{
    glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glFlush();
}
