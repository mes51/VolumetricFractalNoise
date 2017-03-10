#ifndef AEGL_CONTEXT_STORE_H
#define AEGL_CONTEXT_STORE_H

#include <Windows.h>

// AEのOpenGLコンテキスト保存・リストア用
class AEGLContextStore {
public:
    ~AEGLContextStore()
    {
        if (this->hDC != wglGetCurrentDC() || this->hRC != wglGetCurrentContext())
        {
            if (!wglMakeCurrent(this->hDC, this->hRC))
            {
                // show error message
            }
        }
    }

    AEGLContextStore(const AEGLContextStore &context)
    {
        this->hDC = context.hDC;
        this->hRC = context.hRC;
    }

    AEGLContextStore &operator=(const AEGLContextStore &context)
    {
        return AEGLContextStore(context.hDC, context.hRC);
    }

    static AEGLContextStore SaveContext()
    {
        return AEGLContextStore(wglGetCurrentDC(), wglGetCurrentContext());
    }

private:
    HDC hDC;
    HGLRC hRC;

    AEGLContextStore(HDC hDC, HGLRC hRC)
    {
        this->hDC = hDC;
        this->hRC = hRC;
    }
};

#endif // AEGL_CONTEXT_STORE_H