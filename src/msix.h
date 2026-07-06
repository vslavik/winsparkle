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

#ifndef _msix_h_
#define _msix_h_

#include <string>

namespace winsparkle
{

/**
    Silently installs (or upgrades) an MSIX/MSIXBundle package.

    This is used for enclosures marked with sparkle:installerType="msix" in the
    appcast. Unlike .exe/.msi installers (which are launched with ShellExecuteEx),
    .msix packages cannot be installed silently that way: the shell association
    for .msix is the App Installer GUI. Instead, this uses the OS deployment API
    Windows.Management.Deployment.PackageManager.AddPackageAsync() to install the
    package without any UI of its own.

    The install runs on a background thread and returns immediately. Progress and
    the final result are reported asynchronously through the UI notification
    mechanism:
      - UI::NotifyInstallProgress(percent)         while staging (0..100)
      - UI::NotifyInstallFinished(success, error)  when finished

    Before applying the update, the running application is registered for
    automatic restart (RegisterApplicationRestart), so that Windows relaunches it
    after the package files have been swapped during shutdown.

    @param packagePath  Full local path to the downloaded .msix/.msixbundle file.
 */
void StartMsixInstall(const std::wstring& packagePath);

} // namespace winsparkle

#endif // _msix_h_
