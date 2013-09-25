#include "hostpoller.h"

namespace winsparkle
{

HostPoller::HostPoller() 
    : Thread("WinSparkle HostPoller")
{
}

void HostPoller::Run()
{
    SignalReady();

    while(!UI::IsHostReadyToShutDown())
    {
        //sleep for half a second and try again until we are successful
        Sleep(500);
    }
    //Now we are clear to run the installer and exit the app
    UI::ExecuteInstaller();
}


} //namespace winsparkle