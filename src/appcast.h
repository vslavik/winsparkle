/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2016 Vaclav Slavik
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

#ifndef _appcast_h_
#define _appcast_h_

#include <string>

namespace winsparkle
{

/**
    This class contains information from the appcast.
 */
struct Appcast
{
    /// App version fields
    std::string Version;
    std::string ShortVersionString;

    /// URL of the update
    std::string DownloadURL;

    /// URL of the release notes page
    std::string ReleaseNotesURL;

    /// Local Info�� like en, en-US, zh-CN, zh-TW, etc.
    std::string Lang;

    /// URL to launch in web browser (instead of downloading update ourselves)
    std::string WebBrowserURL;

    /// Title of the update
    std::string Title;

    /// Description of the update
    std::string Description;

    // Operating system
    std::string Os;

    // Minimum OS version required for update
    std::string MinOSVersion;

    // Arguments passed on the the updater executable
    std::string InstallerArguments;

    /**
        Initializes the struct with data from XML appcast feed.

        If the feed contains multiple entries, only the latest one is read,
        the rest is ignored. Entries that are not appliable (e.g. for different
        OS) are likewise skipped.

        Throws on error.
        Returns NULL if no error ocurred, but there was no update in the appcast.

        @param xml Appcast feed data.
     */
    static Appcast Load(const std::string& xml, const std::string& lang);

    /// Returns true if the struct constains valid data.
    bool IsValid() const { return !Version.empty(); }

    /// If true, then download and install the update ourselves.
    /// If false, launch a web browser to WebBrowserURL.
    bool HasDownload() const { return !DownloadURL.empty(); }
};

} // namespace winsparkle

#endif // _appcast_h_
