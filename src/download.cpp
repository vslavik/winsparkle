/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009 Vaclav Slavik
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

} // anonymous namespace


/*--------------------------------------------------------------------------*
                                DownloadFile()
 *--------------------------------------------------------------------------*/

void DownloadFile(const std::string& url, IDownloadSink *sink)
{
    const Settings& settings = Settings::Get();
    std::wstring userAgent =
        settings.GetAppName() + L"/" + settings.GetAppVersion() +
        L" WinSparkle/" + AnsiToWide(WIN_SPARKLE_VERSION_STRING);

    InetHandle inet = InternetOpen
                      (
                          userAgent.c_str(),
                          INTERNET_OPEN_TYPE_PRECONFIG,
                          NULL, // lpszProxyName
                          NULL, // lpszProxyBypass
                          0     // dwFlags
                      );
    if ( !inet )
        throw Win32Exception();

    InetHandle conn = InternetOpenUrlA
                      (
                          inet,
                          url.c_str(),
                          NULL, // lpszHeaders
                          -1,   // dwHeadersLength
                          0,    // dwFlags
                          NULL  // dwContext
                      );
    if ( !conn )
        throw Win32Exception();

    char buffer[1024];
    for ( ;; )
    {
        DWORD read;
        if ( !InternetReadFile(conn, buffer, 1024, &read) )
            throw Win32Exception();

        if ( read == 0 )
            break; // all of the file was downloaded

        sink->Add(buffer, read);
    }
}

} // namespace winsparkle
