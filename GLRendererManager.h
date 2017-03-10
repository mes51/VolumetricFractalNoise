#ifndef VFN_GL_RENDERER_MANAGER_H
#define VFN_GL_RENDERER_MANAGER_H

#include <memory>
#include <map>
#include <typeinfo>
#include <thread>

#include "GLContext.h"
#include "GLRenderer.h"

class GLRendererManager
{
public:
    GLRendererManager(std::string fragmentShaderCode)
    {
        this->fragmentShaderCode = fragmentShaderCode;
        this->rootContext = std::make_shared<GLContext>();
    }

    ~GLRendererManager();

    std::shared_ptr<GLFilterRenderer> GetRenderer(std::thread::id index, long width, long height);

private:
    std::string fragmentShaderCode;
    std::shared_ptr<GLContext> rootContext;
    std::map<std::thread::id, std::shared_ptr<GLContext>> contexts;
    std::map<std::shared_ptr<GLContext>, std::shared_ptr<GLFilterShaderProgram>> shaders;
    std::map<std::shared_ptr<GLContext>, std::map<std::pair<long, long>, std::shared_ptr<GLFilterRenderer>>> renderers;

    std::shared_ptr<GLContext> GetContext(std::thread::id index);
};

#endif // !VFN_GL_RENDERER_MANAGER_H
