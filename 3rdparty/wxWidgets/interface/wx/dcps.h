/////////////////////////////////////////////////////////////////////////////
// Name:        dcps.h
// Purpose:     interface of wxPostScriptDC
// Author:      wxWidgets team
// RCS-ID:      $Id: dcps.h 57991 2009-01-10 22:10:54Z FM $
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

/**
    @class wxPostScriptDC

    This defines the wxWidgets Encapsulated PostScript device context, which
    can write PostScript files on any platform. See wxDC for descriptions of
    the member functions.

    @library{wxbase}
    @category{dc}
*/
class wxPostScriptDC : public wxDC
{
public:
    /**
        Constructs a PostScript printer device context from a wxPrintData object.
    */
    wxPostScriptDC(const wxPrintData& printData);

    /**
        Return resolution used in PostScript output.

        @see SetResolution()
    */
    virtual int GetResolution() const;

    /**
        Set resolution (in pixels per inch) that will be used in PostScript
        output. Default is 720ppi.
    */
    virtual wxRect GetPaperRect() const;
};

