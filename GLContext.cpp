#include "GLContext.h"

#include <string>
#include <set>
#include <atomic>

#include <sstream>

#include <glbinding/callbacks.h>
#include <glbinding/Meta.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>
#include <glbinding/Binding.h>
#include <glbinding/AbstractFunction.h>

#include "Util.h"

using namespace gl33core;

// from wglext.h
typedef const char *(WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092

PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

int contextAttributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    0, 0
};

GLContext::GLContext(std::shared_ptr<GLContext> parent) : window(std::make_shared<HiddenWindow>())
{
    this->parent = parent;
    this->hDC = GetDC(this->window->hWnd);

    this->Initialize();
}

GLContext::~GLContext()
{
    DeleteContext();
}

void GLContext::SetContext()
{
    wglMakeCurrent(this->hDC, this->hRC);

    glbinding::Binding::useCurrentContext();
}

void GLContext::DeleteContext()
{
    if (this->hRC)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(this->hRC);
        this->hRC = NULL;
    }
    if (this->hDC)
    {
        ReleaseDC(this->window->hWnd, this->hDC);
        this->hDC = NULL;
    }
}

void GLContext::Initialize()
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    ClearStructure(pixelFormatDescriptor);

    pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormatDescriptor.nVersion = 1;
    pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits = 32;
    pixelFormatDescriptor.cDepthBits = 8;

    GLuint pixelFormat = ChoosePixelFormat(this->hDC, &pixelFormatDescriptor);
    if (pixelFormat == 0 || !SetPixelFormat(this->hDC, pixelFormat, &pixelFormatDescriptor))
    {
        throw GLContextException("Cannot use PixelFormat");
    }

    if (!this->parent)
    {
        this->hRC = wglCreateContext(this->hDC);
        if (!wglMakeCurrent(this->hDC, this->hRC))
        {
            throw GLContextException("Cannot make current", GetLastError());
        }

        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
        if (!wglGetExtensionsStringARB)
        {
            throw GLContextException("Cannot find wglGetExtensionsStringARB");
        }

        std::string extensions = wglGetExtensionsStringARB(this->hDC);
        if (strstr(extensions.c_str(), "WGL_ARB_create_context") != 0)
        {
            wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        }
        if (!wglGetExtensionsStringARB)
        {
            throw GLContextException("Cannot find wglCreateContextAttribsARB");
        }
    }
    else
    {
        this->hRC = wglCreateContextAttribsARB(this->hDC, 0, &contextAttributes[0]);
        if (!this->hRC)
        {
            throw GLContextException("Cannot make RenderContext");
        }

        if (!wglMakeCurrent(this->hDC, this->hRC))
        {
            throw GLContextException("Cannot make current", GetLastError());
        }

        if (!wglShareLists(this->parent->hRC, this->hRC))
        {
            throw GLContextException("Cannot link context to parent", GetLastError());
        }
    }

    wglMakeCurrent(this->hDC, this->hRC);
    glbinding::Binding::initialize(false);
    this->extensions = glbinding::ContextInfo::extensions();
}