/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2013 Vaclav Slavik
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

    /// Title of the update
    std::string Title;

    /// Description of the update
    std::string Description;

    /**
        Initializes the struct with data from XML appcast feed.

        If the feed contains multiple entries, only the latest one is read,
        the rest is ignored.

        Throws on error.

        @param xml Appcast feed data.
     */
    void Load(const std::string& xml);
};

} // namespace winsparkle

#endif // _appcast_h_
