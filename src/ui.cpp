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

#if !wxCHECK_VERSION(2,9,0)
#error "wxWidgets >= 2.9 is required to compile this code"
#endif

namespace winsparkle
{

/*--------------------------------------------------------------------------*
                       Window for communicating with the user
 *--------------------------------------------------------------------------*/

class UpdateDialog : public wxDialog
{
public:
    UpdateDialog();

    // changes state into "checking for updates"
    void StateCheckingUpdates();
    // change state into "no updates found"
    void StateNoUpdateFound();

private:
    void EnablePulsing(bool enable);
    void OnTimer(wxTimerEvent& event);
    void OnCloseButton(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

private:
    wxTimer       m_timer;
    wxSizer      *m_infoSizer;
    wxSizer      *m_buttonSizer;
    wxStaticText *m_message;
    wxGauge      *m_progress;
    wxButton     *m_closeButton;
};

UpdateDialog::UpdateDialog()
    : wxDialog(NULL, wxID_ANY, _("Software Update"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_timer(this)
{
    SetIcons(wxICON(UpdateAvailable));

    wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxIcon bigIcon("UpdateAvailable", wxBITMAP_TYPE_ICO_RESOURCE, 48, 48);
    topSizer->Add
              (
                  new wxStaticBitmap(this, wxID_ANY, bigIcon),
                  wxSizerFlags(0).Border(wxALL, 10)
              );

    m_infoSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(m_infoSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));

    m_infoSizer->AddStretchSpacer(1);
    m_message = new wxStaticText(this, wxID_ANY, "");
    m_infoSizer->Add(m_message, wxSizerFlags(0).Expand());
    m_progress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(300, 16));
    m_infoSizer->Add(m_progress, wxSizerFlags(0).Expand().Border(wxTOP, 10));
    m_infoSizer->AddStretchSpacer(1);

    m_buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_closeButton = new wxButton(this, wxID_CANCEL);
    m_buttonSizer->Add(m_closeButton, wxSizerFlags(0).Border(wxLEFT));

    mainSizer->Add(topSizer, wxSizerFlags(1).Expand());
    mainSizer->Add
               (
                   m_buttonSizer,
                   wxSizerFlags(0).Right().Border(wxLEFT|wxRIGHT|wxBOTTOM, 10)
               );

    SetSizerAndFit(mainSizer);

    Bind(wxEVT_CLOSE_WINDOW, &UpdateDialog::OnClose, this);
    Bind(wxEVT_TIMER, &UpdateDialog::OnTimer, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UpdateDialog::OnCloseButton, this, wxID_CANCEL);
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



#define SHOW(c)    (c)->GetContainingSizer()->Show(c)
#define HIDE(c)    (c)->GetContainingSizer()->Hide(c)

void UpdateDialog::StateCheckingUpdates()
{
    m_message->SetLabel(_("Checking for updates..."));

    SHOW(m_progress);
    SHOW(m_closeButton);
    m_closeButton->SetLabel(_("Cancel"));
    EnablePulsing(true);

    Layout();
}


void UpdateDialog::StateNoUpdateFound()
{
    wxString msg;
    try
    {
        // FIXME: Add bold "You're up to date!" heading.

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

    m_message->SetLabel(msg);

    HIDE(m_progress);
    m_closeButton->SetLabel(_("Close"));
    EnablePulsing(false);

    Layout();
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


/*--------------------------------------------------------------------------*
                                Application
 *--------------------------------------------------------------------------*/

class App : public wxApp
{
public:
    App();

    // Sends a message with ID @a msg to the app.
    void SendMsg(int msg);

private:
    void ShowWindow();

    void OnWindowClose(wxCloseEvent& event);
    void OnTerminate(wxThreadEvent& event);
    void OnShowCheckingUpdates(wxThreadEvent& event);
    void OnNoUpdateFound(wxThreadEvent& event);

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
}


void App::SendMsg(int msg)
{
    wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_THREAD, msg);
    wxQueueEvent(this, event);
}


void App::ShowWindow()
{
    if ( m_win )
    {
        m_win->Raise();
        return;
    }

    m_win = new UpdateDialog();
    m_win->Bind(wxEVT_CLOSE_WINDOW, &App::OnWindowClose, this);
    m_win->Show();
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
    ShowWindow();
    m_win->StateCheckingUpdates();
}


void App::OnNoUpdateFound(wxThreadEvent&)
{
    if ( m_win )
        m_win->StateNoUpdateFound();
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
void UI::ShowCheckingUpdates()
{
    UIThreadAccess uit;
    uit.App().SendMsg(MSG_SHOW_CHECKING_UPDATES);
}

} // namespace winsparkle
