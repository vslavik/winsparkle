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

#include "winsparkle.h"

#include "settings.h"
#include "error.h"
#include "ui.h"
#include "updatechecker.h"

using namespace winsparkle;

extern "C"
{

/*--------------------------------------------------------------------------*
                       Initialization and shutdown
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void win_sparkle_init()
{
    try
    {
        bool checkUpdates;
        if ( Settings::ReadConfigValue("CheckForUpdates", checkUpdates) )
        {
            if ( checkUpdates )
            {
                // Run the check in background. Only show UI if updates are
                // available.
                UpdateChecker *check = new UpdateChecker();
                check->Start();
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


WIN_SPARKLE_API void win_sparkle_cleanup()
{
    try
    {
        UI::ShutDown();

        // FIXME: shut down any worker UpdateChecker threads too
    }
    CATCH_ALL_EXCEPTIONS
}


/*--------------------------------------------------------------------------*
                               Configuration
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void win_sparkle_set_appcast_url(const char *url)
{
    try
    {
        Settings::SetAppcastURL(url);
    }
    CATCH_ALL_EXCEPTIONS
}


/*--------------------------------------------------------------------------*
                              Manual usage
 *--------------------------------------------------------------------------*/

WIN_SPARKLE_API void win_sparkle_check_update_with_ui()
{
    try
    {
        // Initialize UI thread and show progress indicator.
        UI::ShowCheckingUpdates();

        // Then run the actual check in the background.
        UpdateChecker *check = new UpdateChecker();
        check->Start();
    }
    CATCH_ALL_EXCEPTIONS
}

} // extern "C"
