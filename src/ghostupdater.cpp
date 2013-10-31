#include "ghostupdater.h"
#include "ui.h"
#include "errorcodes.h"

namespace winsparkle
{

GhostUpdater::~GhostUpdater()
{
    if( m_updateChecker )
    {
        m_updateChecker->TerminateAndJoin();
        delete m_updateChecker;
        m_updateChecker = NULL;
    }

    if( m_updateDownloader )
    {
        m_updateDownloader->TerminateAndJoin();
        delete m_updateDownloader;
        m_updateDownloader = NULL;
    }
}

void GhostUpdater::CheckForUpdates()
{
    if( m_updateChecker == NULL )
    {
        m_updateChecker = new SilentUpdateChecker(m_checkCallback);
        m_updateChecker->Start();
    }
}

void GhostUpdater::DownloadUpdates()
{
    if( m_updateDownloader == NULL )
    {
        m_updateDownloader = new SilentUpdateDownloader(m_updateChecker->GetAppcast(), m_downloadCallback);
        m_updateDownloader->Start();
    }
}

int GhostUpdater::RunInstaller()
{
    if( m_updateDownloader == NULL )
    {
        return 9; //TODO: replace with proper error code
    }
    std::string filePath = m_updateDownloader->GetFilePath();
    if(32 >= (int) ShellExecute(0, 0,
        std::wstring(filePath.begin(), filePath.end()).c_str(),
        0, 0,
        SW_SHOW
        )
    )
    {
        return UNABLE_TO_LAUNCH_INSTALLER_ERROR;
    }
    else
    {
        return SUCCESS_ERROR;
    }
}


}// namespace winsparkle