#pragma once
#ifndef _hostpoller_h
#define _hostpoller_h

#include "threads.h"
#include "ui.h"

namespace winsparkle
{
class HostPoller : public Thread
{
public:
    HostPoller();

protected:
    virtual void Run();
    virtual bool IsJoinable() const { return true; }

};

} //namespace winsparkle
#endif //_hostpoller_h