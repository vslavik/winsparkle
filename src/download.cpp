/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2010 Vaclav Slavik
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
    std::wstring userAgent =
        Settings::GetAppName() + L"/" + Settings::GetAppVersion() +
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

	const int MAX_COMP_LENGTH = 512; 
	char host[MAX_COMP_LENGTH];
	char user[MAX_COMP_LENGTH];
	char password[MAX_COMP_LENGTH];
	char path[MAX_COMP_LENGTH];
	char extra[MAX_COMP_LENGTH];
	URL_COMPONENTSA urlc;

	memset(&urlc, 0, sizeof(urlc));
	urlc.dwStructSize = sizeof(urlc);
	urlc.lpszHostName = host;
	urlc.dwHostNameLength = MAX_COMP_LENGTH;
	urlc.lpszUserName = user;
	urlc.dwUserNameLength = MAX_COMP_LENGTH;
	urlc.lpszPassword = password;
	urlc.dwPasswordLength = MAX_COMP_LENGTH;
	urlc.lpszUrlPath = path;
	urlc.dwUrlPathLength = MAX_COMP_LENGTH;
	urlc.lpszExtraInfo = extra;
	urlc.dwExtraInfoLength = MAX_COMP_LENGTH;

	if ( !InternetCrackUrlA(url.c_str(), 0, ICU_DECODE, &urlc) )
		throw Win32Exception();

	InetHandle conn = InternetConnectA(inet, host, urlc.nPort, user, password, 
									   INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)sink);
	if ( !conn )
		throw Win32Exception();

	DWORD dwFlags = 0;
	if ( flags & Download_NoCached )
		dwFlags |= INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;

	std::string fullUrl(path);
	fullUrl += extra;
	const char *mimeTypes[] = { "text/*", "application/*", NULL };
	InetHandle http = HttpOpenRequestA(conn, "GET", fullUrl.c_str(), NULL, NULL, mimeTypes, dwFlags, (DWORD_PTR)sink);

	if ( !http )
		throw Win32Exception();

	if ( HttpSendRequestA(http, NULL, -1, NULL, 0) )
	{
		char buffer[1024];
		DWORD bufLength = sizeof(buffer), hdrIndex = 0;
		buffer[0] = 0;
		if (HttpQueryInfoA(http, HTTP_QUERY_CONTENT_LENGTH, buffer, &bufLength, &hdrIndex))
		{
			sink->SetTotalLength(atol(buffer));
			for ( ;; )
			{
				DWORD read;
				if ( !InternetReadFile(http, buffer, 1024, &read) )
					throw Win32Exception();

				if ( read == 0 )
					break; // all of the file was downloaded

				if ( !sink->Add(buffer, read) )
					break; // the consumer could not handle the data: stop
			}
		}
	}
}

std::string GetURLFileName(const std::string& url)
{
	const int MAX_COMP_LENGTH = 512; 
	char path[MAX_COMP_LENGTH];
	URL_COMPONENTSA urlc;

	memset(&urlc, 0, sizeof(urlc));
	urlc.dwStructSize = sizeof(urlc);
	urlc.lpszUrlPath = path;
	urlc.dwUrlPathLength = MAX_COMP_LENGTH;

	if ( !InternetCrackUrlA(url.c_str(), 0, ICU_DECODE, &urlc) )
		throw Win32Exception();

	char *lastSlash = strrchr(path, '/');
	return (lastSlash ? lastSlash + 1 : path);
}

} // namespace winsparkle
