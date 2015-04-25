/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2015 Vaclav Slavik
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

#include "ui.h"
#include "settings.h"
#include "error.h"
#include "updatechecker.h"
#include "updatedownloader.h"
#include "appcontroller.h"

#define wxNO_NET_LIB
#define wxNO_XML_LIB
#define wxNO_XRC_LIB
#define wxNO_ADV_LIB
#define wxNO_HTML_LIB
#include <wx/setup.h>

#include <wx/app.h>
#include <wx/dcclient.h>
#include <wx/dialog.h>
#include <wx/display.h>
#include <wx/evtloop.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/filename.h>
#include <wx/gauge.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/settings.h>
#include <wx/msw/ole/activex.h>
#include <wx/msgdlg.h>

#include <exdisp.h>
#include <mshtml.h>


#if !wxCHECK_VERSION(2,9,0)
#error "wxWidgets >= 2.9 is required to compile this code"
#endif

namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                  helpers
 *--------------------------------------------------------------------------*/

namespace
{

// shows/hides layout element
void DoShowElement(wxWindow *w, bool show)
{
    w->GetContainingSizer()->Show(w, show, true/*recursive*/);
}

void DoShowElement(wxSizer *s, bool show)
{
    s->ShowItems(show);
}

#define SHOW(c)    DoShowElement(c, true)
#define HIDE(c)    DoShowElement(c, false)

wxIcon LoadNamedIcon(HMODULE module, const wchar_t *iconName, int size)
{
    HICON hIcon = NULL;

    typedef HRESULT(WINAPI *LPFN_LOADICONWITHSCALEDOWN)(HINSTANCE, PCWSTR, int, int, HICON*);
    LPFN_LOADICONWITHSCALEDOWN f_LoadIconWithScaleDown =
        (LPFN_LOADICONWITHSCALEDOWN) GetProcAddress(GetModuleHandleA("comctl32"), "LoadIconWithScaleDown");

    if (f_LoadIconWithScaleDown)
    {
        if (FAILED(f_LoadIconWithScaleDown(module, iconName, size, size, &hIcon)))
            hIcon = NULL;
    }

    if (!hIcon)
    {
        hIcon = static_cast<HICON>(LoadImage(module, iconName, IMAGE_ICON, size, size, LR_DEFAULTCOLOR));
    }

    if (!hIcon)
        return wxNullIcon;

    wxIcon icon;
    icon.SetHICON(static_cast<WXHICON>(hIcon));
    icon.SetWidth(size);
    icon.SetHeight(size);

    return icon;
}

BOOL CALLBACK GetFirstIconProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    if (IS_INTRESOURCE(lpszName))
        *((LPTSTR*)lParam) = lpszName;
    else
        *((LPTSTR*)lParam) = _tcsdup(lpszName);
    return FALSE; // stop on the first icon found
}

wxIcon GetApplicationIcon(int size)
{
    HMODULE hParentExe = GetModuleHandle(NULL);
    if ( !hParentExe )
        return wxNullIcon;

    LPTSTR iconName = 0;
    EnumResourceNames(hParentExe, RT_GROUP_ICON, GetFirstIconProc, (LONG_PTR)&iconName);

    if ( GetLastError() != ERROR_SUCCESS && GetLastError() != ERROR_RESOURCE_ENUM_USER_STOP )
        return wxNullIcon;

    wxIcon icon = LoadNamedIcon(hParentExe, iconName, size);

    if ( !IS_INTRESOURCE(iconName) )
        free(iconName);

    return icon;
}


struct EventPayload
{
    Appcast      appcast;
    size_t       sizeDownloaded, sizeTotal;
    std::wstring updateFile;
};


struct EnumProcessWindowsData
{
    DWORD process_id;
    wxRect biggest;
};

BOOL CALLBACK EnumProcessWindowsCallback(HWND handle, LPARAM lParam)
{
    EnumProcessWindowsData& data = *reinterpret_cast<EnumProcessWindowsData*>(lParam);

    if (!IsWindowVisible(handle))
        return TRUE;

    DWORD process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id)
        return TRUE; // another process' window

    if (GetWindow(handle, GW_OWNER) != 0)
        return TRUE; // child, not main, window

    RECT rwin;
    GetWindowRect(handle, &rwin);
    wxRect r(rwin.left, rwin.top, rwin.right - rwin.left, rwin.bottom - rwin.top);
    if (r.width * r.height > data.biggest.width * data.biggest.height)
        data.biggest = r;

    return TRUE;
}

wxRect GetHostApplicationScreenArea()
{
    EnumProcessWindowsData data;
    data.process_id = GetCurrentProcessId();
    EnumWindows(EnumProcessWindowsCallback, (LPARAM)&data);
    return data.biggest;
}

wxPoint GetWindowOriginSoThatItFits(int display, const wxRect& windowRect)
{
    wxPoint pos = windowRect.GetTopLeft();
    wxRect desktop = wxDisplay(display).GetClientArea();
    if (!desktop.Contains(windowRect))
    {
        if (pos.x < desktop.x)
            pos.x = desktop.x;
        if (pos.y < desktop.y)
            pos.y = desktop.y;
       wxPoint bottomRightDiff = windowRect.GetBottomRight() - desktop.GetBottomRight();
       if (bottomRightDiff.x > 0)
           pos.x -= bottomRightDiff.x;
       if (bottomRightDiff.y > 0)
           pos.y -= bottomRightDiff.y;
    }
    return pos;
}

