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

#ifndef _updatechecker_h_
#define _updatechecker_h_

#include "threads.h"

#include <string>

namespace winsparkle
{

struct Appcast;

/**
    This class checks the appcast for updates.

    If an update is found, then UpdateChecker initializes the GUI thread
    and shows information about available update to the user.
 */
class UpdateChecker : public Thread
{
public:
    /// Creates checker thread.
    UpdateChecker();

    /**
        Compares versions @a a and @a b.

        The comparison is somewhat intelligent, it handles beta and RC
        components correctly.

        @return 0 if the versions are identical, negative value if
                @a a is smaller than @a b, positive value if @a a
                is larger than @a b.
     */
    static int CompareVersions(const std::string& a, const std::string& b);

protected:
    /// Returns flags to be used when checking the appcast
    virtual int GetAppcastDownloadFlags() const { return 0; }

    /// Should give version be ignored?
    virtual bool ShouldSkipUpdate(const Appcast& appcast) const;

    virtual void Run();
    virtual bool IsJoinable() const { return false; }

    virtual void NoUpdates() = 0;
    virtual void UpdateAvailable(const Appcast& appcast) = 0;
    virtual void UpdateError(int errorCode) = 0;

};



/**
    Update checker used for checking without UI
*/
class SilentUpdateChecker : public UpdateChecker
{
public:
    SilentUpdateChecker(void (*callback)(int, int))
        : UpdateChecker(), 
        m_updateCheckerCallback(callback),
        m_appcast(NULL)
    {}

    ~SilentUpdateChecker();

    Appcast GetAppcast();

protected:
    virtual void NoUpdates();
    virtual void UpdateAvailable(const Appcast& appcast);
    virtual void UpdateError(int errorCode);

private:
    void (*m_updateCheckerCallback)(int, int);
    Appcast* m_appcast;
};


/**
    Update checker used for checking with UI
*/
class UIUpdateChecker : public UpdateChecker
{
public:
    UIUpdateChecker() : UpdateChecker(){}

protected:
    virtual void NoUpdates();
    virtual void UpdateAvailable(const Appcast& appcast);
    virtual void UpdateError(int errorCode);
};


/**
    Update checker used for manual checking.
 */
class ManualUpdateChecker : public UIUpdateChecker
{
public:
    /// Creates checker thread.
    ManualUpdateChecker() : UIUpdateChecker() {}

protected:
    virtual int GetAppcastDownloadFlags() const;
    virtual bool ShouldSkipUpdate(const Appcast& appcast) const;
};

} // namespace winsparkle

#endif // _updatechecker_h_
