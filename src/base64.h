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

namespace detail
{

inline std::vector<uint8_t> Decode(const std::string& encoded, DWORD flags)
{
    if (encoded.empty())
        return {};

    DWORD size = 0;

    if (!CryptStringToBinaryA(encoded.data(), (DWORD)encoded.size(), flags, NULL, &size, NULL, NULL))
    {
        throw std::invalid_argument("Failed to decode base64 data");
    }

    if (size == 0)
        return {};

    std::vector<uint8_t> bin(size);
    if (!CryptStringToBinaryA(encoded.data(), (DWORD)encoded.size(), flags, bin.data(), &size, NULL, NULL))
    {
        throw std::invalid_argument("Failed to decode base64 data");
    }

    return bin;
}

} // namespace detail

inline std::vector<uint8_t> DecodeBase64(const std::string& base64)
{
    return detail::Decode(base64, CRYPT_STRING_BASE64);
}

inline std::vector<uint8_t> DecodePEMToDER(const std::string& pem)
{
    return detail::Decode(pem, CRYPT_STRING_BASE64HEADER);
}

} // namespace winsparkle

#endif // WINSPARKLE_BASE64_H
