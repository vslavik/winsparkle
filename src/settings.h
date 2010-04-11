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

#ifndef _settings_h_
#define _settings_h_

#include "threads.h"

#include <string>
#include <sstream>


namespace winsparkle
{

/**
    Holds all of WinSparkle configuration.

    It is used both for storing explicitly set configuration values (e.g.
    using win_sparkle_set_appcast_url()) and for retrieving them from default
    locations (e.g. from resources or registry).

    Note that it is only allowed to modify the settings before the first
    call to win_sparkle_init().
 */
class Settings
{
public:
    /// Get location of the appcast
    static std::string GetAppcastURL()
    {
        CriticalSectionLocker lock(ms_csVars);
        return ms_appcastURL;
    }

    /// Return application name
    static std::wstring GetAppName()
        { return GetVerInfoField(L"ProductName"); }

    /// Return (human-readable) application version
    static std::wstring GetAppVersion()
        { return GetVerInfoField(L"ProductVersion"); }

    /// Return name of the vendor
    static std::wstring GetCompanyName()
        { return GetVerInfoField(L"CompanyName"); }

    /// Set appcast location
    static void SetAppcastURL(const char *url)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_appcastURL = url;
    }

    /**
        Access to runtime configuration.

        This is stored in registry, under HKCU\Software\...\...\WinSparkle,
        where the vendor and app names are determined from resources.
     */
    //@{

    // Writes given value to registry under this name.
    template<typename T>
    static void WriteConfigValue(const char *name, const T& value)
    {
        std::ostringstream s;
        s << value;
        DoWriteConfigValue(name, s.str().c_str());
    }

    // Reads a value from registry. Returns true if it was present,
    // false otherwise.
    template<typename T>
    static bool ReadConfigValue(const char *name, T& value)
    {
        const std::string v = DoReadConfigValue(name);
        if ( v.empty() )
            return false;
        std::istringstream s(v);
        s >> value;
        return !s.fail();
    }

    // Reads a value from registry, substituting default value if not present.
    template<typename T>
    static bool ReadConfigValue(const char *name, T& value, const T& defval)
    {
        bool rv = ReadConfigValue(name, value);
        if ( !rv )
            value = defval;
        return rv;
    }

    //@}

private:
    Settings(); // cannot be instantiated

    // Get given field from the VERSIONINFO/StringFileInfo resource,
    // throw on failure
    static std::wstring GetVerInfoField(const wchar_t *field)
        { return DoGetVerInfoField(field, true); }
    // Same, but don't throw if a field is not set
    static std::wstring TryGetVerInfoField(const wchar_t *field)
        { return DoGetVerInfoField(field, false); }
    static std::wstring DoGetVerInfoField(const wchar_t *field, bool fatal);

    static void DoWriteConfigValue(const char *name, const char *value);
    static std::string DoReadConfigValue(const char *name);

private:
    // guards the variables below:
    static CriticalSection ms_csVars;

    static std::string ms_appcastURL;
};

} // namespace winsparkle

#endif // _settings_h_
