/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2016 Vaclav Slavik
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

#include "download.h"

#include "error.h"
#include "settings.h"
#include "utils.h"
#include "winsparkle-version.h"

#include <string>
#include <windows.h>
#include <wininet.h>


namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                helpers
 *--------------------------------------------------------------------------*/

namespace
{

struct InetHandle
{
    InetHandle(HINTERNET handle) : m_handle(handle) {}

    ~InetHandle()
    {
        if ( m_handle )
            InternetCloseHandle(m_handle);
    }

    operator HINTERNET() const { return m_handle; }

    HINTERNET m_handle;
};

std::wstring MakeUserAgent()
{
    std::wstring userAgent =
        Settings::GetAppName() + L"/" + Settings::GetAppVersion() +
        L" WinSparkle/" + AnsiToWide(WIN_SPARKLE_VERSION_STRING);

#ifdef _WIN64
    userAgent += L" (Win64)";
#else
    // If we're running a 32bit process, check if we're on 64bit Windows OS:
    auto f_IsWow64Process = LOAD_DYNAMIC_FUNC(IsWow64Process, kernel32);
    if( f_IsWow64Process )
    {
        BOOL wow64 = FALSE;
        f_IsWow64Process(GetCurrentProcess(), &wow64);
        if ( wow64 )
            userAgent += L" (WOW64)";
    }

#endif

    return userAgent;
}


bool GetHttpHeader(HINTERNET handle, DWORD whatToGet, DWORD& output)
{
    DWORD outputSize = sizeof(output);
    DWORD headerIndex = 0;
    return HttpQueryInfoA
           (
               handle,
               whatToGet | HTTP_QUERY_FLAG_NUMBER,
               &output,
               &outputSize,
               &headerIndex
           ) == TRUE;
}

std::wstring GetURLFileName(const char *url)
{
    const char *lastSlash = strrchr(url, '/');
    const std::string fn(lastSlash ? lastSlash + 1 : url);
    return AnsiToWide(fn);
}

} // anonymous namespace


/*--------------------------------------------------------------------------*
                                public functions
 *--------------------------------------------------------------------------*/

void DownloadFile(const std::string& url, IDownloadSink *sink, int flags)
{
    char url_path[512];
    URL_COMPONENTSA urlc;
    memset(&urlc, 0, sizeof(urlc));
    urlc.dwStructSize = sizeof(urlc);
    urlc.lpszUrlPath = url_path;
    urlc.dwUrlPathLength = sizeof(url_path);

    if ( !InternetCrackUrlA(url.c_str(), 0, ICU_DECODE, &urlc) )
        throw Win32Exception();

    InetHandle inet = InternetOpen
                      (
                          MakeUserAgent().c_str(),
                          INTERNET_OPEN_TYPE_PRECONFIG,
                          NULL, // lpszProxyName
                          NULL, // lpszProxyBypass
                          0     // dwFlags
                      );
    if ( !inet )
        throw Win32Exception();

    DWORD dwFlags = 0;
    if ( flags & Download_NoCached )
        dwFlags |= INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;
    if ( urlc.nScheme == INTERNET_SCHEME_HTTPS )
        dwFlags |= INTERNET_FLAG_SECURE;

    InetHandle conn = InternetOpenUrlA
                      (
                          inet,
                          url.c_str(),
                          NULL, // lpszHeaders
                          -1,   // dwHeadersLength
                          dwFlags,
                          (DWORD_PTR)NULL  // dwContext
                      );
    if ( !conn )
        throw Win32Exception();

    char buffer[10240];

    // Check returned status code - we need to detect 404 instead of
    // downloading the human-readable 404 page:
    DWORD statusCode;
    if ( GetHttpHeader(conn, HTTP_QUERY_STATUS_CODE, statusCode) && statusCode >= 400 )
    {
        throw std::runtime_error("Update file not found on the server.");
    }

    // Get content length if possible:
    DWORD contentLength;
    if ( GetHttpHeader(conn, HTTP_QUERY_CONTENT_LENGTH, contentLength) )
        sink->SetLength(contentLength);

    // Get filename fron Content-Disposition, if available
    char contentDisposition[512];
    DWORD cdSize = 512;
    bool filename_set = false;
    if ( HttpQueryInfoA(conn, HTTP_QUERY_CONTENT_DISPOSITION, contentDisposition, &cdSize, NULL) )
    {
        char *ptr = strstr(contentDisposition, "filename=");
        if ( ptr )
        {
            char c_filename[512];
            ptr += 9;
            while ( *ptr == ' ' )
                ptr++;

            bool quoted = false;
            if ( *ptr == '"' || *ptr == '\'')
            {
                quoted = true;
                ptr++;
            }

            char *ptr2 = c_filename;
            while ( *ptr != ';' && *ptr != 0)
                *ptr2++ = *ptr++;

            if ( quoted )
                *(ptr2 - 1) = 0;
            else
                *ptr2 = 0;

            sink->SetFilename(AnsiToWide(c_filename));
            filename_set = true;
        }
    }

    if ( !filename_set )
    {
        char *optionurl;
        DWORD ousize = 0;
        InternetQueryOptionA(conn, INTERNET_OPTION_URL, nullptr, &ousize);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            optionurl = new char[ousize];
            if ( InternetQueryOptionA(conn, INTERNET_OPTION_URL, optionurl, &ousize) )
                sink->SetFilename(GetURLFileName(optionurl));
            else
                sink->SetFilename(GetURLFileName(urlc.lpszUrlPath));
            
            delete[] optionurl;
        }
        else
        {
            sink->SetFilename(GetURLFileName(urlc.lpszUrlPath));
        }
        
        optionurl = NULL;
    }

    // Download the data:
    for ( ;; )
    {
        DWORD read;
        if ( !InternetReadFile(conn, buffer, sizeof(buffer), &read) )
            throw Win32Exception();

        if ( read == 0 )
            break; // all of the file was downloaded

        sink->Add(buffer, read);
    }
}

} // namespace winsparkle
