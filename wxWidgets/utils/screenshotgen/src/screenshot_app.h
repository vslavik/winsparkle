/////////////////////////////////////////////////////////////////////////////
// Name:        screenshot_app.h
// Purpose:     Defines the Application Class
// Author:      Utensil Candel (UtensilCandel@@gmail.com)
// RCS-ID:      $Id: screenshot_app.h 56343 2008-10-15 18:49:22Z BP $
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCREENSHOT_APP_H_
#define _SCREENSHOT_APP_H_

#include "wx/app.h"

class ScreenshotApp : public wxApp
{
public:
    virtual bool OnInit();
};

#endif // _SCREENSHOT_APP_H_
