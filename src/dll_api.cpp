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

#include "winsparkle.h"

#include "settings.h"
#include "error.h"
#include "ui.h"
#include "updatechecker.h"
#include "updatedownloader.h"

#include <ctime>

using namespace winsparkle;

extern "C"
{

/*--------------------------------------------------------------------------*
                       Callbacks
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void __cdecl win_sparkle_set_callback(::callbackFunction func)
{
    UI::SetCallback(func);
}

/*--------------------------------------------------------------------------*
                       Initialization and shutdown
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void __cdecl win_sparkle_init()
{
    try
    {
        // first things first
        UpdateDownloader::CleanLeftovers();

        bool checkUpdates;
        if ( Settings::ReadConfigValue("CheckForUpdates", checkUpdates) )
        {
            if ( checkUpdates )
            {
                static const time_t ONE_DAY = 60*60*24;

                time_t lastCheck = 0;
                Settings::ReadConfigValue("LastCheckTime", lastCheck);
                const time_t currentTime = time(NULL);

                // Only check for updates in reasonable intervals:
                const int interval = win_sparkle_get_update_check_interval();
                if ( currentTime - lastCheck >= interval )
                {
                    // Run the check in background. Only show UI if updates
                    // are available.
                    UpdateChecker *check = new UpdateChecker();
                    check->Start();
                }
            }
        }
        else // not yet configured
        {
            bool didRunOnce;
            Settings::ReadConfigValue("DidRunOnce", didRunOnce, false);
            if ( !didRunOnce )
            {
                // Do nothing on the first execution of the app, for better
                // first-time impression.
                Settings::WriteConfigValue("DidRunOnce", true);
            }
            else
            {
                // Only when the app is launched for the second time, ask the
                // user for their permission to check for updates.
                UI::AskForPermission();
            }
        }
    }
    CATCH_ALL_EXCEPTIONS
}


WIN_SPARKLE_API void __cdecl win_sparkle_cleanup()
{
    try
    {
        UI::ShutDown();

        // FIXME: shut down any worker UpdateChecker and UpdateDownloader threads too
    }
    CATCH_ALL_EXCEPTIONS
}


/*--------------------------------------------------------------------------*
                               Configuration
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void __cdecl win_sparkle_set_appcast_url(const char *url)
{
    try
    {
        Settings::SetAppcastURL(url);
    }
    CATCH_ALL_EXCEPTIONS
}

WIN_SPARKLE_API void __cdecl win_sparkle_set_app_details(const wchar_t *company_name,
                                                         const wchar_t *app_name,
                                                         const wchar_t *app_version)
{
    try
    {
        Settings::SetCompanyName(company_name);
        Settings::SetAppName(app_name);
        Settings::SetAppVersion(app_version);
    }
    CATCH_ALL_EXCEPTIONS
}

WIN_SPARKLE_API void __cdecl win_sparkle_set_app_build_version(const wchar_t *build)
{
    try
    {
        Settings::SetAppBuildVersion(build);
    }
    CATCH_ALL_EXCEPTIONS
}

WIN_SPARKLE_API void __cdecl win_sparkle_set_registry_path(const char *path)
{
    try
    {
        Settings::SetRegistryPath(path);
    }
    CATCH_ALL_EXCEPTIONS
}

WIN_SPARKLE_API void __cdecl win_sparkle_set_automatic_check_for_updates(int state)
{
    try
    {
        Settings::WriteConfigValue("CheckForUpdates", state != 0);
    }
    CATCH_ALL_EXCEPTIONS
}

WIN_SPARKLE_API int __cdecl win_sparkle_get_automatic_check_for_updates()
{
    try
    {
        bool checkUpdates;
        if ( Settings::ReadConfigValue("CheckForUpdates", checkUpdates, false) )
        return checkUpdates ? 1 : 0;
    }
    CATCH_ALL_EXCEPTIONS

    return 0;
}

WIN_SPARKLE_API void __cdecl win_sparkle_set_update_check_interval(int interval)
{
    static const int MIN_CHECK_INTERVAL = 3600; // one hour

    try
    {
        if ( interval < MIN_CHECK_INTERVAL )
        {
            winsparkle::LogError("Invalid update interval (min: 3600 seconds)");
            interval = MIN_CHECK_INTERVAL;
        }

        Settings::WriteConfigValue("UpdateInterval", interval);
    }
    CATCH_ALL_EXCEPTIONS
}

WIN_SPARKLE_API int __cdecl win_sparkle_get_update_check_interval()
{
    static const int DEFAULT_CHECK_INTERVAL = 60*60*24; // one day

    try
    {
        int interval;
        Settings::ReadConfigValue("UpdateInterval", interval, DEFAULT_CHECK_INTERVAL);
        return interval;
    }
    CATCH_ALL_EXCEPTIONS

    return DEFAULT_CHECK_INTERVAL;
}


/*--------------------------------------------------------------------------*
                              Manual usage
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void __cdecl win_sparkle_check_update_with_ui()
{
    try
    {
        // Initialize UI thread and show progress indicator.
        UI::ShowCheckingUpdates();

        // Then run the actual check in the background.
        UpdateChecker *check = new ManualUpdateChecker();
        check->Start();
    }
    CATCH_ALL_EXCEPTIONS
}

} // extern "C"