void CenterWindowOnHostApplication(wxTopLevelWindow *win)
{
    // find application's biggest window:
    EnumProcessWindowsData data;
    data.process_id = GetCurrentProcessId();
    EnumWindows(EnumProcessWindowsCallback, (LPARAM) &data);

    if (data.biggest.IsEmpty())
        return; // no window to center on

    const wxRect& host(data.biggest);

    // and center WinSparkle on it:
    wxSize winsz = win->GetClientSize();
    wxPoint pos(host.x + (host.width - winsz.x) / 2,
                host.y + (host.height - winsz.y) / 2);

    // make sure the window is fully visible:
    int display = wxDisplay::GetFromPoint(wxPoint(host.x + host.width / 2,
                                                  host.y + host.height / 2));
    if (display != wxNOT_FOUND)
    {
        pos = GetWindowOriginSoThatItFits(display, wxRect(pos, winsz));
    }

    win->Move(pos);
}

void EnsureWindowIsFullyVisible(wxTopLevelWindow *win)
{
    int display = wxDisplay::GetFromWindow(win);
    if (display == wxNOT_FOUND)
        return;

    wxPoint pos = GetWindowOriginSoThatItFits(display, win->GetRect());
    win->Move(pos);
}


// Locks window updates to reduce flicker. Redoes layout in dtor.
struct LayoutChangesGuard
{
    LayoutChangesGuard(wxTopLevelWindow *win) : m_win(win)
    {
        m_win->Freeze();
    }

    ~LayoutChangesGuard()
    {
        m_win->Layout();
        m_win->GetSizer()->SetSizeHints(m_win);
        m_win->Refresh();
        EnsureWindowIsFullyVisible(m_win);
        m_win->Thaw();
        m_win->Update();
    }

    wxTopLevelWindow *m_win;
};

} // anonymous namespace


/*--------------------------------------------------------------------------*
                       Base class for WinSparkle dialogs
 *--------------------------------------------------------------------------*/

class WinSparkleDialog : public wxDialog
{
protected:
    WinSparkleDialog();

    void UpdateLayout();
    static void SetBoldFont(wxWindow *win);
    static void SetHeadingFont(wxWindow *win);

    // enable/disable resizing of the dialog
    void MakeResizable(bool resizable = true);

protected:
    // sizer for the main area of the dialog (to the right of the icon)
    wxSizer      *m_mainAreaSizer;

    // High DPI support:
    double        m_scaleFactor;
    #define PX(x) (int((x) * m_scaleFactor))

    static const int MESSAGE_AREA_WIDTH = 300;
};


