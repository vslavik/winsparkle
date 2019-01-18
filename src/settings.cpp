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

#include "settings.h"

#include "error.h"
#include "utils.h"
#include "threads.h"
#include "signatureverifier.h"


namespace winsparkle
{

CriticalSection Settings::ms_csVars;
Settings::Lang Settings::ms_lang;
std::string  Settings::ms_appcastURL;
std::string  Settings::ms_registryPath;
std::wstring Settings::ms_companyName;
std::wstring Settings::ms_appName;
std::wstring Settings::ms_appVersion;
std::wstring Settings::ms_appBuildVersion;
std::string Settings::ms_DSAPubKey;


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


std::wstring Settings::DoGetVerInfoField(const wchar_t *field, bool fatal)
{
    TCHAR exeFilename[MAX_PATH + 1];

    if ( !GetModuleFileName(NULL, exeFilename, MAX_PATH) )
        throw Win32Exception();

    DWORD unusedHandle;
    DWORD fiSize = GetFileVersionInfoSize(exeFilename, &unusedHandle);
    if ( fiSize == 0 )
        throw Win32Exception("Executable doesn't have the required VERSIONINFO resource");

    DataBuffer<unsigned char> fi(fiSize);

    if ( !GetFileVersionInfo(exeFilename, unusedHandle, fiSize, fi.data) )
        throw Win32Exception();

    const std::wstring key =
        TEXT("\\StringFileInfo\\") + GetVerInfoLang(fi.data) + TEXT("\\") + field;
    LPTSTR key_str = (LPTSTR)key.c_str(); // explicit cast to work around VC2005 bug

    TCHAR *value;
    UINT len;
    if ( !VerQueryValue(fi.data, key_str, (LPVOID*)&value, &len) )
    {
        if ( fatal )
            throw Win32Exception("Executable doesn't have required key in StringFileInfo");
        else
            return std::wstring();
    }

    return value;
}


std::string Settings::GetCustomResource(const char *name, const char *type)
{
    const HINSTANCE module = 0; // main executable
    HRSRC hRes = FindResourceA(module, name, type);
    if ( hRes )
    {
        HGLOBAL hData = LoadResource(module, hRes);
        if ( hData )
        {
            const char *data = (const char*)::LockResource(hData);
            size_t size = ::SizeofResource(module, hRes);

            if ( data && size )
            {
                if ( data[size-1] == '\0' ) // null-terminated string
                    size--;
                return std::string(data, size);
            }
        }
    }

    std::string err;
    err += "Failed to get resource \"";
    err += name;
    err += "\" (type \"";
    err += type;
    err += "\")";
    throw Win32Exception(err.c_str());
}


/*--------------------------------------------------------------------------*
                             runtime config access
 *--------------------------------------------------------------------------*/

std::string Settings::GetDefaultRegistryPath()
{
    std::string s("Software\\");
    std::wstring vendor = Settings::GetCompanyName();
    if ( !vendor.empty() )
        s += WideToAnsi(vendor) + "\\";
    s += WideToAnsi(Settings::GetAppName());
    s += "\\WinSparkle";

    return s;
}

namespace
{

void RegistryWrite(const char *name, const wchar_t *value)
{
    const std::string subkey = Settings::GetRegistryPath();

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

    result = RegSetValueEx
             (
                 key,
                 AnsiToWide(name).c_str(),
                 0,
                 REG_SZ,
                 (const BYTE*)value,
                 (DWORD)((wcslen(value) + 1) * sizeof(wchar_t))
             );

    RegCloseKey(key);

    if ( result != ERROR_SUCCESS )
        throw Win32Exception("Cannot write settings to registry");
}


void RegistryDelete(const char *name)
{
    const std::string subkey = Settings::GetRegistryPath();

    HKEY key;
    LONG result = RegOpenKeyExA
                  (
                      HKEY_CURRENT_USER,
                      subkey.c_str(),
                      0,
                      KEY_SET_VALUE,
                      &key
                  );
    if ( result != ERROR_SUCCESS )
        throw Win32Exception("Cannot delete settings from registry");

    result = RegDeleteValueA(key, name);

    RegCloseKey(key);

    if ( result != ERROR_SUCCESS )
        throw Win32Exception("Cannot delete settings from registry");
}


int DoRegistryRead(HKEY root, const char *name, wchar_t *buf, size_t len)
{
    const std::string subkey = Settings::GetRegistryPath();

    HKEY key;
    LONG result = RegOpenKeyExA
                  (
                      root,
                      subkey.c_str(),
                      0,
                      KEY_QUERY_VALUE,
                      &key
                  );
    if ( result != ERROR_SUCCESS )
    {
        if ( result == ERROR_FILE_NOT_FOUND )
            return 0;
        throw Win32Exception("Cannot read settings from registry");
    }

    DWORD buflen = (DWORD)len;
    DWORD type;
    result = RegQueryValueEx
             (
                 key,
                 AnsiToWide(name).c_str(),
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
        throw Win32Exception("Cannot read settings from registry");
    }

    if ( type != REG_SZ )
    {
        // incorrect type -- pretend that the setting doesn't exist, it will
        // be newly written by WinSparkle anyway
        return 0;
    }

    return 1;
}


int RegistryRead(const char *name, wchar_t *buf, size_t len)
{
    // Try reading from HKCU first. If that fails, look at HKLM too, in case
    // some settings have globally set values (either by the installer or the
    // administrator).
    if ( DoRegistryRead(HKEY_CURRENT_USER, name, buf, len) )
    {
        return 1;
    }
    else
    {
        return DoRegistryRead(HKEY_LOCAL_MACHINE, name, buf, len);
    }
}

// Critical section to guard DoWriteConfigValue/DoReadConfigValue.
CriticalSection g_csConfigValues;

} // anonymous namespace


void Settings::DoWriteConfigValue(const char *name, const wchar_t *value)
{
    CriticalSectionLocker lock(g_csConfigValues);

    RegistryWrite(name, value);
}


std::wstring Settings::DoReadConfigValue(const char *name)
{
    CriticalSectionLocker lock(g_csConfigValues);

    wchar_t buf[512];
    if ( RegistryRead(name, buf, sizeof(buf)) )
        return buf;
    else
        return std::wstring();
}

void Settings::DeleteConfigValue(const char *name)
{
    CriticalSectionLocker lock(g_csConfigValues);

    RegistryDelete(name);
}

void Settings::SetDSAPubKeyPem(const std::string &pem)
{
    CriticalSectionLocker lock(ms_csVars);
    SignatureVerifier::VerifyDSAPubKeyPem(pem);
    ms_DSAPubKey = pem;
}

} // namespace winsparkle
