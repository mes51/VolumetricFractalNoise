#ifndef VFN_UTIL_H
#define VFN_UTIL_H

#include <Windows.h>

template <typename T> inline void ClearStructure(T& v)
{
    ::SecureZeroMemory(&v, sizeof(T));
}

#endif // VFN_UTIL_H