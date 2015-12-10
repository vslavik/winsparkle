/*
 *  This file is part of WinSparkle (http://winsparkle.org)
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

#ifndef _appcontroller_h_
#define _appcontroller_h_

#include "winsparkle.h"
#include "threads.h"


namespace winsparkle
{

/**
    Interface to controlling the hosting application.
 */
class ApplicationController
{
public:
    /**
        Control functions.
     */
    //@{

    /// Ask the application if it is ready to terminate itself.
    static bool IsReadyToShutdown();

    /// Tell the host to terminate.
    static void RequestShutdown();

    //@}

    /**
        Notifications.
     */
    //@{

    /// Notify that an error occurred.
    static void NotifyUpdateError();

    /// Notify that an update has been found.
    static void NotifyUpdateFound();

    /// Notify that an update has not been found.
    static void NotifyUpdateNotFound();

    /// Notify that an update was cancelled.
    static void NotifyUpdateCancelled();

    //@}

    /**
        Behavior customizations -- callbacks.
     */
    //@{

    /// Set the win_sparkle_error_callback_t function
    static void SetErrorCallback(win_sparkle_error_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbError = callback;
    }

    /// Set the win_sparkle_can_shutdown_callback_t function
    static void SetCanShutdownCallback(win_sparkle_can_shutdown_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbIsReadyToShutdown = callback;
    }

    /// Set the win_sparkle_shutdown_request_callback_t function
    static void SetShutdownRequestCallback(win_sparkle_shutdown_request_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbRequestShutdown = callback;
    }

    /// Set the win_sparkle_did_find_update_callback_t function
    static void SetDidFindUpdateCallback(win_sparkle_did_find_update_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbDidFindUpdate = callback;
    }

    /// Set the win_sparkle_did_not_find_update_callback_t function
    static void SetDidNotFindUpdateCallback(win_sparkle_did_not_find_update_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbDidNotFindUpdate = callback;
    }

    /// Set the win_sparkle_update_cancelled_callback_t function
    static void SetUpdateCancelledCallback(win_sparkle_update_cancelled_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbUpdateCancelled = callback;
    }

    //@}

private:
    ApplicationController(); // cannot be instantiated

    // guards the variables below:
    static CriticalSection ms_csVars;

    static win_sparkle_error_callback_t               ms_cbError;
    static win_sparkle_can_shutdown_callback_t        ms_cbIsReadyToShutdown;
    static win_sparkle_shutdown_request_callback_t    ms_cbRequestShutdown;
    static win_sparkle_did_find_update_callback_t     ms_cbDidFindUpdate;
    static win_sparkle_did_not_find_update_callback_t ms_cbDidNotFindUpdate;
    static win_sparkle_update_cancelled_callback_t    ms_cbUpdateCancelled;
};

} // namespace winsparkle

#endif // _appcontroller_h_
