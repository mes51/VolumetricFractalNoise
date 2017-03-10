#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include <Windows.h>
#include <stdlib.h>
#include <stdint.h>

#if (_MSC_VER >= 1400)
#define THREAD_LOCAL __declspec(thread)
#endif

#include <memory>
#include <string>
#include <set>
#include <stdexcept>

#include <glbinding/gl33core/gl.h>

#include "HiddenWindow.h"

class GLContext
{
public:
    std::set<gl::GLextension> extensions;

    GLContext(std::shared_ptr<GLContext> parent = nullptr);
    ~GLContext();

    void SetContext();
    void DeleteContext();

private:
    std::shared_ptr<GLContext> parent;
    std::shared_ptr<HiddenWindow> window;
    HDC hDC;
    HGLRC hRC;

    void Initialize();
};

class GLContextException : public std::runtime_error
{
public:
    DWORD lastError;

    explicit GLContextException(const std::string& message) : std::runtime_error(message) { }

    explicit GLContextException(const char *message) : std::runtime_error(message) { }

    explicit GLContextException(const std::string& message, DWORD lastError) : std::runtime_error(message)
    {
        this->lastError = lastError;
    }

    explicit GLContextException(const char *message, DWORD lastError) : std::runtime_error(message)
    {
        this->lastError = lastError;
    }
};

#endif //GL_CONTEXT_H