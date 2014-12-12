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

#include "appcontroller.h"


namespace winsparkle
{

Appcast ApplicationController::m_appcast;

CriticalSection ApplicationController::ms_csVars;

win_sparkle_can_shutdown_callback_t     ApplicationController::ms_cbIsReadyToShutdown = NULL;
win_sparkle_shutdown_request_callback_t ApplicationController::ms_cbRequestShutdown = NULL;
win_sparkle_check_update_callback_t ApplicationController::ms_cbCheckUpdate = NULL;
win_sparkle_download_progress_callback_t ApplicationController::ms_cbDownloadProgress = NULL;


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

void ApplicationController::CheckUpdateResult(int result)
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbCheckUpdate )
        {
            (*ms_cbCheckUpdate)(result);
            return;
        }
    }

    // default implementations:

    // nothing yet
}

void ApplicationController::ReportDownloadProgress(unsigned int progress, unsigned int total)
{
    {
        CriticalSectionLocker lock(ms_csVars);
        if ( ms_cbDownloadProgress )
        {
            (*ms_cbDownloadProgress)(progress, total);
            return;
        }
    }

    // default implementations:

    // nothing yet
}


} // namespace winsparkle
