/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2026 Vaclav Slavik
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

#include "msix.h"
#include "ui.h"
#include "error.h"

// MSIX silent installation relies on the C++/WinRT projection of
// Windows.Management.Deployment, which ships with the Windows 10 SDK. When those
// headers aren't available (e.g. a MinGW build), this file compiles to a stub
// that simply reports the feature as unsupported.
#if defined(__has_include)
    #if __has_include(<winrt/Windows.Management.Deployment.h>)
        #define WINSPARKLE_HAVE_MSIX 1
    #endif
#endif

#ifdef WINSPARKLE_HAVE_MSIX

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <shlwapi.h>

#include "utils.h"
#include "threads.h"

#include <winrt/Windows.Foundation.h>
// Required for the IIterable<Uri> parameter of AddPackageAsync(); without it the
// parameter type is incomplete and the overload won't resolve.
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Management.Deployment.h>

#include <string>
#include <stdexcept>

// The WinRT runtime imports are delay-loaded (see the /DELAYLOAD entries in the
// project file) so that WinSparkle.dll still loads on Windows versions that
// predate MSIX. runtimeobject.lib provides the Ro*/Windows*String imports that
// the C++/WinRT projection calls into; if a future SDK requires windowsapp.lib
// instead, adjust the project's linker inputs accordingly.
#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "shlwapi.lib")

namespace winsparkle
{

namespace
{

// Probe whether the WinRT deployment API is present before touching any C++/WinRT
// code. The API set below is absent on pre-Win10 systems; calling into a WinRT
// function there would trigger a delay-load failure (a structured exception that
// /EHsc does not catch). Probing first keeps us safe and lets us report cleanly.
bool IsMsixDeploymentAvailable()
{
    HMODULE mod = ::LoadLibraryExW(
        L"api-ms-win-core-winrt-l1-1-0.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!mod)
        return false;
    ::FreeLibrary(mod);
    return true;
}

// Turn a local filesystem path into a properly escaped file:// URI, which is what
// PackageManager.AddPackageAsync() expects.
std::wstring MakeFileUri(const std::wstring& path)
{
    wchar_t url[4096];
    DWORD len = ARRAYSIZE(url);
    HRESULT hr = ::UrlCreateFromPathW(path.c_str(), url, &len, 0);
    if (FAILED(hr))
        throw std::runtime_error("Cannot build file URI for MSIX package");
    return std::wstring(url);
}

void InstallWorker(std::wstring packagePath)
{
    if (!IsMsixDeploymentAvailable())
    {
        LogError("MSIX deployment API is not available on this version of Windows.");
        UI::NotifyInstallFinished(false, L"MSIX installation is not supported on this version of Windows.");
        return;
    }

    bool apartmentInited = false;
    try
    {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
        apartmentInited = true;

        // Register for automatic restart *before* applying the update, so that
        // Windows relaunches the app after ForceApplicationShutdown swaps files.
        ::RegisterApplicationRestart(L"", RESTART_NO_CRASH | RESTART_NO_HANG | RESTART_NO_REBOOT);

        namespace deploy = winrt::Windows::Management::Deployment;

        deploy::PackageManager manager;
        auto op = manager.AddPackageAsync(
            winrt::Windows::Foundation::Uri(MakeFileUri(packagePath)),
            nullptr,
            deploy::DeploymentOptions::ForceApplicationShutdown);

        op.Progress([](auto const&, deploy::DeploymentProgress const& progress)
        {
            UI::NotifyInstallProgress(static_cast<int>(progress.percentage));
        });

        deploy::DeploymentResult result = op.get();

        if (result.ExtendedErrorCode() < 0)
        {
            std::wstring err(result.ErrorText().c_str());
            LogError("MSIX deployment failed: " + WideToAnsi(err));
            UI::NotifyInstallFinished(false, err);
        }
        else
        {
            UI::NotifyInstallFinished(true, std::wstring());
        }
    }
    catch (const winrt::hresult_error& ex)
    {
        std::wstring err(ex.message().c_str());
        LogError("MSIX deployment error: " + WideToAnsi(err));
        UI::NotifyInstallFinished(false, err);
    }
    catch (const std::exception& ex)
    {
        LogError(std::string("MSIX deployment error: ") + ex.what());
        UI::NotifyInstallFinished(false, L"MSIX installation failed.");
    }
    catch (...)
    {
        LogError("MSIX deployment failed with an unknown error.");
        UI::NotifyInstallFinished(false, L"MSIX installation failed.");
    }

    if (apartmentInited)
        winrt::uninit_apartment();
}


// This deliberately does not run on the UI thread: that thread is a
// single-threaded COM apartment (wxWidgets initializes it via OleInitialize), and
// the blocking DeploymentOperation::get() call in InstallWorker() is illegal there
// -- C++/WinRT asserts on it in debug builds and it would stall the message pump
// (deadlock-prone) in release builds. This worker initializes a multi-threaded
// apartment instead, where the blocking wait is legal and safe.
//
// Fire-and-forget: the thread self-destructs once Run() returns
// (IsJoinable() == false), matching the previous detached-thread behavior.
class MsixInstallThread : public Thread
{
public:
    explicit MsixInstallThread(const std::wstring& packagePath)
        : Thread("WinSparkle MSIX installer"), m_packagePath(packagePath)
    {
    }

protected:
    void Run() override
    {
        // No initialization that Start() must wait for, so signal readiness
        // immediately; the deployment below can take a long time and must not
        // block the (UI) thread that called Start().
        SignalReady();
        InstallWorker(m_packagePath);
    }

    bool IsJoinable() const override { return false; }

private:
    std::wstring m_packagePath;
};

} // anonymous namespace


void StartMsixInstall(const std::wstring& packagePath)
{
    // The deployment runs off the UI thread (see MsixInstallThread); progress and
    // completion are reported back asynchronously via UI::NotifyInstall*(). The
    // thread is fire-and-forget and deletes itself when finished.
    (new MsixInstallThread(packagePath))->Start();
}

} // namespace winsparkle

#else // !WINSPARKLE_HAVE_MSIX

namespace winsparkle
{

void StartMsixInstall(const std::wstring& /*packagePath*/)
{
    UI::NotifyInstallFinished(false, L"MSIX installation is not supported in this build.");
}

} // namespace winsparkle

#endif // WINSPARKLE_HAVE_MSIX
