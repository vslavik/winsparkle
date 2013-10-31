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

#ifndef _error_h_
#define _error_h_

#include <stdexcept>
#include "errorcodes.h"

namespace winsparkle
{

/**
    Extension for std::runtime_error that includes an error code
*/
class WinSparkleError : public std::runtime_error
{
public:
    WinSparkleError(const std::string message, int errorCode) :
      std::runtime_error(message), m_errorCode(errorCode)
      {}

      int ErrorCode(){ return m_errorCode; }
protected:
    int m_errorCode;
};

/**
    Exception thrown if a Platform SDK error happens.

    This exception automatically sets the error message to the appropriate
    error message for GetLastError()'s error code.
 */
class Win32Exception : public WinSparkleError
{
public:
    /**
        Creates the exception.

        @param extraMsg  Extra message shown in front of the win32 error.
     */
    Win32Exception(const char *extraMsg = NULL);
};

class XMLParserError : public WinSparkleError 
{
public:
    XMLParserError(std::string message) : 
        WinSparkleError(message, XML_PARSER_ERROR)
        {}
};

class XMLParserCreationError : public WinSparkleError
{
public:
    XMLParserCreationError(std::string message) :
      WinSparkleError(message, XML_PARSER_CREATION_ERROR)
      {}
};

class FileNotFoundError : public WinSparkleError
{
public:
    FileNotFoundError(std::string message) : 
      WinSparkleError(message, FILE_NOT_FOUND_ERROR)
      {}
};

class NoTranslationsError : public WinSparkleError
{
public:
    NoTranslationsError(std::string message) : 
      WinSparkleError(message, NO_TRANSLATIONS_FOUND_ERROR)
      {}
};

class AppcastURLNotSpecifiedError : public WinSparkleError
{
public:
    AppcastURLNotSpecifiedError(std::string message) : 
      WinSparkleError(message, APPCAST_URL_NOT_SPECIFIED_ERROR)
      {}
};

class UpdateFileAlreadySetError : public WinSparkleError
{
public:
    UpdateFileAlreadySetError(std::string message) : 
      WinSparkleError(message, UPDATE_FILE_ALREADY_SET_ERROR)
      {}
};

class UpdateUnableToSaveFileError : public WinSparkleError
{
public:
    UpdateUnableToSaveFileError(std::string message) : 
      WinSparkleError(message, UPDATE_UNABLE_TO_SAVE_FILE_ERROR)
      {}
};

class UpdateFileNotSetError : public WinSparkleError
{
public:
    UpdateFileNotSetError(std::string message) : 
      WinSparkleError(message, UPDATE_FILE_NOT_SET_ERROR)
      {}
};

/**
    Logs error to, currently, debug output.
 */
void LogError(const char *msg);


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
