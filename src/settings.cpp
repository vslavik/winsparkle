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
std::wstring Settings::GetVerInfoField(const wchar_t *field)
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
        throw Win32Exception("Executable doesn't have required key in StringFileInfo");
    }

    return value;
}


} // namespace winsparkle
