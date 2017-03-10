#include "HiddenWindow.h"

#include <Windowsx.h>

#include <memory>
#include <atomic>
#include <sstream>
#include <stdexcept>

#include "Util.h"

#ifndef HIDDEN_WINDOW_WINDOW_CLASS_PREFIX
    #define HIDDEN_WINDOW_WINDOW_CLASS_PREFIX "HiddenWindow"
#endif

std::string GenerateWindowClassName()
{
    static std::atomic_int id = 1;

    std::stringstream stringStream;
    stringStream << HIDDEN_WINDOW_WINDOW_CLASS_PREFIX << "_HiddenWindow_" << id.load();
    id++;

    return stringStream.str();
}

WNDCLASSEX CreateWindowClass(std::string& className)
{
    WNDCLASSEX windowClass;
    ClearStructure(windowClass);

    windowClass.lpszClassName = className.c_str();
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ::DefWindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = GetStockBrush(BLACK_BRUSH);

    return windowClass;
}

HiddenWindow::HiddenWindow()
{
    this->className = GenerateWindowClassName();
    this->windowClass = CreateWindowClass(this->className);

    if (!::RegisterClassEx(&this->windowClass))
    {
        ClearStructure(windowClass);
        throw std::runtime_error("Cannot register window class");
    }

    this->hWnd = ::CreateWindowEx(
        WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
        this->windowClass.lpszClassName,
        "HiddenWindow",
        0,
        0,
        0,
        8,
        8,
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (!this->hWnd)
    {
        throw std::runtime_error("Cannot create window");
    }
}

HiddenWindow::~HiddenWindow()
{
    if (this->hWnd)
    {
        DestroyWindow(this->hWnd);
    }
    if (this->windowClass.lpszClassName)
    {
        ::UnregisterClass(this->windowClass.lpszClassName, NULL);
    }
}