WinSparkleDialog::WinSparkleDialog()
    : wxDialog(NULL, wxID_ANY, _("Software Update"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT)
{
    wxSize dpi = wxClientDC(this).GetPPI();
    m_scaleFactor = dpi.y / 96.0;

    SetIcon(LoadNamedIcon(UI::GetDllHINSTANCE(), L"UpdateAvailable", GetSystemMetrics(SM_CXSMICON)));

    wxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

    // Load the dialog box icon: the first 48x48 application icon will be loaded, if available,
    // otherwise, the standard WinSparkle icon will be used.
    wxIcon bigIcon = GetApplicationIcon(PX(48));
    if ( !bigIcon.IsOk() )
        bigIcon = LoadNamedIcon(UI::GetDllHINSTANCE(), L"UpdateAvailable", PX(48));

    topSizer->Add
              (
                  new wxStaticBitmap(this, wxID_ANY, bigIcon),
                  wxSizerFlags(0).Border(wxALL, PX(10))
              );

    m_mainAreaSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(m_mainAreaSizer, wxSizerFlags(1).Expand().Border(wxALL, PX(10)));

    SetSizer(topSizer);

    MakeResizable(false);
}


void WinSparkleDialog::MakeResizable(bool resizable)
{
    bool is_resizable = (GetWindowStyleFlag() & wxRESIZE_BORDER) != 0;
    if ( is_resizable == resizable )
        return;

    ToggleWindowStyle(wxRESIZE_BORDER);
    Refresh(); // to paint the gripper
}


void WinSparkleDialog::UpdateLayout()
{
    Layout();
    GetSizer()->SetSizeHints(this);
}


void WinSparkleDialog::SetBoldFont(wxWindow *win)
{
    wxFont f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    f.SetWeight(wxFONTWEIGHT_BOLD);

    win->SetFont(f);
}


void WinSparkleDialog::SetHeadingFont(wxWindow *win)
{
    wxFont f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    int winver;
    wxGetOsVersion(&winver);
    if ( winver >= 6 ) // Windows Vista, 7 or newer
    {
        // 9pt is base font, 12pt is for "Main instructions". See
        // http://msdn.microsoft.com/en-us/library/aa511282%28v=MSDN.10%29.aspx
        f.SetPointSize(f.GetPointSize() + 3);
        win->SetForegroundColour(wxColour(0x00, 0x33, 0x99));
    }
    else // Windows XP/2000
    {
        f.SetWeight(wxFONTWEIGHT_BOLD);
        f.SetPointSize(f.GetPointSize() + 1);
    }

    win->SetFont(f);
}


/*--------------------------------------------------------------------------*
                      Window for communicating with the user
 *--------------------------------------------------------------------------*/

const int ID_SKIP_VERSION = wxNewId();
const int ID_REMIND_LATER = wxNewId();
const int ID_INSTALL = wxNewId();
const int ID_RUN_INSTALLER = wxNewId();

class UpdateDialog : public WinSparkleDialog
{
public:
    UpdateDialog();

    // changes state into "checking for updates"
    void StateCheckingUpdates();
    // change state into "no updates found"
    void StateNoUpdateFound();
    // change state into "update error"
    void StateUpdateError();
    // change state into "a new version is available"
    void StateUpdateAvailable(const Appcast& info);
    // change state into "downloading update"
    void StateDownloading();
    // update download progress
    void DownloadProgress(size_t downloaded, size_t total);
    // change state into "update downloaded"
    void StateUpdateDownloaded(const std::wstring& updateFile, const std::string &installerArguments);

private:
    void EnablePulsing(bool enable);
    void OnTimer(wxTimerEvent& event);
    void OnCloseButton(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnSkipVersion(wxCommandEvent&);
    void OnRemindLater(wxCommandEvent&);
    void OnInstall(wxCommandEvent&);

    void OnRunInstaller(wxCommandEvent&);

    bool RunInstaller();

    void SetMessage(const wxString& text, int width = MESSAGE_AREA_WIDTH);
    void ShowReleaseNotes(const Appcast& info);

private:
    wxTimer       m_timer;
    wxSizer      *m_buttonSizer;
    wxStaticText *m_heading;
    wxStaticText *m_message;
    wxGauge      *m_progress;
    wxStaticText *m_progressLabel;
    wxButton     *m_closeButton;
    wxSizer      *m_closeButtonSizer;
    wxButton     *m_runInstallerButton;
    wxSizer      *m_runInstallerButtonSizer;
    wxButton     *m_installButton;
    wxSizer      *m_updateButtonsSizer;
    wxSizer      *m_releaseNotesSizer;
    wxPanel      *m_browserParent;

    wxAutoOleInterface<IWebBrowser2> m_webBrowser;

    // current appcast data (only valid after StateUpdateAvailable())
    Appcast m_appcast;
    // current update file (only valid after StateUpdateDownloaded)
    wxString m_updateFile;
    // space separated arguments to update file (only valid after StateUpdateDownloaded)
    std::string m_installerArguments;
    // downloader (only valid between OnInstall and OnUpdateDownloaded)
    UpdateDownloader* m_downloader;

    static const int RELNOTES_WIDTH = 460;
    static const int RELNOTES_HEIGHT = 200;
};


UpdateDialog::UpdateDialog()
    : m_timer(this),
      m_downloader(NULL)
{
    m_heading = new wxStaticText(this, wxID_ANY, "");
    SetHeadingFont(m_heading);
    m_mainAreaSizer->Add(m_heading, wxSizerFlags(0).Expand().Border(wxBOTTOM, PX(10)));

    m_message = new wxStaticText(this, wxID_ANY, "",
                                 wxDefaultPosition, wxSize(PX(MESSAGE_AREA_WIDTH), -1));
    m_mainAreaSizer->Add(m_message, wxSizerFlags(0).Expand());

    m_progress = new wxGauge(this, wxID_ANY, 100,
                             wxDefaultPosition, wxSize(PX(MESSAGE_AREA_WIDTH), PX(16)));
    m_progressLabel = new wxStaticText(this, wxID_ANY, "");
    m_mainAreaSizer->Add(m_progress, wxSizerFlags(0).Expand().Border(wxTOP|wxBOTTOM, PX(10)));
    m_mainAreaSizer->Add(m_progressLabel, wxSizerFlags(0).Expand());
    m_mainAreaSizer->AddStretchSpacer(1);

    m_releaseNotesSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText *notesLabel = new wxStaticText(this, wxID_ANY, _("Release notes:"));
    SetBoldFont(notesLabel);
    m_releaseNotesSizer->Add(notesLabel, wxSizerFlags().Border(wxTOP, PX(10)));

    m_browserParent = new wxPanel(this, wxID_ANY,
                                  wxDefaultPosition,
                                  wxSize(PX(RELNOTES_WIDTH), PX(RELNOTES_HEIGHT)));
    m_browserParent->SetBackgroundColour(*wxWHITE);
    m_releaseNotesSizer->Add
                         (
                             m_browserParent,
                             wxSizerFlags(1).Expand().Border(wxTOP, PX(10))
                         );

    m_mainAreaSizer->Add
                 (
                     m_releaseNotesSizer,
                     // proportion=10000 to overcome stretch spacer above
                     wxSizerFlags(10000).Expand()
                 );

    m_buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    m_updateButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    m_updateButtonsSizer->Add
                          (
                            new wxButton(this, ID_SKIP_VERSION, _("Skip this version")),
                            wxSizerFlags().Border(wxRIGHT, PX(20))
                          );
    m_updateButtonsSizer->AddStretchSpacer(1);
    m_updateButtonsSizer->Add
                          (
                            new wxButton(this, ID_REMIND_LATER, _("Remind me later")),
                            wxSizerFlags().Border(wxRIGHT, PX(10))
                          );
    m_updateButtonsSizer->Add
                          (
                            m_installButton = new wxButton(this, ID_INSTALL, _("Install update")),
                            wxSizerFlags()
                          );
    m_buttonSizer->Add(m_updateButtonsSizer, wxSizerFlags(1));

    m_closeButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_closeButton = new wxButton(this, wxID_CANCEL);
    m_closeButtonSizer->AddStretchSpacer(1);
    m_closeButtonSizer->Add(m_closeButton, wxSizerFlags(0).Border(wxLEFT));
    m_buttonSizer->Add(m_closeButtonSizer, wxSizerFlags(1));

    m_runInstallerButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    // TODO: make this "Install and relaunch"
    m_runInstallerButton = new wxButton(this, ID_RUN_INSTALLER, _("Install update"));
    m_runInstallerButtonSizer->AddStretchSpacer(1);
    m_runInstallerButtonSizer->Add(m_runInstallerButton, wxSizerFlags(0).Border(wxLEFT));
    m_buttonSizer->Add(m_runInstallerButtonSizer, wxSizerFlags(1));

    m_mainAreaSizer->Add
                 (
                     m_buttonSizer,
                     wxSizerFlags(0).Expand().Border(wxTOP, PX(10))
                 );

    UpdateLayout();

    Bind(wxEVT_CLOSE_WINDOW, &UpdateDialog::OnClose, this);
    Bind(wxEVT_TIMER, &UpdateDialog::OnTimer, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnCloseButton, this, wxID_CANCEL);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnSkipVersion, this, ID_SKIP_VERSION);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnRemindLater, this, ID_REMIND_LATER);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnInstall, this, ID_INSTALL);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnRunInstaller, this, ID_RUN_INSTALLER);
}


