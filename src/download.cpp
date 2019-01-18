/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2019 Vaclav Slavik
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

#ifndef INTERNET_OPTION_ENABLE_HTTP_PROTOCOL
    #define INTERNET_OPTION_ENABLE_HTTP_PROTOCOL 148
#endif
#ifndef HTTP_PROTOCOL_FLAG_HTTP2
    #define HTTP_PROTOCOL_FLAG_HTTP2 0x2
#endif


namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                helpers
 *--------------------------------------------------------------------------*/

namespace
{

struct InetHandle
{
    InetHandle(HINTERNET handle = 0) : m_handle(handle), m_callback(NULL) {}

    ~InetHandle()
    {
        Close();
    }

    InetHandle& operator=(HINTERNET handle)
    {
        Close();
        m_handle = handle;
        return *this;
    }

    void SetStatusCallback(INTERNET_STATUS_CALLBACK callback)
    {
        m_callback = callback;
        InternetSetStatusCallback(m_handle, m_callback);
    }

    void Close()
    {
        if (m_handle)
        {
            if (m_callback)
                InternetSetStatusCallback(m_handle, NULL);
            InternetCloseHandle(m_handle);
            m_handle = NULL;
        }
    }

    operator HINTERNET() const { return m_handle; }

    HINTERNET m_handle;
    INTERNET_STATUS_CALLBACK m_callback;
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
    std::string fn(lastSlash ? lastSlash + 1 : url);
    if (fn.find_first_of('?') != std::string::npos)
        fn = fn.substr(0, fn.find_first_of('?'));
    return AnsiToWide(fn);
}

struct DownloadCallbackContext
{
    DownloadCallbackContext(InetHandle *conn_) : conn(conn_), lastError(ERROR_SUCCESS) {}
    InetHandle *conn;
    DWORD lastError;
    Event eventRequestComplete;
};

void CALLBACK DownloadInternetStatusCallback(_In_ HINTERNET hInternet,
                                             _In_ DWORD_PTR dwContext,
                                             _In_ DWORD     dwInternetStatus,
                                             _In_ LPVOID    lpvStatusInformation,
                                             _In_ DWORD     dwStatusInformationLength)
{
    DownloadCallbackContext *context = (DownloadCallbackContext*)dwContext;
    INTERNET_ASYNC_RESULT *res = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;

    switch (dwInternetStatus)
    {
        case INTERNET_STATUS_HANDLE_CREATED:
            context->conn->m_handle = (HINTERNET)(res->dwResult);
            break;

        case INTERNET_STATUS_REQUEST_COMPLETE:
            context->lastError = res->dwError;
            context->eventRequestComplete.Signal();
            break;
    }
}

void WaitUntilSignaledWithTerminationCheck(Event& event, Thread *thread)
{
    for (;;)
    {
        if (thread)
            thread->CheckShouldTerminate();
        if (event.WaitUntilSignaled(100))
            return;
    }
}

} // anonymous namespace


/*--------------------------------------------------------------------------*
                                public functions
 *--------------------------------------------------------------------------*/

void DownloadFile(const std::string& url, IDownloadSink *sink, Thread *onThread, int flags)
{
    char url_path[2048];
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
                          INTERNET_FLAG_ASYNC // dwFlags
                      );
    if ( !inet )
        throw Win32Exception();

    DWORD dwOption = HTTP_PROTOCOL_FLAG_HTTP2;
    InternetSetOptionW(inet, INTERNET_OPTION_ENABLE_HTTP_PROTOCOL, &dwOption, sizeof(dwOption));

    // Never allow local caching, always contact the server for both
    // appcast feeds and downloads. This is useful in case of
    // misconfigured servers.
    DWORD dwFlags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD;
    // For some requests (appcast feeds), don't even allow proxies to cache,
    // as we need the most up-to-date information.
    if ( flags & Download_BypassProxies )
        dwFlags |= INTERNET_FLAG_PRAGMA_NOCACHE;
    if ( urlc.nScheme == INTERNET_SCHEME_HTTPS )
        dwFlags |= INTERNET_FLAG_SECURE;

    InetHandle conn;

    DownloadCallbackContext context(&conn);
    inet.SetStatusCallback(&DownloadInternetStatusCallback);

    HINTERNET conn_raw = InternetOpenUrlA
                         (
                             inet,
                             url.c_str(),
                             NULL, // lpszHeaders
                             -1,   // dwHeadersLength
                             dwFlags,
                             (DWORD_PTR)&context  // dwContext
                         );
    // InternetOpenUrl() may return NULL handle and then fill it in asynchronously from 
    // DownloadInternetStatusCallback. We must make sure we don't overwrite the handle
    // in that case, or throw an error.
    if (conn_raw)
    {
        conn = conn_raw;
    }
    else
    {
        if (GetLastError() != ERROR_IO_PENDING)
            throw Win32Exception();
    }

    WaitUntilSignaledWithTerminationCheck(context.eventRequestComplete, onThread);

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
        DWORD ousize = 0;
        InternetQueryOptionA(conn, INTERNET_OPTION_URL, NULL, &ousize);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            DataBuffer<char> optionurl(ousize);
            if ( InternetQueryOptionA(conn, INTERNET_OPTION_URL, optionurl, &ousize) )
                sink->SetFilename(GetURLFileName(optionurl));
            else
                sink->SetFilename(GetURLFileName(urlc.lpszUrlPath));
        }
        else
        {
            sink->SetFilename(GetURLFileName(urlc.lpszUrlPath));
        }
    }

    // Download the data:
    char buffer[10240];
    for ( ;; )
    {
        INTERNET_BUFFERS ibuf = { 0 };
        ibuf.dwStructSize = sizeof(ibuf);
        ibuf.lpvBuffer = buffer;
        ibuf.dwBufferLength = sizeof(buffer);

        if (!InternetReadFileEx(conn, &ibuf, IRF_ASYNC | IRF_NO_WAIT, NULL))
        {
            if (GetLastError() != ERROR_IO_PENDING)
                throw Win32Exception();

            WaitUntilSignaledWithTerminationCheck(context.eventRequestComplete, onThread);
            continue;
        }

        if (ibuf.dwBufferLength == 0)
        {
            if (context.lastError != ERROR_SUCCESS)
                throw Win32Exception();
            else
                break; // all of the file was downloaded
        }

        sink->Add(ibuf.lpvBuffer, ibuf.dwBufferLength);
    }
}

} // namespace winsparkle
