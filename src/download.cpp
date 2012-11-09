/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2012 Vaclav Slavik
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

void DownloadFile(const std::string& url, IDownloadSink *sink, int flags)
{
	BOOL isWow64 = FALSE;
	BOOL is64 = FALSE;

#ifdef _WIN64
	is64 = TRUE;
#else
	// If we're running a 32bit process, check if we're running fake 32bit Windows on 64bit Windows
	IsWow64Process(GetCurrentProcess(), &isWow64);

	// If we're running a 32bit binary on a 64bit machine:
	//isWow will be TRUE

	// If we're running a 32bit binary on a 32bit machine:
	//isWow will be FALSE

	// If we have the capability of running a 64bit binary, ask for one even if we're currently running a 32bit binary
	if(isWow64) is64 = TRUE;
#endif

	std::wstring userAgent;

	if(is64)
	{
		userAgent =
        Settings::GetAppName() + L"/" + Settings::GetAppVersion() +
        L" WinSparkle/" + AnsiToWide(WIN_SPARKLE_VERSION_STRING) + L" WIN64";
	}
	else
	{
		userAgent =
        Settings::GetAppName() + L"/" + Settings::GetAppVersion() +
        L" WinSparkle/" + AnsiToWide(WIN_SPARKLE_VERSION_STRING);
	}

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

    DWORD dwFlags = 0;
    if ( flags & Download_NoCached )
        dwFlags |= INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;

    InetHandle conn = InternetOpenUrlA
                      (
                          inet,
                          url.c_str(),
                          NULL, // lpszHeaders
                          -1,   // dwHeadersLength
                          dwFlags,
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
