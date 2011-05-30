/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2010 Vaclav Slavik
 *  Copyright (C) 2011 Vasco Veloso
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

#ifndef _updater_h_
#define _updater_h_

#include "ui.h"
#include "download.h"

#define wxNO_NET_LIB
#define wxNO_XML_LIB
#define wxNO_XRC_LIB
#define wxNO_ADV_LIB
#define wxNO_HTML_LIB
#include <wx/setup.h>

#include <wx/file.h>
#include <wx/thread.h>
#include <wx/dialog.h>

namespace winsparkle
{
/**
    This class downloads an update file.
 */
class Updater : public wxThread, public IDownloadSink
{
public:
    Updater(const Appcast &appcast, wxDialog *notificationDlg);

	void RequestStop();

	// IDownloadSink methods
	bool Add(const void *data, size_t len);
	void SetTotalLength(size_t length);

	static void RunUpdate(const wxString updateFilePath);

	static int PROGRESS_PERCENT_UPDATE;
	static int UPDATE_COMPLETE;
	static int UPDATE_CANCELLED;

private:
	bool m_stop;
	wxString m_updateFileUrl;
	wxString m_updateFileName;
	wxFile m_file;
	wxDialog *m_notify;
	size_t m_totalLength;
	size_t m_lengthReceived;

public:
    virtual ExitCode Entry();
};

} // namespace winsparkle

#endif // _updatechecker_h_
