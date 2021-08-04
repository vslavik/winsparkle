/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2013 Vaclav Slavik
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

#include "appcontroller.h"
#include "appcast.h"


namespace winsparkle
{

CriticalSection ApplicationController::ms_csVars;

win_sparkle_error_callback_t               ApplicationController::ms_cbError = NULL;
win_sparkle_can_shutdown_callback_t        ApplicationController::ms_cbIsReadyToShutdown = NULL;
win_sparkle_shutdown_request_callback_t    ApplicationController::ms_cbRequestShutdown = NULL;
win_sparkle_did_find_update_callback_t     ApplicationController::ms_cbDidFindUpdate = NULL;
win_sparkle_did_not_find_update_callback_t ApplicationController::ms_cbDidNotFindUpdate = NULL;
win_sparkle_update_cancelled_callback_t    ApplicationController::ms_cbUpdateCancelled = NULL;
win_sparkle_update_skipped_callback_t      ApplicationController::ms_cbUpdateSkipped = NULL;
win_sparkle_update_postponed_callback_t    ApplicationController::ms_cbUpdatePostponed = NULL;
win_sparkle_update_dismissed_callback_t    ApplicationController::ms_cbUpdateDismissed = NULL;
win_sparkle_user_run_installer_callback_t  ApplicationController::ms_cbUserRunInstaller = NULL;
win_sparkle_alternate_appcast_callback_t   ApplicationController::ms_cbAlternateAppcast = NULL;

bool ApplicationController::IsReadyToShutdown()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbIsReadyToShutdown )
            return (*ms_cbIsReadyToShutdown)() == 0 ? false : true;
    }

    // default implementations:

    return true;
}

void ApplicationController::RequestShutdown()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbRequestShutdown )
        {
            (*ms_cbRequestShutdown)();
            return;
        }
    }

    // default implementations:

    // nothing yet
}

void ApplicationController::NotifyUpdateError()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbError )
        {
            (*ms_cbError)();
            return;
        }
    }
}

void ApplicationController::NotifyUpdateFound()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if (ms_cbDidFindUpdate)
        {
            (*ms_cbDidFindUpdate)();
            return;
        }
    }
}

void ApplicationController::NotifyUpdateNotFound()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbDidNotFindUpdate )
        {
            (*ms_cbDidNotFindUpdate)();
            return;
        }
    }
}

void ApplicationController::NotifyUpdateCancelled()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbUpdateCancelled )
        {
            (*ms_cbUpdateCancelled)();
            return;
        }
    }
}

void ApplicationController::NotifyUpdateSkipped()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if (ms_cbUpdateSkipped)
        {
            (*ms_cbUpdateSkipped)();
            return;
        }
    }
}

void ApplicationController::NotifyUpdatePostponed()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if (ms_cbUpdatePostponed)
        {
            (*ms_cbUpdatePostponed)();
            return;
        }
    }
}

void ApplicationController::NotifyUpdateDismissed()
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if (ms_cbUpdateDismissed)
        {
            (*ms_cbUpdateDismissed)();
            return;
        }
    }
}


int ApplicationController::UserRunInstallerCallback(const wchar_t* filePath)
{
    if (!ms_cbUserRunInstaller)
        return false;

    return ms_cbUserRunInstaller(filePath);
}

int ApplicationController::AlternateAppcastCallback(bool manual, struct Appcast& appcast)
{
    // Get the alternate appcast data (use the user defined callback) if one is defined
    if (!ms_cbAlternateAppcast)
        return 0;

    // Use temporary storage for receiving the data (to be populated by the callback)
    // The DLL <-> App interface does not allow for (easy) passing (marshalling) of
    // dynamic objects like std::string, so the implementation is limited to passing
    // pointers to predefined size char arrays that need to be defined large enough
    // to store representations of version strings, URLs and some arbitrary strings.
    // The size of the arrays is also passed along to the App as a parameter to be able
    // to prevent buffer overruns. This of course assumes that the size parameter is
    // checked for and considered in the callback.
    static const int appcastBufferLength = 500;

    char version[appcastBufferLength];
    char downloadUrl[appcastBufferLength];
    char releaseNotesUrl[appcastBufferLength];
    char webBrowserUrl[appcastBufferLength];
    char title[appcastBufferLength];
    char description[appcastBufferLength];

    memset(version, 0x00, appcastBufferLength);
    memset(downloadUrl, 0x00, appcastBufferLength);
    memset(releaseNotesUrl, 0x00, appcastBufferLength);
    memset(webBrowserUrl, 0x00, appcastBufferLength);
    memset(title, 0x00, appcastBufferLength);
    memset(description, 0x00, appcastBufferLength);

    // Actually get the alternate appcast data (execute the user defined callback)
    int retVal = ms_cbAlternateAppcast(manual, appcastBufferLength - 1, version, downloadUrl, releaseNotesUrl, webBrowserUrl, title, description);

    if (retVal == 1)
    {
        // Update is available, marshall the data to our space
        appcast.Version = version;
        appcast.DownloadURL = downloadUrl;
        appcast.ReleaseNotesURL = releaseNotesUrl;
        appcast.WebBrowserURL = webBrowserUrl;
        appcast.Title = title;
        appcast.Description = description;
    }

    return retVal;
}

} // namespace winsparkle
