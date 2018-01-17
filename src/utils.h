/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2018 Vaclav Slavik
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _utils_h_
#define _utils_h_

#include "error.h"

#include <string>
#include <string.h>

namespace winsparkle
{

/// Helper class for RIIA handling of allocated buffers
template<typename T>
struct DataBuffer
{
    DataBuffer(size_t size)
    {
        data = new T[size];
        memset(data, 0, size * sizeof(T));
    }

    ~DataBuffer() { delete[] data; }

    operator T*() { return data; }
    operator const T*() const { return data; }

    T *data;
};


// Simple conversion between wide and narrow strings (only safe for ASCII!):

template<typename TIn, typename TOut>
inline std::basic_string<TOut> ConvertString(const std::basic_string<TIn>& s)
{
    std::basic_string<TOut> out;
    out.reserve(s.length());

    for ( typename std::basic_string<TIn>::const_iterator i = s.begin(); i != s.end(); ++i )
    {
        out += static_cast<TOut>(*i);
    }

    return out;
}

inline std::string WideToAnsi(const std::wstring& s)
{
    return ConvertString<wchar_t, char>(s);
}

inline std::wstring AnsiToWide(const std::string& s)
{
    return ConvertString<char, wchar_t>(s);
}


// Checking of Windows version

inline bool IsWindowsVistaOrGreater()
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(
        VerSetConditionMask(
        VerSetConditionMask(
        0, VER_MAJORVERSION, VER_GREATER_EQUAL),
        VER_MINORVERSION, VER_GREATER_EQUAL),
        VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
    osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
    osvi.wServicePackMajor = 0;

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}


// Dynamic loading of symbols that may be unavailable on earlier
// versions of Windows

template<typename T>
inline T* LoadDynamicFunc(const char *func, const char *dll)
{
    return reinterpret_cast<T*>(GetProcAddress(GetModuleHandleA(dll), func));
}

#define LOAD_DYNAMIC_FUNC(func, dll) \
    LoadDynamicFunc<decltype(func)>(#func, #dll)


// Check for insecure URLs
inline bool CheckForInsecureURL(const std::string& url, const std::string& purpose)
{
    if (url.compare(0, 8, "https://") != 0)
    {
        LogError("----------------------------");
        LogError("*** USING INSECURE URL: " + purpose + " from " + url + " ***");
        LogError("----------------------------");
        return false;
    }
    return true;
}

} // namespace winsparkle

#endif // _utils_h_
