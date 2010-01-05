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

#include "settings.h"

#include "error.h"
#include "utils.h"


namespace winsparkle
{


Settings Settings::ms_instance;


Settings& Settings::Get()
{
    return ms_instance;
}


/*--------------------------------------------------------------------------*
                             resources access
 *--------------------------------------------------------------------------*/

namespace
{

struct TranslationInfo
{
    WORD wLanguage;
    WORD wCodePage;
};

// return language code for the resource to query
std::wstring GetVerInfoLang(void *fi)
{
    struct TranslationInfo
    {
        WORD language;
        WORD codepage;
    } *translations;
    unsigned translationsCnt;

    if ( !VerQueryValue(fi, TEXT("\\VarFileInfo\\Translation"),
                        (LPVOID*)&translations, &translationsCnt) )
        throw Win32Exception("Executable doesn't have required VERSIONINFO\\VarFileInfo resource");

    translationsCnt /= sizeof(struct TranslationInfo);
    if ( translationsCnt == 0 )
        throw std::runtime_error("No translations in VarFileInfo resource?");

    // TODO: be smarter about which language to use:
    //       1. use app main thread's locale
    //       2. failing that, try language-neutral or English
    //       3. use the first one as fallback
    const size_t idx = 0;

    wchar_t lang[9];
    HRESULT hr = _snwprintf_s(lang, 9, 8,
                              L"%04x%04x",
                              translations[idx].language,
                              translations[idx].codepage);
    if ( FAILED(hr) )
        throw Win32Exception();

    return lang;
}

} // anonymous namespace


/* static */
std::wstring Settings::DoGetVerInfoField(const wchar_t *field, bool fatal)
{
    TCHAR exeFilename[MAX_PATH + 1];

    if ( !GetModuleFileName(NULL, exeFilename, MAX_PATH) )
        throw Win32Exception();

    DWORD unusedHandle;
    DWORD fiSize = GetFileVersionInfoSize(exeFilename, &unusedHandle);
    if ( fiSize == 0 )
        throw Win32Exception("Executable doesn't have the required VERSIONINFO resource");

    DataBuffer fi(fiSize);

    if ( !GetFileVersionInfo(exeFilename, unusedHandle, fiSize, fi.data) )
        throw Win32Exception();

    const std::wstring key =
        TEXT("\\StringFileInfo\\") + GetVerInfoLang(fi.data) + TEXT("\\") + field;

    TCHAR *value;
    UINT len;
    if ( !VerQueryValue(fi.data, key.c_str(), (LPVOID*)&value, &len) )
    {
        if ( fatal )
            throw Win32Exception("Executable doesn't have required key in StringFileInfo");
        else
            return std::wstring();
    }

    return value;
}


/*--------------------------------------------------------------------------*
                             runtime config access
 *--------------------------------------------------------------------------*/

namespace
{

std::string MakeSubKey(const char *name)
{
    std::string s("Software\\");

    std::wstring vendor = Settings::Get().GetCompanyName();
    if ( !vendor.empty() )
        s += WideToAnsi(vendor) + "\\";
    s += WideToAnsi(Settings::Get().GetAppName());
    s += "\\WinSparkle";

    return s;
}


void RegistryWrite(const char *name, const char *value)
{
    const std::string subkey = MakeSubKey(name);

    HKEY key;
    LONG result = RegCreateKeyExA
                  (
                      HKEY_CURRENT_USER,
                      subkey.c_str(),
                      0,
                      NULL,
                      REG_OPTION_NON_VOLATILE,
                      KEY_SET_VALUE,
                      NULL,
                      &key,
                      NULL
                  );
    if ( result != ERROR_SUCCESS )
        throw Win32Exception("Cannot write settings to registry");

    result = RegSetValueExA
             (
                 key,
                 name,
                 0,
                 REG_SZ,
                 (const BYTE*)value,
                 strlen(value) + 1
             );
    if ( result != ERROR_SUCCESS )
        throw Win32Exception("Cannot write settings to registry");

    RegCloseKey(key);
}


int RegistryRead(const char *name, char *buf, size_t len)
{
    const std::string subkey = MakeSubKey(name);

    HKEY key;
    LONG result = RegOpenKeyExA
                  (
                      HKEY_CURRENT_USER,
                      subkey.c_str(),
                      0,
                      KEY_QUERY_VALUE,
                      &key
                  );
    if ( result != ERROR_SUCCESS )
    {
        if ( result == ERROR_FILE_NOT_FOUND )
            return 0;
        throw Win32Exception("Cannot write settings to registry");
    }

    DWORD buflen = len;
    DWORD type;
    result = RegQueryValueExA
             (
                 key,
                 name,
                 0,
                 &type,
                 (BYTE*)buf,
                 &buflen
             );

    RegCloseKey(key);

    if ( result != ERROR_SUCCESS )
    {
        if ( result == ERROR_FILE_NOT_FOUND )
            return 0;
        throw Win32Exception("Cannot write settings to registry");
    }

    if ( type != REG_SZ )
    {
        // incorrect type -- pretend that the setting doesn't exist, it will
        // be newly written by WinSparkle anyway
        return 0;
    }

    return 1;
}

} // anonymous namespace


void Settings::DoWriteConfigValue(const char *name, const char *value)
{
    RegistryWrite(name, value);
}


std::string Settings::DoReadConfigValue(const char *name) const
{
    char buf[512];
    if ( RegistryRead(name, buf, sizeof(buf)) )
        return buf;
    else
        return std::string();
}

} // namespace winsparkle
