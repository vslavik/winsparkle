/////////////////////////////////////////////////////////////////////////////
// Name:        cocoa/ObjcRef.mm
// Purpose:     wxObjcAutoRefBase implementation
// Author:      David Elliott
// Modified by: 
// Created:     2004/03/28
// RCS-ID:      $Id: ObjcRef.mm 27405 2004-05-23 15:10:40Z JS $
// Copyright:   (c) 2004 David Elliott <dfe@cox.net>
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/cocoa/ObjcRef.h"

#include <Foundation/NSObject.h>

/*static*/ struct objc_object* wxObjcAutoRefBase::ObjcRetain(struct objc_object* obj)
{
    return [obj retain];
}

/*static*/ void wxObjcAutoRefBase::ObjcRelease(struct objc_object* obj)
{
    [obj release];
}

