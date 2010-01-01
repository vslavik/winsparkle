/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009 Vaclav Slavik
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

// silence warnings in wxWidgets' CRT wrappers
#define _CRT_SECURE_NO_WARNINGS

#include "ui.h"
#include "settings.h"

#define wxNO_NET_LIB
#define wxNO_XML_LIB
#define wxNO_XRC_LIB
#define wxNO_ADV_LIB
#define wxNO_HTML_LIB
#include <wx/setup.h>

#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/settings.h>

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

// Locks window updates to reduce flicker. Redoes layout in dtor.
struct LayoutChangesGuard
{
    LayoutChangesGuard(wxWindow *win) : m_win(win)
    {
        m_win->Freeze();
    }

    ~LayoutChangesGuard()
    {
        m_win->Layout();
        m_win->GetSizer()->SetSizeHints(m_win);
        m_win->Refresh();
        m_win->Thaw();
    }

    wxWindow *m_win;
};


// shows/hides layout element
void DoShowElement(wxWindow *w, bool show)
{
    w->GetContainingSizer()->Show(w, show, true/*recursive*/);
}

void DoShowElement(wxSizer *s, bool show)
{
    s->ShowItems(show);
}


} // anonymous namespace


/*--------------------------------------------------------------------------*
                       Window for communicating with the user
 *--------------------------------------------------------------------------*/

const int ID_SKIP_VERSION = wxNewId();
const int ID_REMIND_LATER = wxNewId();
const int ID_INSTALL = wxNewId();

class UpdateDialog : public wxDialog
{
public:
    UpdateDialog();

    // changes state into "checking for updates"
    void StateCheckingUpdates();
    // change state into "no updates found"
    void StateNoUpdateFound();
    // change state into "a new version is available"
    void StateUpdateAvailable(const Appcast& info);

private:
    void EnablePulsing(bool enable);
    void OnTimer(wxTimerEvent& event);
    void OnCloseButton(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnSkipVersion(wxCommandEvent&);
    void OnRemindLater(wxCommandEvent&);
    void OnInstall(wxCommandEvent&);

    void SetMessage(const wxString& text);

private:
    wxTimer       m_timer;
    wxSizer      *m_infoSizer;
    wxSizer      *m_buttonSizer;
    wxStaticText *m_heading;
    wxStaticText *m_message;
    wxGauge      *m_progress;
    wxButton     *m_closeButton;
    wxSizer      *m_closeButtonSizer;
    wxSizer      *m_updateButtonsSizer;

    // current appcast data (only valid after StateUpdateAvailable())
    Appcast       m_appcast;

    static const int MESSAGE_AREA_WIDTH = 300;
};

UpdateDialog::UpdateDialog()
    : wxDialog(NULL, wxID_ANY, _("Software Update"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_timer(this)
{
    SetIcons(wxICON(UpdateAvailable));

    wxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxIcon bigIcon("UpdateAvailable", wxBITMAP_TYPE_ICO_RESOURCE, 48, 48);
    topSizer->Add
              (
                  new wxStaticBitmap(this, wxID_ANY, bigIcon),
                  wxSizerFlags(0).Border(wxALL, 10)
              );

    m_infoSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(m_infoSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));

    m_heading = new wxStaticText(this, wxID_ANY, "");
    wxFont bold = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    bold.SetPointSize(bold.GetPointSize() + 1);
    bold.SetWeight(wxFONTWEIGHT_BOLD);
    m_heading->SetFont(bold);
    m_infoSizer->Add(m_heading, wxSizerFlags(0).Expand().Border(wxBOTTOM, 10));

    m_message = new wxStaticText(this, wxID_ANY, "",
                                 wxDefaultPosition, wxSize(MESSAGE_AREA_WIDTH, -1));
    m_infoSizer->Add(m_message, wxSizerFlags(0).Expand());

    m_progress = new wxGauge(this, wxID_ANY, 100,
                             wxDefaultPosition, wxSize(MESSAGE_AREA_WIDTH, 16));
    m_infoSizer->Add(m_progress, wxSizerFlags(0).Expand().Border(wxTOP, 10));
    m_infoSizer->AddStretchSpacer(1);

    m_buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    m_updateButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    // FIXME: enable this button once it is implemented
#if 0
    m_updateButtonsSizer->Add
                          (
                            new wxButton(this, ID_SKIP_VERSION, _("Skip this version")),
                            wxSizerFlags().Border(wxRIGHT, 20)
                          );
#endif
    m_updateButtonsSizer->AddStretchSpacer(1);
    m_updateButtonsSizer->Add
                          (
                            new wxButton(this, ID_REMIND_LATER, _("Remind me later")),
                            wxSizerFlags().Border(wxRIGHT, 10)
                          );
    m_updateButtonsSizer->Add
                          (
                            new wxButton(this, ID_INSTALL, _("Install update")),
                            wxSizerFlags()
                          );
    m_buttonSizer->Add(m_updateButtonsSizer, wxSizerFlags(1));

    m_closeButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_closeButton = new wxButton(this, wxID_CANCEL);
    m_closeButtonSizer->AddStretchSpacer(1);
    m_closeButtonSizer->Add(m_closeButton, wxSizerFlags(0).Border(wxLEFT));
    m_buttonSizer->Add(m_closeButtonSizer, wxSizerFlags(1));

    m_infoSizer->Add
                 (
                     m_buttonSizer,
                     wxSizerFlags(0).Expand().Border(wxTOP, 20)
                 );

    SetSizerAndFit(topSizer);

    Bind(wxEVT_CLOSE_WINDOW, &UpdateDialog::OnClose, this);
    Bind(wxEVT_TIMER, &UpdateDialog::OnTimer, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnCloseButton, this, wxID_CANCEL);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnSkipVersion, this, ID_SKIP_VERSION);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnRemindLater, this, ID_REMIND_LATER);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnInstall, this, ID_INSTALL);
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
    // We need to override this, because by default, wxDialog doesn't
    // destroy itself in Close().
    Destroy();
}


