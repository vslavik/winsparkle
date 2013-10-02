#pragma once
#ifndef _ghostupdater_h_

#include "appcast.h"
#include "updatechecker.h"
#include "updatedownloader.h"

namespace winsparkle
{

class GhostUpdater
{
public:
    GhostUpdater(void (*checkCallback)(int, int), void (*downloadCallback)(int)) :
    m_checkCallback(checkCallback), m_downloadCallback(downloadCallback),
    m_updateChecker(NULL), m_updateDownloader(NULL)
    {}

    ~GhostUpdater();

    //Spawn a SilentUpdateThread and go nuts
    void CheckForUpdates();
    //Spawn a SilentDownloadThread and go nuts
    void DownloadUpdates();
    //Launch the installer
    int RunInstaller();

private:
    Appcast m_appcast;
    void (*m_checkCallback)(int, int);
    void (*m_downloadCallback)(int);

    SilentUpdateChecker* m_updateChecker;
    SilentUpdateDownloader* m_updateDownloader;
};



} // namespace winsparkle
#endif // _ghostupdater_h_