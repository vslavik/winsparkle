/////////////////////////////////////////////////////////////////////////////
// Name:        src/cocoa/main.cpp
// Purpose:     Entry point
// Author:      David Elliott
// Modified by:
// Created:     2002/11/11
// RCS-ID:      $Id: main.cpp 42046 2006-10-16 09:30:01Z ABX $
// Copyright:   (c) 2002 David Elliott
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/* DFE: ^^^^^ Was that really necessary :-) */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

WXDLLEXPORT int main(int argc, char* argv[])
{
    return wxEntry(argc, argv);
}
