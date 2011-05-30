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

#define _CRT_SECURE_NO_WARNINGS

#include "ui.h"
#include "updater.h"
#include "error.h"
#include "resources.h"
#include "winsparkle-resources.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>

namespace winsparkle
{

int Updater::PROGRESS_PERCENT_UPDATE = wxNewId();
int Updater::UPDATE_COMPLETE = wxNewId();
int Updater::UPDATE_CANCELLED = wxNewId();

Updater::Updater(const Appcast &appcast, wxDialog *notificationDlg)
{
	m_stop = false;
	m_notify = notificationDlg;
	m_updateFileUrl = appcast.DownloadURL;
	m_updateFileName = wxStandardPaths::Get().GetTempDir() + "\\" + GetURLFileName((const char*)m_updateFileUrl);
}

wxThread::ExitCode Updater::Entry()
{
	try
	{
		if ( !m_file.Create(m_updateFileName, true) )
			return 0;

		DownloadFile((const char*)m_updateFileUrl, this, Download_NoCached);
		
		m_file.Close();

		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_THREAD, m_stop ? UPDATE_CANCELLED : UPDATE_COMPLETE);
		if (evt)
		{
			evt->SetInt(m_stop);
			if (!m_stop)
				evt->SetString(m_updateFileName.c_str());
			wxQueueEvent(m_notify->GetEventHandler(), evt);
		}

		if (m_stop)
			::DeleteFile(m_updateFileName);
	}
	catch (Win32Exception e)
	{
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_THREAD, UPDATE_CANCELLED);
		if (evt)
		{
			evt->SetInt(0);
			wxQueueEvent(m_notify->GetEventHandler(), evt);
		}

		m_file.Close();
		::DeleteFile(m_updateFileName);

		UI::NotifyUpdateError();
	}

	return 0;
}

void Updater::RequestStop()
{
	m_stop = true;
}

bool Updater::Add(const void *data, size_t len)
{
	m_lengthReceived += len;

	wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_THREAD, PROGRESS_PERCENT_UPDATE);
	if (evt)
	{
		evt->SetInt((m_lengthReceived * 100) / m_totalLength);
		wxQueueEvent(m_notify->GetEventHandler(), evt);
	}

	m_stop = (m_stop || !m_file.Write(data, len));

	return !m_stop;
}

void Updater::SetTotalLength(size_t length)
{
	m_totalLength = length;
	m_lengthReceived = 0;
}

void Updater::RunUpdate( const wxString updateFilePath )
{
	wxFile batch;
	wxFileName appFileName(Resources::GetExeFileName());
	wxString batchScript(Resources::LoadString(STR_UPDATE_BATCH));
	wxFileName batchFileName(wxFileName::CreateTempFileName("prk"));
	bool spawned = false;

	::DeleteFile(batchFileName.GetFullPath()); // delete the file that was created automatically with extension .tmp
	batchFileName.SetExt("cmd");

	batchScript.Replace(":appexename", appFileName.GetFullName());
	batchScript.Replace(":setuppath", updateFilePath);
	
	if (batch.Create(batchFileName.GetFullPath(), true))
	{
		if (batch.Write(batchScript))
		{
			batch.Close();
			spawned = ((int)::ShellExecute(HWND_DESKTOP, L"open", batchFileName.GetFullPath(), NULL, NULL, SW_HIDE) > 32);
		}
	}
	if (!spawned)
	{
		batch.Close();
		::DeleteFile(batchFileName.GetFullPath());
	}
}

};
