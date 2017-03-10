#ifndef VFN_GL_RENDERER_H
#define VFN_GL_RENDERER_H

#include <memory>

#include <glbinding/gl33core/gl.h>

#include "GLContext.h"
#include "GLShaderProgram.h"

class GLRenderer
{
public:
    std::shared_ptr<GLContext> context;
    std::shared_ptr<GLShaderProgram> shader;

    GLRenderer(std::shared_ptr<GLContext> context, std::shared_ptr<GLShaderProgram> shader);
    ~GLRenderer();

    virtual void ResizeBuffer(long width, long height);
    boolean IsNeedResize(long width, long height);
    virtual void DeleteBuffer();
    virtual void SetContext();
    long GetTotalPixels();
    void ReadPixels(gl::GLenum format, void* buffer);

protected:
    long width;
    long height;

private:
    gl::GLuint frameBuffer;
    gl::GLuint multisampleFrameBuffer;
    gl::GLuint outputTexture;
    gl::GLuint multisampleOutputTexture;
};

class GLFilterRenderer : public GLRenderer
{
public:
    GLFilterRenderer(std::shared_ptr<GLContext> context, std::shared_ptr<GLFilterShaderProgram> shader)
        : GLRenderer::GLRenderer(context, shader) { };

    virtual void ResizeBuffer(long width, long height) override;
    virtual void DeleteBuffer() override;
    virtual void SetContext() override;
    void Draw();

private:
    gl::GLuint vao;
    gl::GLuint vbo;
};

#endif // !VFN_GL_RENDERER_H
