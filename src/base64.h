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

#ifndef WINSPARKLE_BASE64_H
#define WINSPARKLE_BASE64_H

#include "wrapwin.h"
#include <wincrypt.h>

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

#ifdef _MSC_VER
#pragma comment(lib, "crypt32.lib")
#endif


namespace winsparkle
{

inline std::string EncodeBase64(const uint8_t* data, size_t len)
{
    if (len == 0)
        return {};

    DWORD base64_len = 0;
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64_len))
    {
        throw std::runtime_error("Failed to encode as base64");
    }

    // base64_len includes the null terminator, the actual string length is one less
    std::string str(base64_len - 1, '\0');
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, str.data(), &base64_len))
    {
        throw std::runtime_error("Failed to encode as base64");
    }

    return str;
}

inline std::vector<uint8_t> DecodeBase64(const std::string& base64)
{
    if (base64.empty())
        return {};

    DWORD size = 0;

    if (!CryptStringToBinaryA(&base64[0], (DWORD)base64.size(), CRYPT_STRING_BASE64, NULL, &size, NULL, NULL))
    {
        throw std::invalid_argument("Failed to decode base64 string");
    }

    if (size == 0)
        return {};

    std::vector<uint8_t> bin(size);
    if (!CryptStringToBinaryA(&base64[0], (DWORD)base64.size(), CRYPT_STRING_BASE64, (BYTE*)&bin[0], &size, NULL, NULL))
    {
        throw std::invalid_argument("Failed to decode base64 string");
    }

    return bin;
}

} // namespace winsparkle

#endif // WINSPARKLE_BASE64_H
