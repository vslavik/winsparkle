/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2026 Vaclav Slavik
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

#ifndef WINSPARKLE_MMAP_H
#define WINSPARKLE_MMAP_H

#include "error.h"
#include "wrapwin.h"

#include <cstdint>
#include <string>

namespace winsparkle
{

namespace detail
{

template<typename T>
inline auto InvokeCallback(T&& callback, const uint8_t *buffer, size_t length)
{
#ifdef _MSC_VER
    __try
    {
        return callback(buffer, length);
    }
    __except(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        throw std::runtime_error("Failed to read file");
    }
#else
    #warning "SEH exceptions not caught"
    return callback(buffer, length);
#endif
}

template<typename T>
inline auto WithMappedFile(HANDLE handle, T&& callback)
{
    struct Resources
    {
        HANDLE file;
        HANDLE mapping = nullptr;
        void *view = nullptr;

        ~Resources()
        {
            if (view)
                UnmapViewOfFile(view);
            if (mapping)
                CloseHandle(mapping);
            if (file != INVALID_HANDLE_VALUE)
                CloseHandle(file);
        }
    } res{handle};

    if (res.file == INVALID_HANDLE_VALUE)
        throw winsparkle::Win32Exception("Failed to open file");

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(res.file, &fileSize))
        throw winsparkle::Win32Exception("Failed to get file size");

    if (fileSize.QuadPart == 0)
    {
        const uint8_t empty = 0;
        return callback(&empty, 0);
    }

    res.mapping = CreateFileMappingW(res.file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!res.mapping)
        throw winsparkle::Win32Exception("Failed to create file mapping");

    res.view = MapViewOfFile(res.mapping, FILE_MAP_READ, 0, 0, 0);
    if (!res.view)
        throw winsparkle::Win32Exception("Failed to map file");

    return InvokeCallback(callback, static_cast<const uint8_t*>(res.view), static_cast<size_t>(fileSize.QuadPart));
}

} // namespace detail


/// Memory-map the file and call the callback closure with its contents.
template<typename T>
inline auto WithMappedFile(const std::string& filename, T&& callback)
{
    HANDLE handle = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    return detail::WithMappedFile(handle, callback);
}

/// Memory-map the file and call the callback closure with its contents.
template<typename T>
inline auto WithMappedFile(const std::wstring& filename, T&& callback)
{
    HANDLE handle = CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    return detail::WithMappedFile(handle, callback);
}

} // namespace winsparkle

#endif // WINSPARKLE_MMAP_H
