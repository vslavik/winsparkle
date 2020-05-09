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

#ifndef _error_h_
#define _error_h_

#include <stdexcept>

namespace winsparkle
{

/**
    Exception thrown if a Platform SDK error happens.

    This exception automatically sets the error message to the appropriate
    error message for GetLastError()'s error code.
 */
class Win32Exception : public std::runtime_error
{
public:
    /**
        Creates the exception.

        @param extraMsg  Extra message shown in front of the win32 error.
     */
    Win32Exception(const char *extraMsg = NULL);
};

/**
    Logs error to, currently, debug output.
 */
void LogError(const char *msg);

inline void LogError(const std::string& msg)
{
    LogError(msg.c_str());
}


/**
    Helper macro for catching exceptions in DLL API interface.

    Currently, the errors are simply logged to debug output with
    OutputDebugString().

    @todo Proper errors reporting is needed.

    Usage:
    @code
    try
    {
        ...
    }
    CATCH_ALL_EXCEPTIONS
    @endcode
 */
#define CATCH_ALL_EXCEPTIONS                                        \
    catch (const std::exception& e)                                 \
    {                                                               \
        winsparkle::LogError(e.what());                             \
    }                                                               \
    catch (...)                                                     \
    {                                                               \
        winsparkle::LogError("Unknown error.");                     \
    }


} // namespace winsparkle

#endif // _error_h_
