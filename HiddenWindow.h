#ifndef HIDDEN_WINDOW_H
#define HIDDEN_WINDOW_H

#include <windows.h>

#include <string>

class HiddenWindow
{
public:
    HWND hWnd;

    HiddenWindow();
    ~HiddenWindow();

private:
    std::string className;
    WNDCLASSEX windowClass;
};

#endif // HIDDEN_WINDOW_H