void UpdateDialog::EnablePulsing(bool enable)
{
    if ( enable && !m_timer.IsRunning() )
        m_timer.Start(100);
    else if ( !enable && m_timer.IsRunning() )
        m_timer.Stop();
}


void UpdateDialog::OnTimer(wxTimerEvent&)
{
    m_progress->Pulse();
}


void UpdateDialog::OnCloseButton(wxCommandEvent&)
{
    Close();
}


void UpdateDialog::OnClose(wxCloseEvent&)
{
    if ( m_downloader )
    {
        m_downloader->TerminateAndJoin();
        delete m_downloader;
        m_downloader = NULL;

        UpdateDownloader::CleanLeftovers();
    }

    // We need to override this, because by default, wxDialog doesn't
    // destroy itself in Close().
    Destroy();
}


void UpdateDialog::OnSkipVersion(wxCommandEvent&)
{
    Settings::WriteConfigValue("SkipThisVersion", m_appcast.Version);
    Close();
}


void UpdateDialog::OnRemindLater(wxCommandEvent&)
{
    // Just abort the update. Next time it's scheduled to run,
    // the user will be prompted.
    Close();
}


void UpdateDialog::OnInstall(wxCommandEvent&)
{
    if ( !m_appcast.HasDownload() )
    {
        wxLaunchDefaultBrowser(m_appcast.WebBrowserURL, wxBROWSER_NEW_WINDOW);
        Close();
    }
    else if ( m_downloader == NULL )
    {
        StateDownloading();

        // Run the download in background.
        m_downloader = new UpdateDownloader(m_appcast);
        m_downloader->Start();
    }
}

void UpdateDialog::OnRunInstaller(wxCommandEvent&)
{
    if( !ApplicationController::IsReadyToShutdown() )
    {
        wxMessageDialog dlg(this,
                            wxString::Format(_("%s cannot be restarted."), Settings::GetAppName()),
                            _("Software Update"),
                            wxOK | wxOK_DEFAULT | wxICON_EXCLAMATION);
        dlg.SetExtendedMessage(_("Make sure that you don't have any unsaved documents and try again."));
        dlg.ShowModal();
        return;
    }

    wxBusyCursor bcur;

    m_message->SetLabel(_("Launching the installer..."));
    m_runInstallerButton->Disable();

    if ( !RunInstaller() )
    {
        wxMessageDialog dlg(this,
            _("Failed to launch the installer."),
            _("Software Update"),
            wxOK | wxOK_DEFAULT | wxICON_EXCLAMATION);
        dlg.ShowModal();
    }
    else
    {
        Close();
        ApplicationController::RequestShutdown();
    }
}

bool UpdateDialog::RunInstaller()
{
    if (m_installerArguments.empty()) 
    {
        // keep old way of calling updater to not accidentally break any existing code
        return wxLaunchDefaultApplication(m_updateFile);
    } 
    else 
    {
        // wxExecute() returns a process id, or zero on failure
        long processId = wxExecute(m_updateFile + " " + m_installerArguments);
        return processId != 0;
    }
}

void UpdateDialog::SetMessage(const wxString& text, int width)
{
    m_message->SetLabel(text);
    m_message->Wrap(PX(width));
}


void UpdateDialog::StateCheckingUpdates()
{
    LayoutChangesGuard guard(this);

    SetMessage(_("Checking for updates..."));

    m_closeButton->SetLabel(_("Cancel"));
    EnablePulsing(true);

    HIDE(m_heading);
    SHOW(m_progress);
    HIDE(m_progressLabel);
    SHOW(m_closeButtonSizer);
    HIDE(m_runInstallerButtonSizer);
    HIDE(m_releaseNotesSizer);
    HIDE(m_updateButtonsSizer);
    MakeResizable(false);
}


