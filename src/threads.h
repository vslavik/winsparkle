/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2020 Vaclav Slavik
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

#ifndef _threads_h_
#define _threads_h_

#include "error.h"

#include <windows.h>

namespace winsparkle
{

/// C++ wrapper for win32 event
class Event
{
public:
    Event()
    {
        m_handle = CreateEvent
                   (
                       NULL,  // default security attributes
                       FALSE, // = is auto-resetting
                       FALSE, // initially non-signaled
                       NULL   // anonymous
                   );
        if (!m_handle)
            throw Win32Exception();
    }

    ~Event() { CloseHandle(m_handle); }

    /// Signal the event
    void Signal()
    {
        SetEvent(m_handle);
    }

    /// Wait until the event is signalled (true) or timeout ellapses (false)
    bool WaitUntilSignaled(unsigned timeoutMilliseconds = INFINITE)
    {
        return WaitForSingleObject(m_handle, timeoutMilliseconds) == WAIT_OBJECT_0;
    }

    bool CheckIfSignaled()
    {
        return WaitUntilSignaled(0);
    }

private:
    HANDLE m_handle;
};


/**
C++ wrapper for win32 critical section object.
*/
class CriticalSection
{
public:
    CriticalSection() { InitializeCriticalSection(&m_cs); }
    ~CriticalSection() { DeleteCriticalSection(&m_cs); }

    void Enter() { EnterCriticalSection(&m_cs); }
    void Leave() { LeaveCriticalSection(&m_cs); }

private:
    CRITICAL_SECTION m_cs;
};


/**
Locks a critical section as RIIA. Use this instead of manually calling
CriticalSection::Enter() and CriticalSection::Leave().
*/
class CriticalSectionLocker
{
public:
    CriticalSectionLocker(CriticalSection& cs) : m_cs(cs) { cs.Enter(); }
    ~CriticalSectionLocker() { m_cs.Leave(); }

private:
    CriticalSection& m_cs;
};


/**
    Lightweight thread class.

    It's a base class; derived class must implement at least Run().

    Create the thread on heap, then call Start() on it. If the thread is joinable
    (see IsJoinable()), then you must call TerminateAndJoin() on it to destroy it.
    Otherwise, it self-destructs.
 */
class Thread
{
public:
    /**
        Creates the thread.

        Note that you must explicitly call Start() to start it.

        @param name Descriptive name of the thread. This is shown in (Visual C++)
                    debugger and should always be set to something meaningful to
                    help identify WinSparkle threads.
     */
    Thread(const char *name);

    virtual ~Thread();

    /**
        Launch the thread.

        Calls Run() in the new thread's context.

        This method doesn't return until Run() calls SignalReady().

        Throws on error.
     */
    void Start();

    /**
        Wait for the thread to terminate.
     */
    void Join();

    /**
        Signal the thread to terminate and call Join().

        @note The thread must support this and call
              CheckShouldTerminate() frequently.
     */
    void TerminateAndJoin();
    
    /// Check if the thread should terminate and throw TerminateThreadException if so.
    void CheckShouldTerminate();

protected:
    /// Signals Start() that the thread is up and ready.
    void SignalReady();

    /**
        Code to run in the thread's context.

        This method @a must call SignalReady() as soon as it is initialized.
     */
    virtual void Run() = 0;

    /// Is the thread joinable?
    virtual bool IsJoinable() const = 0;

    /// This exception is thrown when the thread was terminated.
    struct TerminateThreadException
    {
    };

private:
    static unsigned __stdcall ThreadEntryPoint(void *data);

protected:
    HANDLE m_handle;
    unsigned m_id;
    Event m_signalEvent, m_terminateEvent;
};

} // namespace winsparkle

#endif // _threads_h_
