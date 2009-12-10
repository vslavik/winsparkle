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

/*--------------------------------------------------------------------------*
                         Version information
 *--------------------------------------------------------------------------*/

#define WIN_SPARKLE_VERSION_MAJOR   0
#define WIN_SPARKLE_VERSION_MINOR   1

/**
    Checks if WinSparkle version is at least @a major.@a minor.
 */
#define WIN_SPARKLE_VERSION(major, minor)                        \
            (WIN_SPARKLE_VERSION_MAJOR > (major) ||              \
               (WIN_SPARKLE_VERSION_MAJOR == (major) &&          \
                WIN_SPARKLE_VERSION_MINOR >= (minor)))


#ifdef BUILDING_WIN_SPARKLE
    #define WIN_SPARKLE_API extern "C" __declspec(dllexport)
#else
    #define WIN_SPARKLE_API extern "C" __declspec(dllimport)
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

#endif // _winsparkle_h_