void UpdateDialog::StateNoUpdateFound()
{
    LayoutChangesGuard guard(this);

    m_heading->SetLabel(_("You're up to date!"));

    wxString msg;
    try
    {
        msg = wxString::Format
              (
                  _("%s %s is currently the newest version available."),
                  Settings::GetAppName(),
                  Settings::GetAppVersion()
              );
    }
    catch ( std::exception& )
    {
        // GetAppVersion() may fail
        msg = "Error: Updates checking not properly configured.";
    }

    SetMessage(msg);

    m_closeButton->SetLabel(_("Close"));
    m_closeButton->SetDefault();
    EnablePulsing(false);

    SHOW(m_heading);
    HIDE(m_progress);
    HIDE(m_progressLabel);
    SHOW(m_closeButtonSizer);
    HIDE(m_runInstallerButtonSizer);
    HIDE(m_releaseNotesSizer);
    HIDE(m_updateButtonsSizer);
    MakeResizable(false);
}


void UpdateDialog::StateUpdateError()
{
    LayoutChangesGuard guard(this);

    m_heading->SetLabel(_("Update Error!"));

    wxString msg = _("An error occurred in retrieving update information; are you connected to the internet? Please try again later.");
    SetMessage(msg);

    m_closeButton->SetLabel(_("Cancel"));
    m_closeButton->SetDefault();
    EnablePulsing(false);

    SHOW(m_heading);
    HIDE(m_progress);
    HIDE(m_progressLabel);
    SHOW(m_closeButtonSizer);
    HIDE(m_runInstallerButtonSizer);
    HIDE(m_releaseNotesSizer);
    HIDE(m_updateButtonsSizer);
    MakeResizable(false);
}



void UpdateDialog::StateUpdateAvailable(const Appcast& info)
{
    m_appcast = info;

    const bool showRelnotes = !info.ReleaseNotesURL.empty() || !info.Description.empty();

    {
        LayoutChangesGuard guard(this);

        const wxString appname = Settings::GetAppName();

        wxString ver_my = Settings::GetAppVersion();
        wxString ver_new = info.ShortVersionString;
        if ( ver_new.empty() )
            ver_new = info.Version;
        if ( ver_my == ver_new )
        {
            ver_my = wxString::Format("%s (%s)", ver_my, Settings::GetAppBuildVersion());
            ver_new = wxString::Format("%s (%s)", ver_new, info.Version);
        }

        m_heading->SetLabel(
            wxString::Format(_("A new version of %s is available!"), appname));

        if ( !info.HasDownload() )
            m_installButton->SetLabel(_("Get update"));

        SetMessage
        (
            wxString::Format
            (
                _("%s %s is now available (you have %s). Would you like to download it now?"),
                appname, ver_new, ver_my
            ),
            showRelnotes ? RELNOTES_WIDTH : MESSAGE_AREA_WIDTH
        );

        EnablePulsing(false);

        m_installButton->SetDefault();

        SHOW(m_heading);
        HIDE(m_progress);
        HIDE(m_progressLabel);
        HIDE(m_closeButtonSizer);
        HIDE(m_runInstallerButtonSizer);
        SHOW(m_updateButtonsSizer);
        DoShowElement(m_releaseNotesSizer, showRelnotes);
        MakeResizable(showRelnotes);
    }

    // Only show the release notes now that the layout was updated, as it may
    // take some time to load the MSIE control:
    if ( showRelnotes )
        ShowReleaseNotes(info);
}


void UpdateDialog::StateDownloading()
{
    LayoutChangesGuard guard(this);

    SetMessage(_("Downloading update..."));

    m_closeButton->SetLabel(_("Cancel"));
    EnablePulsing(false);

    HIDE(m_heading);
    SHOW(m_progress);
    SHOW(m_progressLabel);
    SHOW(m_closeButtonSizer);
    HIDE(m_runInstallerButtonSizer);
    HIDE(m_releaseNotesSizer);
    HIDE(m_updateButtonsSizer);
    MakeResizable(false);
}


void UpdateDialog::DownloadProgress(size_t downloaded, size_t total)
{
    wxString label;

    if ( total )
    {
        if ( m_progress->GetRange() != total )
            m_progress->SetRange(total);
        m_progress->SetValue(downloaded);
        label = wxString::Format
                (
                    _("%s of %s"),
                    wxFileName::GetHumanReadableSize(downloaded, "", 1, wxSIZE_CONV_SI),
                    wxFileName::GetHumanReadableSize(total, "", 1, wxSIZE_CONV_SI)
                );
    }
    else
    {
        m_progress->Pulse();
        label = wxFileName::GetHumanReadableSize(downloaded, "", 1, wxSIZE_CONV_SI);
    }

    if ( label != m_progressLabel->GetLabel() )
      m_progressLabel->SetLabel(label);

    Refresh();
    Update();
}


