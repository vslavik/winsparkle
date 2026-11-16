/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2010-2026 Vaclav Slavik
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

#include "ui_webview.h"

#include <wx/sizer.h>
#include <wx/msw/webview_ie.h>


namespace winsparkle
{

EmbeddedWebView::EmbeddedWebView(wxWindow *parent, const wxSize& size)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, size)
{
    SetBackgroundColour(*wxWHITE);
}

void EmbeddedWebView::EnsureBrowser()
{
    if (m_browser)
        return;

    wxBusyCursor busy;

    if (!wxWebView::IsBackendAvailable(wxWebViewBackendEdge))
        wxWebViewIE::MSWSetEmulationLevel(wxWEBVIEWIE_EMU_IE11);

    // Note that about:blank is used so that subsequent SetPage() call doesn't fail with
    // MSIE. See https://github.com/vslavik/winsparkle/issues/155 and
    // https://github.com/wxWidgets/wxWidgets/issues/14768
    m_browser = wxWebView::New(this, wxID_ANY, "about:blank", wxDefaultPosition, GetClientSize());

    m_browser->SetBackgroundColour(GetBackgroundColour());

    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_browser, wxSizerFlags(1).Expand());
    this->SetSizer(sizer);

    // The subsequent code's purpose to is to reliably ensure that external links in the release
    // notes are opened in the default browser, and not in the embedded web view. This is surprisingly
    // complicated, because we need to ensure that the release notes page itself won't open
    // externally as well. And the two supported backends (MSIE and Edge) behave wildly differently.
    //
    // Edge:
    //
    //    ReleaseNotesURL:
    //        1. wxEVT_WEBVIEW_NAVIGATING(ReleaseNotesURL)
    //        2. wxEVT_WEBVIEW_LOADED(ReleaseNotesURL)
    //
    //    Description only (inline notes):
    //        1. wxEVT_WEBVIEW_NAVIGATING("data:text/html;...")
    //        2. wxEVT_WEBVIEW_LOADED("about:blank")
    //
    // MSIE:
    //
    //    ReleaseNotesURL:
    //        1. wxEVT_WEBVIEW_LOADED("about:blank")
    //        2. wxEVT_WEBVIEW_NAVIGATING(ReleaseNotesURL)
    //        3. wxEVT_WEBVIEW_LOADED(ReleaseNotesURL)
    //
    //    Description only (inline notes):
    //        1. wxEVT_WEBVIEW_LOADED("")
    //
    // To further complicate matters, the ReleaseNotesURL page may be a redirect, triggering
    // a chain of navigating events before eventually landing at loaded.

    m_browser->Bind(wxEVT_WEBVIEW_LOADED, [this](wxWebViewEvent& evt)
        {
            switch (this->m_webBrowserState.SourceKind)
            {
                case WebBrowserSourceInline:
                    // for inline release notes, loaded event means the page is ready:
                    this->m_webBrowserState.PageLoaded = true;
                    break;

                case WebBrowserSourceRemote:
                    // for remote, we need to filter out internal events like about:blank:
                    if (evt.GetURL().starts_with("http"))
                        this->m_webBrowserState.PageLoaded = true;
                    break;

                case WebBrowserSourceNone:
                    break;
            }
        });

    // Open all links in the default browser once the documented is loaded.
    // Note that wxEVT_WEBVIEW_NAVIGATING is emitted for redirects during page loading
    // as well, as we can only "hijack" the event once the release notes page was loaded
    m_browser->Bind(wxEVT_WEBVIEW_NAVIGATING, [this](wxWebViewEvent& evt)
        {
            auto& url = evt.GetURL();
            if (this->m_webBrowserState.PageLoaded && url.starts_with("http"))
            {
                wxLaunchDefaultBrowser(url);
                evt.Veto();
            }
        });
}

void EmbeddedWebView::LoadURL(const wxString& url)
{
    EnsureBrowser();

    m_webBrowserState.Reset(WebBrowserSourceRemote);
    m_browser->LoadURL(url);
}

void EmbeddedWebView::SetContent(const wxString& content)
{
    EnsureBrowser();

    m_webBrowserState.Reset(WebBrowserSourceInline);
    m_browser->SetPage(content, "");
}

} // namespace winsparkle
