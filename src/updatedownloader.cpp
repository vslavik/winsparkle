/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2012-2013 Vaclav Slavik
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

#include "updatedownloader.h"
#include "download.h"
#include "settings.h"
#include "ui.h"
#include "error.h"

#include <wx/string.h>

#include <sstream>
#include <rpc.h>
#include <time.h>
#include <string>

namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                  helpers
 *--------------------------------------------------------------------------*/

namespace
{

std::string CreateUniqueTempDirectory()
{
    // We need to put downloaded updates into a directory of their own, because
    // if we put it in $TMP, some DLLs could be there and interfere with the
    // installer.
    //
    // This code creates a new randomized directory name and tries to create it;
    // this process is repeated if the directory already exists.
    char tmpdir[MAX_PATH+1];
    if ( GetTempPathA(sizeof(tmpdir), tmpdir) == 0 )
        throw Win32Exception("Cannot create temporary directory");

    for ( ;; )
    {
        std::ostringstream sdir;
        sdir << tmpdir << "Update-";

        UUID uuid;
        UuidCreate(&uuid);
        RPC_CSTR uuidStr;
        UuidToStringA(&uuid, &uuidStr);
        sdir << uuidStr;
        RpcStringFreeA(&uuidStr);

        const std::string dir(sdir.str());

        if ( CreateDirectoryA(dir.c_str(), NULL) )
            return dir;
        else if ( GetLastError() != ERROR_ALREADY_EXISTS )
            throw Win32Exception("Cannot create temporary directory");
    }
}

struct UpdateDownloadSink : public IDownloadSink
{
    UpdateDownloadSink(UpdateDownloader& thread, const std::string& dir, bool isSilent=false)
        : m_thread(thread),
          m_dir(dir), m_path(""), m_file(NULL),
          m_downloaded(0), m_total(0), m_lastUpdate(-1),
          m_isSilent(isSilent)
    {}

    ~UpdateDownloadSink() { Close(); }

    void Close()
    {
        if ( m_file )
        {
            fclose(m_file);
            m_file = NULL;
        }
    }

    std::string GetFilePath(void) { return m_path; }

    virtual void SetLength(size_t l) { m_total = l; }

    virtual void SetFilename(const std::string& filename)
    {
        if ( m_file )
            throw UpdateFileAlreadySetError("Update file already set");

        m_path = m_dir + "\\" + filename;
        m_file = fopen(m_path.c_str(), "wb");
        if ( !m_file )
            throw UpdateUnableToSaveFileError("Cannot save update file");
    }

    virtual void Add(const void *data, size_t len)
    {
        if ( !m_file )
            throw UpdateFileAlreadySetError("Filename is not set");

        m_thread.CheckShouldTerminate();

        if ( fwrite(data, len, 1, m_file) != 1 )
            throw UpdateUnableToSaveFileError("Cannot save update file");
        m_downloaded += len;

        if(!m_isSilent)
        {
            // only update at most 10 times/sec so that we don't flood the UI:
            clock_t now = clock();
            if ( now == -1 || m_downloaded == m_total ||
                 ((double(now - m_lastUpdate) / CLOCKS_PER_SEC) >= 0.1) )
            {
                UI::NotifyDownloadProgress(m_downloaded, m_total);
                m_lastUpdate = now;
            }
        }
    }

    Thread& m_thread;
    size_t m_downloaded, m_total;
    FILE *m_file;
    std::string m_dir;
    std::string m_path;
    clock_t m_lastUpdate;
    bool m_isSilent;
};

} // anonymous namespace


/*--------------------------------------------------------------------------*
                            updater initialization
 *--------------------------------------------------------------------------*/

UpdateDownloader::UpdateDownloader(const Appcast& appcast, bool isSilent)
    : Thread("WinSparkle updater"),
      m_appcast(appcast), m_isSilent(isSilent)
{
}


/*--------------------------------------------------------------------------*
                              downloading
 *--------------------------------------------------------------------------*/

void UpdateDownloader::Run()
{
    // no initialization to do, so signal readiness immediately
    SignalReady();

    try
    {
      const std::string tmpdir = CreateUniqueTempDirectory();
      Settings::WriteConfigValue("UpdateTempDir", tmpdir);

      UpdateDownloadSink sink(*this, tmpdir, m_isSilent);
      DownloadFile(m_appcast.DownloadURL, &sink);
      sink.Close();
      UpdateDownloaded(sink.GetFilePath());
    }
    catch(WinSparkleError& e)
    {
        UpdateError(e.ErrorCode());
        throw;
    }
    catch ( ... )
    {
        UpdateError(UNKNOWN_ERROR);
        throw;
    }
}


/*--------------------------------------------------------------------------*
                               cleanup
 *--------------------------------------------------------------------------*/

void UpdateDownloader::CleanLeftovers()
{
    // Note: this is called at startup. Do not use wxWidgets from this code!

    std::string tmpdir;
    if ( !Settings::ReadConfigValue("UpdateTempDir", tmpdir) )
        return;

    tmpdir.append(1, '\0'); // double NULL-terminate for SHFileOperation

    SHFILEOPSTRUCTA fos = {0};
    fos.wFunc = FO_DELETE;
    fos.pFrom = tmpdir.c_str();
    fos.fFlags = FOF_NO_UI | // Vista+-only
                 FOF_SILENT |
                 FOF_NOCONFIRMATION |
                 FOF_NOERRORUI;

    if ( SHFileOperationA(&fos) == 0 )
    {
        Settings::DeleteConfigValue("UpdateTempDir");
    }
    // else: try another time, this is just a "soft" error
}

void UIUpdateDownloader::UpdateDownloaded(std::string filePath)
{
      UI::NotifyUpdateDownloaded(filePath);
}

void UIUpdateDownloader::UpdateError(int errorCode)
{
    UI::NotifyUpdateError();
}

void SilentUpdateDownloader::UpdateDownloaded(std::string filePath)
{
    m_filePath = filePath;
    m_downloadedCallback(SUCCESS_ERROR);
}

void SilentUpdateDownloader::UpdateError(int errorCode)
{
    m_downloadedCallback(errorCode);
}

std::string SilentUpdateDownloader::GetFilePath()
{
    return m_filePath;
}

} // namespace winsparkle