void UpdateDialog::StateUpdateDownloaded(const std::wstring& updateFile, const std::string& installerArguments)
{
    m_downloader->Join();
    delete m_downloader;
    m_downloader = NULL;

    m_updateFile = updateFile;
    m_installerArguments = installerArguments;

    LayoutChangesGuard guard(this);

    SetMessage(_("Ready to install."));

    m_progress->SetRange(1);
    m_progress->SetValue(1);

    m_runInstallerButton->SetDefault();

    HIDE(m_heading);
    SHOW(m_progress);
    HIDE(m_progressLabel);
    HIDE(m_closeButtonSizer);
    SHOW(m_runInstallerButtonSizer);
    HIDE(m_releaseNotesSizer);
    HIDE(m_updateButtonsSizer);
    MakeResizable(false);
}


void UpdateDialog::ShowReleaseNotes(const Appcast& info)
{
    if ( !m_webBrowser.IsOk() )
    {
        // Load MSIE control

        wxBusyCursor busy;

        IWebBrowser2 *browser;
        HRESULT hr = CoCreateInstance
                     (
                         CLSID_WebBrowser,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         IID_IWebBrowser2,
                         (void**)&browser
                     );

        if ( FAILED(hr) )
        {
            // hide the notes again, we cannot show them
            LayoutChangesGuard guard(this);
            HIDE(m_releaseNotesSizer);
            MakeResizable(false);
            LogError("Failed to create WebBrowser ActiveX control.");
            return;
        }

        m_webBrowser = browser;

        new wxActiveXContainer(m_browserParent, IID_IWebBrowser2, browser);
    }

    if( !info.ReleaseNotesURL.empty() )
    {
        m_webBrowser->Navigate
                      (
                          wxBasicString(info.ReleaseNotesURL),
                          NULL,  // Flags
                          NULL,  // TargetFrameName
                          NULL,  // PostData
                          NULL   // Headers
                      );
    }
    else if ( !info.Description.empty() )
    {
        m_webBrowser->Navigate
                      (
                          wxBasicString("about:blank"),
                          NULL,  // Flags
                          NULL,  // TargetFrameName
                          NULL,  // PostData
                          NULL   // Headers
                      );

        HRESULT hr = E_FAIL;
        IHTMLDocument2 *doc;
        hr = m_webBrowser->get_Document((IDispatch **)&doc);
        if ( FAILED(hr) || !doc )
        {
            LogError("Failed to get HTML document");
            return;
        }

        // If the code below looks crazy, that's because it is. Apparently,
        // the document may be in some uninitialized state first time around,
        // so we need to write an empty string into it first and only then the
        // real content. (At least that's what wxWebView does...)
        //
        // See https://github.com/vslavik/winsparkle/issues/29
        SAFEARRAY *psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
        if ( psaStrings != NULL )
        {
            VARIANT *param;
            SafeArrayAccessData(psaStrings, (LPVOID*)&param);
            param->vt = VT_BSTR;
            param->bstrVal = SysAllocString(OLESTR(""));
            SafeArrayUnaccessData(psaStrings);

            doc->write(psaStrings);
            doc->close();

            SafeArrayDestroy(psaStrings);

            psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
            if (psaStrings != NULL)
            {
                VARIANT *param;
                SafeArrayAccessData(psaStrings, (LPVOID*) &param);
                param->vt = VT_BSTR;
                param->bstrVal = wxBasicString(wxString::FromUTF8(info.Description.c_str()));
                SafeArrayUnaccessData(psaStrings);

                doc->write(psaStrings);
                doc->close();

                SafeArrayDestroy(psaStrings);
            }

            doc->Release();
        }
    }

    SetWindowStyleFlag(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
}


/*--------------------------------------------------------------------------*
              Dialog that asks for permission to check for updates
 *--------------------------------------------------------------------------*/

class AskPermissionDialog : public WinSparkleDialog
{
public:
    AskPermissionDialog();
};


AskPermissionDialog::AskPermissionDialog()
{
    wxStaticText *heading =
            new wxStaticText(this, wxID_ANY,
                             _("Check for updates automatically?"));
    SetHeadingFont(heading);
    m_mainAreaSizer->Add(heading, wxSizerFlags(0).Expand().Border(wxBOTTOM, PX(10)));

    wxStaticText *message =
            new wxStaticText
                (
                    this, wxID_ANY,
                    wxString::Format
                    (
                        _("Should %s automatically check for updates? You can always check for updates manually from the menu."),
                        Settings::GetAppName()
                    ),
                    wxDefaultPosition, wxSize(PX(MESSAGE_AREA_WIDTH), -1)
                );
    message->Wrap(PX(MESSAGE_AREA_WIDTH));
    m_mainAreaSizer->Add(message, wxSizerFlags(0).Expand());

    m_mainAreaSizer->AddStretchSpacer(1);

    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    buttonSizer->Add
                 (
                     new wxButton(this, wxID_OK, _("Check automatically")),
                     wxSizerFlags().Border(wxRIGHT)
                 );
    buttonSizer->Add
                 (
                     new wxButton(this, wxID_CANCEL, _("Don't check"))
                 );

    m_mainAreaSizer->Add
                 (
                     buttonSizer,
                     wxSizerFlags(0).Right().Border(wxTOP, PX(10))
                 );

    UpdateLayout();
}


/*--------------------------------------------------------------------------*
                             Inter-thread messages
 *--------------------------------------------------------------------------*/

// Terminate the wx thread
const int MSG_TERMINATE = wxNewId();

// Show "Checking for updates..." window
const int MSG_SHOW_CHECKING_UPDATES = wxNewId();

// Notify the UI that there were no updates
const int MSG_NO_UPDATE_FOUND = wxNewId();

// Notify the UI that a new version is available
const int MSG_UPDATE_AVAILABLE = wxNewId();

// Notify the UI that a new version is available
const int MSG_UPDATE_ERROR = wxNewId();

// Inform the UI about download progress
const int MSG_DOWNLOAD_PROGRESS = wxNewId();

// Inform the UI that update download finished
const int MSG_UPDATE_DOWNLOADED = wxNewId();

// Tell the UI to ask for permission to check updates
const int MSG_ASK_FOR_PERMISSION = wxNewId();


/*--------------------------------------------------------------------------*
                                Application
 *--------------------------------------------------------------------------*/

class App : public wxApp
{
public:
    App();

    // Sends a message with ID @a msg to the app.
    void SendMsg(int msg, EventPayload *data = NULL);

private:
    void InitWindow();
    void ShowWindow();

    void OnWindowClose(wxCloseEvent& event);
    void OnTerminate(wxThreadEvent& event);
    void OnShowCheckingUpdates(wxThreadEvent& event);
    void OnNoUpdateFound(wxThreadEvent& event);
    void OnUpdateAvailable(wxThreadEvent& event);
    void OnUpdateError(wxThreadEvent& event);
    void OnDownloadProgress(wxThreadEvent& event);
    void OnUpdateDownloaded(wxThreadEvent& event);
    void OnAskForPermission(wxThreadEvent& event);

private:
    UpdateDialog *m_win;
};

IMPLEMENT_APP_NO_MAIN(App)

App::App()
{
    m_win = NULL;

    // Keep the wx "main" thread running even without windows. This greatly
    // simplifies threads handling, because we don't have to correctly
    // implement wx-thread restarting.
    //
    // Note that this only works if we don't explicitly call ExitMainLoop(),
    // except in reaction to win_sparkle_cleanup()'s message.
    // win_sparkle_cleanup() relies on the availability of wxApp instance and
    // if the event loop terminated, wxEntry() would return and wxApp instance
    // would be destroyed.
    //
    // Also note that this is efficient, because if there are no windows, the
    // thread will sleep waiting for a new event. We could safe some memory
    // by shutting the thread down when it's no longer needed, though.
    SetExitOnFrameDelete(false);

    // Bind events to their handlers:
    Bind(wxEVT_COMMAND_THREAD, &App::OnTerminate, this, MSG_TERMINATE);
    Bind(wxEVT_COMMAND_THREAD, &App::OnShowCheckingUpdates, this, MSG_SHOW_CHECKING_UPDATES);
    Bind(wxEVT_COMMAND_THREAD, &App::OnNoUpdateFound, this, MSG_NO_UPDATE_FOUND);
    Bind(wxEVT_COMMAND_THREAD, &App::OnUpdateAvailable, this, MSG_UPDATE_AVAILABLE);
    Bind(wxEVT_COMMAND_THREAD, &App::OnUpdateError, this, MSG_UPDATE_ERROR);
    Bind(wxEVT_COMMAND_THREAD, &App::OnDownloadProgress, this, MSG_DOWNLOAD_PROGRESS);
    Bind(wxEVT_COMMAND_THREAD, &App::OnUpdateDownloaded, this, MSG_UPDATE_DOWNLOADED);
    Bind(wxEVT_COMMAND_THREAD, &App::OnAskForPermission, this, MSG_ASK_FOR_PERMISSION);
}


void App::SendMsg(int msg, EventPayload *data)
{
    wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_THREAD, msg);
    if ( data )
        event->SetPayload(*data);

    wxQueueEvent(this, event);
}