void UpdateDialog::OnSkipVersion(wxCommandEvent&)
{
    // FIXME: record that this version should be skipped
}


void UpdateDialog::OnRemindLater(wxCommandEvent&)
{
    // Just abort the update. Next time it's scheduled to run,
    // the user will be prompted.
    Close();
}


void UpdateDialog::OnInstall(wxCommandEvent&)
{
    // FIXME: download the file within WinSparkle UI, stop the app,
    // elevate privileges, launch the installer
    wxLaunchDefaultBrowser(m_appcast.DownloadURL);
}


void UpdateDialog::SetMessage(const wxString& text)
{
    m_message->SetLabel(text);
    m_message->Wrap(MESSAGE_AREA_WIDTH);
}


#define SHOW(c)    DoShowElement(c, true)
#define HIDE(c)    DoShowElement(c, false)

void UpdateDialog::StateCheckingUpdates()
{
    LayoutChangesGuard guard(this);

    SetMessage(_("Checking for updates..."));

    m_closeButton->SetLabel(_("Cancel"));
    EnablePulsing(true);

    HIDE(m_heading);
    SHOW(m_progress);
    SHOW(m_closeButtonSizer);
    HIDE(m_updateButtonsSizer);
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
                  Settings::Get().GetAppName(),
                  Settings::Get().GetAppVersion()
              );
    }
    catch ( std::exception& )
    {
        // GetAppVersion() may fail
        msg = "Error: Updates checking not properly configured.";
    }

    SetMessage(msg);

    m_closeButton->SetLabel(_("Close"));
    EnablePulsing(false);

    SHOW(m_heading);
    HIDE(m_progress);
    SHOW(m_closeButtonSizer);
    HIDE(m_updateButtonsSizer);
}


void UpdateDialog::StateUpdateAvailable(const Appcast& info)
{
    m_appcast = info;

    LayoutChangesGuard guard(this);

    const wxString appname = Settings::Get().GetAppName();

    m_heading->SetLabel(
        wxString::Format(_("A new version of %s is available!"), appname));

    SetMessage
    (
        wxString::Format
        (
            _("%s %s is now available (you have %s). Would you like to download it now?"),
            appname,
            info.Version,
            Settings::Get().GetAppVersion()
        )
    );

    EnablePulsing(false);

    SHOW(m_heading);
    HIDE(m_progress);
    HIDE(m_closeButtonSizer);
    SHOW(m_updateButtonsSizer);
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


/*--------------------------------------------------------------------------*
                                Application
 *--------------------------------------------------------------------------*/

class App : public wxApp
{
public:
    App();

    // Sends a message with ID @a msg to the app.
    void SendMsg(int msg, void *data = NULL);

private:
    void InitWindow();
    void ShowWindow();

    void OnWindowClose(wxCloseEvent& event);
    void OnTerminate(wxThreadEvent& event);
    void OnShowCheckingUpdates(wxThreadEvent& event);
    void OnNoUpdateFound(wxThreadEvent& event);
    void OnUpdateAvailable(wxThreadEvent& event);

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
}


void App::SendMsg(int msg, void *data)
{
    wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_THREAD, msg);
    if ( data )
        event->SetClientData(data);

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
    ExitMainLoop();
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


void App::OnUpdateAvailable(wxThreadEvent& event)
{
    InitWindow();

    Appcast *appcast = static_cast<Appcast*>(event.GetClientData());

    m_win->StateUpdateAvailable(*appcast);

    delete appcast;

    ShowWindow();
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
    uit.App().SendMsg(MSG_UPDATE_AVAILABLE, new Appcast(info));
}


/*static*/
void UI::ShowCheckingUpdates()
{
    UIThreadAccess uit;
    uit.App().SendMsg(MSG_SHOW_CHECKING_UPDATES);
}

} // namespace winsparkle
