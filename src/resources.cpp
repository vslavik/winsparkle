/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2010 Vaclav Slavik
 *  Copyright (C) 2011 Vasco Veloso
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

#define _CRT_SECURE_NO_WARNINGS

#include "ui.h"

#define wxNO_NET_LIB
#define wxNO_XML_LIB
#define wxNO_XRC_LIB
#define wxNO_ADV_LIB
#define wxNO_HTML_LIB
#include <wx/setup.h>
#include <wx/app.h>

#include "resources.h"

namespace winsparkle
{

bool Resources::LoadDialogIcon(wxIcon &icon)
{
	// load the dialog box icon: the first 48x48 application icon will be loaded, if available;
	// otherwise, the standard winsparkle icon will be used.
	bool iconLoaded = false;
	HMODULE hParentExe = GetModuleHandle(NULL);
	if (hParentExe)
	{
		LPTSTR iconName = 0;
		EnumResourceNames(hParentExe, RT_GROUP_ICON, GetFirstIconProc, (LONG_PTR)&iconName);
		if (GetLastError() == ERROR_SUCCESS || GetLastError() == ERROR_RESOURCE_ENUM_USER_STOP)
		{
			HANDLE hIcon = LoadImage(hParentExe, iconName, IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
			if (hIcon)
			{
				icon.SetHICON(hIcon);
				icon.SetWidth(48);
				icon.SetHeight(48);
				iconLoaded = true;
			}
			if (!IS_INTRESOURCE(iconName))
				free(iconName);
		}
	}
	if (!iconLoaded)
		icon.LoadFile("UpdateAvailable", wxBITMAP_TYPE_ICO_RESOURCE, 48, 48);
	return iconLoaded;
}

wxString Resources::LoadString( unsigned int id )
{
	wchar_t *pBuf = NULL;
	int len = ::LoadStringW(UI::GetDllHINSTANCE(), id, reinterpret_cast<LPWSTR>(&pBuf), 0);
	if (len)
		return wxString(pBuf, len);
	else
		return wxString();
}

BOOL CALLBACK Resources::GetFirstIconProc(HMODULE hModule, LPCTSTR lpszType,
	LPTSTR lpszName, LONG_PTR lParam)
{
	if (IS_INTRESOURCE(lpszName))
		*((LPTSTR*)lParam) = lpszName;
	else
		*((LPTSTR*)lParam) = _tcsdup(lpszName);
	return FALSE; // stop on the first icon found
}

wxString Resources::GetExeFileName()
{
	wchar_t buffer[MAX_PATH];
	buffer[0] = 0;
	::GetModuleFileName(NULL, buffer, MAX_PATH);
	return wxString(buffer);
}

};