void App::InitWindow()
{
    if ( !m_win )
    {
        m_win = new UpdateDialog();
        m_win->Bind(wxEVT_CLOSE_WINDOW, &App::OnWindowClose, this);
    }
}


void App::ShowWindow()
{
    wxASSERT( m_win );

    m_win->Freeze();
    if (!m_win->IsShown())
        CenterWindowOnHostApplication(m_win);
    m_win->Show();
    m_win->Thaw();
    m_win->Raise();
}


void App::OnWindowClose(wxCloseEvent& event)
{
    m_win = NULL;
    event.Skip();
}


void App::OnTerminate(wxThreadEvent&)
{
    wxEventLoopBase *activeLoop = wxEventLoop::GetActive();
    if (!activeLoop->IsMain())
        activeLoop->Exit(wxID_CANCEL);
    GetMainLoop()->ScheduleExit(0);
}


void App::OnShowCheckingUpdates(wxThreadEvent&)
{
    InitWindow();
    m_win->StateCheckingUpdates();
    ShowWindow();
}


void App::OnNoUpdateFound(wxThreadEvent&)
{
    if ( m_win )
        m_win->StateNoUpdateFound();
}


void App::OnUpdateError(wxThreadEvent&)
{
    if ( m_win )
        m_win->StateUpdateError();
}

void App::OnDownloadProgress(wxThreadEvent& event)
{
    if ( m_win )
    {
        EventPayload payload(event.GetPayload<EventPayload>());
        m_win->DownloadProgress(payload.sizeDownloaded, payload.sizeTotal);
    }
}

