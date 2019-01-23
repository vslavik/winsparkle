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

#ifndef _settings_h_
#define _settings_h_

#include "threads.h"
#include "utils.h"

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
    /**
        Getting app metadata.
     */
    //@{

    /// Get location of the appcast
    static std::string GetAppcastURL()
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_appcastURL.empty() )
            ms_appcastURL = GetCustomResource("FeedURL", "APPCAST");
        return ms_appcastURL;
    }

    /// Return application name
    static std::wstring GetAppName()
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_appName.empty() )
            ms_appName = GetVerInfoField(L"ProductName");
        return ms_appName;
    }

    /// Return (human-readable) application version
    static std::wstring GetAppVersion()
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_appVersion.empty() )
            ms_appVersion = GetVerInfoField(L"ProductVersion");
        return ms_appVersion;
    }

    /// Return (internal) application build version
    static std::wstring GetAppBuildVersion()
    {
        {
            CriticalSectionLocker lock(ms_csVars);
            if ( !ms_appBuildVersion.empty() )
                return ms_appBuildVersion;
        }
        // fallback if build number wasn't set:
        return GetAppVersion();
    }

    /// Return name of the vendor
    static std::wstring GetCompanyName()
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_companyName.empty() )
            ms_companyName = GetVerInfoField(L"CompanyName");
        return ms_companyName;
    }

    /// Return the registry path to store settings in
    static std::string GetRegistryPath()
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_registryPath.empty() )
            ms_registryPath = GetDefaultRegistryPath();
        return ms_registryPath;
    }

    /// Return DSA public key to verify update file signature
    static const std::string &GetDSAPubKeyPem()
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_DSAPubKey.empty() )
            ms_DSAPubKey = GetCustomResource("DSAPub", "DSAPEM");
        return ms_DSAPubKey;
    }

    /// Return true if DSA public key is available
    static bool HasDSAPubKeyPem()
    {
        try
        {
            return !GetDSAPubKeyPem().empty();
        }
        CATCH_ALL_EXCEPTIONS
        return false;
    }

    //@}

    /**
        UI language.
    */
    //@{

    struct Lang
    {
        Lang() : langid(0) {}
        bool IsOk() const { return !lang.empty() || langid != 0; }

        std::string lang;
        unsigned short langid;
    };

    static Lang GetLanguage()
    {
        CriticalSectionLocker lock(ms_csVars);
        return ms_lang;
    }

    static void SetLanguage(const char *lang)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_lang.lang = lang;
    }
    
    static void SetLanguage(unsigned short langid)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_lang.langid = langid;
    }

    //@}

    /**
        Overwriting app metadata.

        Normally, they would be retrieved from resources, but it's sometimes
        necessary to set them manually.
     */
    //@{

    /// Set appcast location
    static void SetAppcastURL(const char *url)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_appcastURL = url;
    }

    /// Set application name
    static void SetAppName(const wchar_t *name)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_appName = name;
    }

    /// Set application version
    static void SetAppVersion(const wchar_t *version)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_appVersion = version;
    }

    /// Set application's build version number
    static void SetAppBuildVersion(const wchar_t *version)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_appBuildVersion = version;
    }

    /// Set company name
    static void SetCompanyName(const wchar_t *name)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_companyName = name;
    }

    /// Set Windows registry path to store settings in (relative to HKCU/KHLM).
    static void SetRegistryPath(const char *path)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_registryPath = path;
    }

    /// Set PEM data and verify in contains valid DSA public key
    static void SetDSAPubKeyPem(const std::string &pem);
    //@}


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
        std::wostringstream s;
        s << value;
        DoWriteConfigValue(name, s.str().c_str());
    }

    static void WriteConfigValue(const char *name, const std::string& value)
    {
        DoWriteConfigValue(name, AnsiToWide(value).c_str());
    }

    static void WriteConfigValue(const char *name, const std::wstring& value)
    {
        DoWriteConfigValue(name, value.c_str());
    }

    // Reads a value from registry. Returns true if it was present,
    // false otherwise.
    template<typename T>
    static bool ReadConfigValue(const char *name, T& value)
    {
        const std::wstring v = DoReadConfigValue(name);
        if ( v.empty() )
            return false;
        std::wistringstream s(v);
        s >> value;
        return !s.fail();
    }

    static bool ReadConfigValue(const char *name, std::string& value)
    {
        const std::wstring v = DoReadConfigValue(name);
        if (v.empty())
            return false;
        value = WideToAnsi(v);
        return true;
    }

    static bool ReadConfigValue(const char *name, std::wstring& value)
    {
        value = DoReadConfigValue(name);
        return !value.empty();
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

    // Deletes value from registry.
    static void DeleteConfigValue(const char *name);

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
    // Gets custom win32 resource data
    static std::string GetCustomResource(const char *name, const char *type);

    static std::string GetDefaultRegistryPath();

    static void DoWriteConfigValue(const char *name, const wchar_t *value);
    static std::wstring DoReadConfigValue(const char *name);

private:
    // guards the variables below:
    static CriticalSection ms_csVars;

    static Lang         ms_lang;
    static std::string  ms_appcastURL;
    static std::string  ms_registryPath;
    static std::wstring ms_companyName;
    static std::wstring ms_appName;
    static std::wstring ms_appVersion;
    static std::wstring ms_appBuildVersion;
    static std::string  ms_DSAPubKey;
};

} // namespace winsparkle

#endif // _settings_h_
