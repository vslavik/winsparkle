/////////////////////////////////////////////////////////////////////////////
// Name:        src/cocoa/dcscreen.cpp
// Purpose:     wxScreenDCImpl class
// Author:      David Elliott
// Modified by:
// Created:     2003/03/16
// RCS-ID:      $Id: dcscreen.cpp 50462 2007-12-04 04:22:16Z DE $
// Copyright:   (c) 2002 David Elliott
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/cocoa/dcscreen.h"

#ifndef WX_PRECOMP
#endif //WX_PRECOMP

//-----------------------------------------------------------------------------
// wxMemoryDC
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxScreenDCImpl,wxCocoaDCImpl)

wxScreenDCImpl::wxScreenDCImpl(wxScreenDC *owner)
:   wxCocoaDCImpl(owner)
{
    m_ok = false;
}

wxScreenDCImpl::wxScreenDCImpl(wxScreenDC *owner, wxDC *WXUNUSED(dc) )
:   wxCocoaDCImpl(owner)
{
    m_ok = false;
}

wxScreenDCImpl::~wxScreenDCImpl(void)
{
}
