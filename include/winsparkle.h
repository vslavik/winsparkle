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

#ifndef _winsparkle_h_
#define _winsparkle_h_

#include "winsparkle-version.h"

#if !BUILDING_WIN_SPARKLE && defined(_MSC_VER)
#pragma comment(lib, "WinSparkle.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_WIN_SPARKLE
    #define WIN_SPARKLE_API __declspec(dllexport)
#else
    #define WIN_SPARKLE_API __declspec(dllimport)
#endif

/*--------------------------------------------------------------------------*
                       Initialization and shutdown
 *--------------------------------------------------------------------------*/

/**
    @name Initialization functions
 */
//@{

/**
    Starts WinSparkle.

    If WinSparkle is configured to check for updates on startup, proceeds
    to perform the check. You should only call this function when your app
    is initialized and shows its main window.

    @see win_sparkle_cleanup()
 */
WIN_SPARKLE_API void win_sparkle_init();

/**
    Cleans up after WinSparkle.

    Should be called by the app when it's shutting down. Cancels any
    pending Sparkle operations and shuts down its helper threads.
 */
WIN_SPARKLE_API void win_sparkle_cleanup();

//@}


/*--------------------------------------------------------------------------*
                               Configuration
 *--------------------------------------------------------------------------*/

/**
    @name Configuration functions

    Functions for setting up WinSparkle.

    All functions in this category can only be called @em before the first
    call to win_sparkle_init()! Additionally, they aren't MT-safe and cannot
    be called in parallel.

    Typically, the application would configure WinSparkle on startup and then
    call win_sparkle_init(), all from its main thread.
 */
//@{

/**
    Sets URL for the app's appcast.

    Only http and https schemes are supported.

    Currently, this function must be called (@todo - get it from resources).

    @param url  URL of the appcast.
 */
WIN_SPARKLE_API void win_sparkle_set_appcast_url(const char *url);

//@}


/*--------------------------------------------------------------------------*
                              Manual usage
 *--------------------------------------------------------------------------*/

/**
    @name Manually using WinSparkle
 */
//@{

/**
    Checks if an update is available, showing progress UI to the user.

    Normally, WinSparkle checks for updates on startup and only shows its UI
    when it finds an update. If the application disables this behavior, it
    can hook this function to "Check for updates..." menu item.

    When called, background thread is started to check for updates. A small
    window is shown to let the user know the progress. If no update is found,
    the user is told so. If there is an update, the usual "update available"
    window is shown.

    This function returns immediately.
 */
WIN_SPARKLE_API void win_sparkle_check_update_with_ui();

//@}

#ifdef __cplusplus
}
#endif

#endif // _winsparkle_h_
