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

#ifndef _ui_h_
#define _ui_h_

#include "winsparkle.h"
#include "threads.h"
#include "appcast.h"
#include <wx/stattext.h>

namespace winsparkle
{

/**
    The main UI thread.

    This is where any user interface (available updates, download progress
    etc.) are shown.

    This thread is only created when needed -- in most cases, it isn't. Once it
    is created, it runs indefinitely (without wasting CPU time -- it sleeps
    waiting for incoming messages).
 */
class UI : public Thread
{
public:
    /**
        Shuts the UI thread down.

        @note Currently, this may only be called once, from
              win_sparkle_cleanup().
     */
    static void ShutDown();

    /**
        Notifies the UI that no updates were found.

        If the UI thread is running and WinSparkle window is currently open
        (i.e. the user is manually checking for updates), "no updates found"
        message is shown. Otherwise, does nothing.
     */
    static void NotifyNoUpdates();

    /**
        Notifies the UI that there was an error retrieving updates.

        If the UI thread is running and WinSparkle window is currently open
        (i.e. the user is manually checking for updates), "update error"
        message is shown. Otherwise, does nothing.
     */
    static void NotifyUpdateError();

    /**
        Notifies the UI that a new version is available.

        If the UI thread isn't running yet, it will be launched.
     */
    static void NotifyUpdateAvailable(const Appcast& info);

    /**
        Notifies the UI about download progress.
     */
    static void NotifyDownloadProgress(size_t downloaded, size_t total);

    /**
        Notifies the UI that an update was downloaded.
     */
    static void NotifyUpdateDownloaded(const std::string& updateFile);

    /**
        Shows the WinSparkle window in "checking for updates..." state.
     */
    static void ShowCheckingUpdates();

    /**
        Shows the dialog asking user for permission to check for updates.
     */
    static void AskForPermission();

    /**
        Sets HINSTANCE of the DLL.

        Must be called on DLL initialization.
     */
    static void SetDllHINSTANCE(HINSTANCE h) { ms_hInstance = h; }

	    /// Set the win_sparkle_localized_string_callback_t function
    static void SetLocalizedStringCallback(win_sparkle_localized_string_callback_t callback)
    {
        CriticalSectionLocker lock(ms_csVars);
        ms_cbLocalizedString = callback;
    }

	static const wxString LocalizedString( const char *str );

protected:
    virtual void Run();
    virtual bool IsJoinable() const { return true; }

private:
    UI();

    static HINSTANCE ms_hInstance;
 
	// guards the variables below:
    static CriticalSection ms_csVars;

    static win_sparkle_localized_string_callback_t     ms_cbLocalizedString;

    friend class UIThreadAccess;
};

} // namespace winsparkle

#endif // _ui_h_
