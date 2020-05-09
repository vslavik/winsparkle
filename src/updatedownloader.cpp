/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2012-2020 Vaclav Slavik
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
#include "updatedownloader.h"
#include "download.h"
#include "settings.h"
#include "ui.h"
#include "error.h"
#include "signatureverifier.h"

#include <wx/string.h>

#include <sstream>
#include <rpc.h>
#include <time.h>

namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                  helpers
 *--------------------------------------------------------------------------*/

namespace
{

std::wstring GetUniqueTempDirectoryPrefix()
{
    wchar_t tmpdir[MAX_PATH + 1];
    if (GetTempPath(MAX_PATH + 1, tmpdir) == 0)
        throw Win32Exception("Cannot create temporary directory");

    std::wstring dir(tmpdir);
    dir += L"Update-";
    return dir;
}

std::wstring CreateUniqueTempDirectory()
{
    // We need to put downloaded updates into a directory of their own, because
    // if we put it in $TMP, some DLLs could be there and interfere with the
    // installer.
    //
    // This code creates a new randomized directory name and tries to create it;
    // this process is repeated if the directory already exists.
    const std::wstring tmpdir = GetUniqueTempDirectoryPrefix();

    for ( ;; )
    {
        std::wstring dir(tmpdir);
        UUID uuid;
        UuidCreate(&uuid);
        RPC_WSTR uuidStr;
        RPC_STATUS status = UuidToString(&uuid, &uuidStr);
        dir += reinterpret_cast<wchar_t*>(uuidStr);
        RpcStringFree(&uuidStr);

        if ( CreateDirectory(dir.c_str(), NULL) )
            return dir;
        else if ( GetLastError() != ERROR_ALREADY_EXISTS )
            throw Win32Exception("Cannot create temporary directory");
    }
}

struct UpdateDownloadSink : public IDownloadSink
{
    UpdateDownloadSink(Thread& thread, const std::wstring& dir)
        : m_thread(thread),
          m_dir(dir), m_file(NULL),
          m_downloaded(0), m_total(0), m_lastUpdate(-1)
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

    std::wstring GetFilePath(void) { return m_path; }

    virtual void SetLength(size_t l) { m_total = l; }

    virtual void SetFilename(const std::wstring& filename)
    {
        if ( m_file )
            throw std::runtime_error("Update file already set");

        m_path = m_dir + L"\\" + filename;
        m_file = _wfopen(m_path.c_str(), L"wb");
        if ( !m_file )
            throw std::runtime_error("Cannot save update file");
    }

    virtual void Add(const void *data, size_t len)
    {
        if ( !m_file )
            throw std::runtime_error("Filename is not net");

        m_thread.CheckShouldTerminate();

        if ( fwrite(data, len, 1, m_file) != 1 )
            throw std::runtime_error("Cannot save update file");
        m_downloaded += len;

        // only update at most 10 times/sec so that we don't flood the UI:
        clock_t now = clock();
        if ( now == -1 || m_downloaded == m_total ||
             ((double(now - m_lastUpdate) / CLOCKS_PER_SEC) >= 0.1) )
        {
          UI::NotifyDownloadProgress(m_downloaded, m_total);
          m_lastUpdate = now;
        }
    }

    Thread& m_thread;
    size_t m_downloaded, m_total;
    FILE *m_file;
    std::wstring m_dir;
    std::wstring m_path;
    clock_t m_lastUpdate;
};

} // anonymous namespace


/*--------------------------------------------------------------------------*
                            updater initialization
 *--------------------------------------------------------------------------*/

UpdateDownloader::UpdateDownloader(const Appcast& appcast)
    : Thread("WinSparkle updater"),
      m_appcast(appcast)
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
      const std::wstring tmpdir = CreateUniqueTempDirectory();
      Settings::WriteConfigValue("UpdateTempDir", tmpdir);

      UpdateDownloadSink sink(*this, tmpdir);
      DownloadFile(m_appcast.DownloadURL, &sink, this);
      sink.Close();

      if (Settings::HasDSAPubKeyPem())
      {
          SignatureVerifier::VerifyDSASHA1SignatureValid(sink.GetFilePath(), m_appcast.DsaSignature);
      }
      else
      {
          // backward compatibility - accept as is, but complain about it
          LogError("Using unsigned updates!");
      }

      UI::NotifyUpdateDownloaded(sink.GetFilePath(), m_appcast);
    }
    catch (BadSignatureException&)
    {
        CleanLeftovers();  // remove potentially corrupted file
        UI::NotifyUpdateError(Err_BadSignature);
        throw;
    }
    catch ( ... )
    {
        UI::NotifyUpdateError();
        throw;
    }
}


/*--------------------------------------------------------------------------*
                               cleanup
 *--------------------------------------------------------------------------*/

void UpdateDownloader::CleanLeftovers()
{
    // Note: this is called at startup. Do not use wxWidgets from this code!

    std::wstring tmpdir;
    if ( !Settings::ReadConfigValue("UpdateTempDir", tmpdir) )
        return;

    // Check that the directory actually is a valid update temp dir, to prevent
    // malicious users from forcing us into deleting arbitrary directories:
    try
    {
        if (tmpdir.find(GetUniqueTempDirectoryPrefix()) != 0)
        {
            Settings::DeleteConfigValue("UpdateTempDir");
            return;
        }
    }
    catch (Win32Exception&) // cannot determine temp directory
    {
        return;
    }

    tmpdir.append(1, '\0'); // double NULL-terminate for SHFileOperation

    SHFILEOPSTRUCT fos = {0};
    fos.wFunc = FO_DELETE;
    fos.pFrom = tmpdir.c_str();
    fos.fFlags = FOF_NO_UI | // Vista+-only
                 FOF_SILENT |
                 FOF_NOCONFIRMATION |
                 FOF_NOERRORUI;

    if ( SHFileOperation(&fos) == 0 )
    {
        Settings::DeleteConfigValue("UpdateTempDir");
    }
    // else: try another time, this is just a "soft" error
}

} // namespace winsparkle
