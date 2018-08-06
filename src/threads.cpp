/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2018 Vaclav Slavik
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

#include "threads.h"

#include <windows.h>
#include <process.h>

namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                 Helpers
 *--------------------------------------------------------------------------*/

namespace
{

// Sets thread's name for the debugger
void SetThreadName(DWORD threadId, const char *name)
{
#ifdef _MSC_VER
    // This code is copied verbatim from MSDN, see
    // http://msdn.microsoft.com/en-us/library/xcb2z8hs%28VS.100%29.aspx

    #define MS_VC_EXCEPTION 0x406D1388

    #pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
       DWORD dwType; // Must be 0x1000.
       LPCSTR szName; // Pointer to name (in user addr space).
       DWORD dwThreadID; // Thread ID (-1=caller thread).
       DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
    #pragma pack(pop)

    Sleep(10);
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

    __try
    {
        RaiseException
        (
            MS_VC_EXCEPTION,
            0,
            sizeof(info) / sizeof(ULONG_PTR),
            (ULONG_PTR*)&info
        );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
#endif // _MSC_VER
}

} // anonymous namespace


/*--------------------------------------------------------------------------*
                              Thread class
 *--------------------------------------------------------------------------*/

Thread::Thread(const char *name) : m_handle(NULL), m_id(0)
{
    m_handle = (HANDLE)_beginthreadex
                       (
                           NULL,                      // default security
                           0,                         // default stack size
                           &Thread::ThreadEntryPoint,
                           this,                      // arguments
                           CREATE_SUSPENDED,
                           &m_id                      // thread ID
                       );

    if ( !m_handle )
        throw Win32Exception();

    SetThreadName(m_id, name);
}


Thread::~Thread()
{
    if ( m_handle )
        CloseHandle(m_handle);
}


/*static*/ unsigned __stdcall Thread::ThreadEntryPoint(void *data)
{
    try
    {
        Thread *thread = reinterpret_cast<Thread*>(data);
        thread->Run();

        if ( !thread->IsJoinable() )
            delete thread;
    }
    catch ( TerminateThreadException& )
    {
        // this is OK, just return
    }
    CATCH_ALL_EXCEPTIONS

    return 0;
}


void Thread::Start()
{
    if ( !m_handle )
        throw Win32Exception();

    if ( ResumeThread(m_handle) == (DWORD)-1 )
        throw Win32Exception();

    // Wait until Run() signals that it is fully initialized.
    // Note that this must be the last manipulation of 'this' in this function!
    m_signalEvent.WaitUntilSignaled();
}


void Thread::Join()
{
    if ( !m_handle )
        throw Win32Exception();

    if ( WaitForSingleObject(m_handle, INFINITE) != WAIT_OBJECT_0 )
        throw Win32Exception();
}


void Thread::TerminateAndJoin()
{
    m_terminateEvent.Signal();
    Join();
}


void Thread::CheckShouldTerminate()
{
    if (m_terminateEvent.CheckIfSignaled())
        throw TerminateThreadException();
}


void Thread::SignalReady()
{
    m_signalEvent.Signal();
}

} // namespace winsparkle