void App::OnUpdateDownloaded(wxThreadEvent& event)
{
    if ( m_win )
    {
        EventPayload payload(event.GetPayload<EventPayload>());
        m_win->StateUpdateDownloaded(payload.updateFile, payload.appcast.InstallerArguments);
    }
}


void App::OnUpdateAvailable(wxThreadEvent& event)
{
    InitWindow();

    EventPayload payload(event.GetPayload<EventPayload>());
    m_win->StateUpdateAvailable(payload.appcast);

    ShowWindow();
}


void App::OnAskForPermission(wxThreadEvent& event)
{
    AskPermissionDialog dlg;
    bool shouldCheck = (dlg.ShowModal() == wxID_OK);

    Settings::WriteConfigValue("CheckForUpdates", shouldCheck);

    if ( shouldCheck )
    {
        UpdateChecker *check = new UpdateChecker();
        check->Start();
    }
}


/*--------------------------------------------------------------------------*
                             winsparkle::UI class
 *--------------------------------------------------------------------------*/

// helper for accessing the UI thread
class UIThreadAccess
{
public:
    UIThreadAccess() : m_lock(ms_uiThreadCS) {}

    App& App()
    {
        StartIfNeeded();
        return wxGetApp();
    };

    bool IsRunning() const { return ms_uiThread != NULL; }

    // intentionally not static, to force locking before access
    UI* UIThread() { return ms_uiThread; }

    void ShutDownThread()
    {
        if ( ms_uiThread )
        {
            ms_uiThread->Join();
            delete ms_uiThread;
            ms_uiThread = NULL;
        }
    }

private:
    void StartIfNeeded()
    {
        // if the thread is not running yet, we have to start it
        if ( !ms_uiThread )
        {
            ms_uiThread = new UI();
            ms_uiThread->Start();
        }
    }

    CriticalSectionLocker m_lock;

    static UI *ms_uiThread;
    static CriticalSection ms_uiThreadCS;
};

UI *UIThreadAccess::ms_uiThread = NULL;
CriticalSection UIThreadAccess::ms_uiThreadCS;


HINSTANCE UI::ms_hInstance = NULL;


UI::UI() : Thread("WinSparkle UI thread")
{
}


void UI::Run()
{
    // Note: The thread that called UI::Get() holds gs_uiThreadCS
    //       at this point and won't release it until we signal it.

    // We need to pass correct HINSTANCE to wxEntry() and the right value is
    // HINSTANCE of this DLL, not of the main .exe.

    if ( !ms_hInstance )
        return; // DllMain() not called? -- FIXME: throw

    // IMPLEMENT_WXWIN_MAIN does this as the first thing
    wxDISABLE_DEBUG_SUPPORT();

    // We do this before wxEntry() explicitly, even though wxEntry() would
    // do it too, so that we know when wx is initialized and can signal
    // run_wx_gui_from_dll() about it *before* starting the event loop.
    wxInitializer wxinit;
    if ( !wxinit.IsOk() )
        return; // failed to init wx -- FIXME: throw

    // Signal run_wx_gui_from_dll() that it can continue
    SignalReady();

    // Run the app:
    wxMSWDisableSettingHighDPIAware();
    wxEntry(ms_hInstance);
}


/*static*/
void UI::ShutDown()
{
    UIThreadAccess uit;

    if ( !uit.IsRunning() )
        return;

    uit.App().SendMsg(MSG_TERMINATE);
    uit.ShutDownThread();
}


/*static*/
void UI::NotifyNoUpdates()
{
    UIThreadAccess uit;

    if ( !uit.IsRunning() )
        return;

    uit.App().SendMsg(MSG_NO_UPDATE_FOUND);
}


/*static*/
void UI::NotifyUpdateAvailable(const Appcast& info)
{
    UIThreadAccess uit;
    EventPayload payload;
    payload.appcast = info;
    uit.App().SendMsg(MSG_UPDATE_AVAILABLE, &payload);
}


/*static*/
void UI::NotifyDownloadProgress(size_t downloaded, size_t total)
{
    UIThreadAccess uit;
    EventPayload payload;
    payload.sizeDownloaded = downloaded;
    payload.sizeTotal = total;
    uit.App().SendMsg(MSG_DOWNLOAD_PROGRESS, &payload);
}


/*static*/
void UI::NotifyUpdateDownloaded(const std::wstring& updateFile, const Appcast &appcast)
{
    UIThreadAccess uit;
    EventPayload payload;
    payload.updateFile = updateFile;
    payload.appcast = appcast;
    uit.App().SendMsg(MSG_UPDATE_DOWNLOADED, &payload);
}


/*static*/
void UI::NotifyUpdateError()
{
    UIThreadAccess uit;

    if ( !uit.IsRunning() )
        return;

    uit.App().SendMsg(MSG_UPDATE_ERROR);
}


/*static*/
void UI::ShowCheckingUpdates()
{
    UIThreadAccess uit;
    uit.App().SendMsg(MSG_SHOW_CHECKING_UPDATES);
}


/*static*/
void UI::AskForPermission()
{
    UIThreadAccess uit;
    uit.App().SendMsg(MSG_ASK_FOR_PERMISSION);
}

} // namespace winsparkle
