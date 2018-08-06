/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2018 Vaclav Slavik
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

#include "appcast.h"
#include "error.h"

#include <expat.h>
#include <vector>
#include <algorithm>
#include <windows.h>

namespace winsparkle
{

/*--------------------------------------------------------------------------*
                                XML parsing
 *--------------------------------------------------------------------------*/

namespace
{

#define MVAL(x) x
#define CONCAT3(a,b,c) MVAL(a)##MVAL(b)##MVAL(c)

#define NS_SPARKLE      "http://www.andymatuschak.org/xml-namespaces/sparkle"
#define NS_SEP          '#'
#define NS_SPARKLE_NAME(name) NS_SPARKLE "#" name

#define NODE_CHANNEL    "channel"
#define NODE_ITEM       "item"
#define NODE_RELNOTES   NS_SPARKLE_NAME("releaseNotesLink")
#define NODE_TITLE "title"
#define NODE_DESCRIPTION "description"
#define NODE_LINK       "link"
#define NODE_ENCLOSURE  "enclosure"
#define NODE_MIN_OS_VERSION NS_SPARKLE_NAME("minimumSystemVersion")
#define ATTR_URL        "url"
#define ATTR_VERSION    NS_SPARKLE_NAME("version")
#define ATTR_SHORTVERSION NS_SPARKLE_NAME("shortVersionString")
#define ATTR_DSASIGNATURE NS_SPARKLE_NAME("dsaSignature")
#define ATTR_OS         NS_SPARKLE_NAME("os")
#define ATTR_ARGUMENTS  NS_SPARKLE_NAME("installerArguments")
#define NODE_VERSION      ATTR_VERSION        // These can be nodes or
#define NODE_SHORTVERSION ATTR_SHORTVERSION   // attributes.
#define NODE_DSASIGNATURE ATTR_DSASIGNATURE
#define OS_MARKER       "windows"
#define OS_MARKER_LEN   7

// context data for the parser
struct ContextData
{
    ContextData(XML_Parser& p)
        : parser(p),
        in_channel(0), in_item(0), in_relnotes(0), in_title(0), in_description(0), in_link(0),
        in_version(0), in_shortversion(0), in_dsasignature(0), in_min_os_version(0)
    {}

    // the parser we're using
    XML_Parser& parser;

    // is inside <channel>, <item> or <sparkle:releaseNotesLink>, <title>, <description>, or <link> respectively?
    int in_channel, in_item, in_relnotes, in_title, in_description, in_link;

    // is inside <sparkle:version> or <sparkle:shortVersionString> node?
    int in_version, in_shortversion, in_dsasignature, in_min_os_version;

    // parsed <item>s
    std::vector<Appcast> items;
};

bool is_windows_version_acceptable(const Appcast &item)
{
    if (item.MinOSVersion.empty())
        return true;

    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(
        VerSetConditionMask(
        VerSetConditionMask(
        0, VER_MAJORVERSION, VER_GREATER_EQUAL),
        VER_MINORVERSION, VER_GREATER_EQUAL),
        VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

    sscanf(item.MinOSVersion.c_str(), "%lu.%lu.%hu", &osvi.dwMajorVersion,
        &osvi.dwMinorVersion, &osvi.wServicePackMajor);

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION |
        VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

void XMLCALL OnStartElement(void *data, const char *name, const char **attrs)
{
    ContextData& ctxt = *static_cast<ContextData*>(data);

    if ( strcmp(name, NODE_CHANNEL) == 0 )
    {
        ctxt.in_channel++;
    }
    else if ( ctxt.in_channel && strcmp(name, NODE_ITEM) == 0 )
    {
        ctxt.in_item++;
        Appcast item;
        ctxt.items.push_back(item);
    }
    else if ( ctxt.in_item )
    {
        if ( strcmp(name, NODE_RELNOTES) == 0 )
        {
            ctxt.in_relnotes++;
        }
        else if ( strcmp(name, NODE_TITLE) == 0 )
        {
            ctxt.in_title++;
        }
        else if ( strcmp(name, NODE_DESCRIPTION) == 0 )
        {
            ctxt.in_description++;
        }
        else if ( strcmp(name, NODE_LINK) == 0 )
        {
            ctxt.in_link++;
        }
        else if ( strcmp(name, NODE_VERSION) == 0 )
        {
            ctxt.in_version++;
        }
        else if ( strcmp(name, NODE_SHORTVERSION) == 0 )
        {
            ctxt.in_shortversion++;
        }
        else if (strcmp(name, NODE_DSASIGNATURE) == 0)
        {
            ctxt.in_dsasignature++;
        }
        else if (strcmp(name, NODE_MIN_OS_VERSION) == 0)
        {
            ctxt.in_min_os_version++;
        }
        else if (strcmp(name, NODE_ENCLOSURE) == 0)
        {
            const size_t size = ctxt.items.size();
            for ( int i = 0; attrs[i]; i += 2 )
            {
                const char *name = attrs[i];
                const char *value = attrs[i+1];

                if ( strcmp(name, ATTR_URL) == 0 )
                    ctxt.items[size-1].DownloadURL = value;
                else if ( strcmp(name, ATTR_VERSION) == 0 )
                    ctxt.items[size-1].Version = value;
                else if ( strcmp(name, ATTR_SHORTVERSION) == 0 )
                    ctxt.items[size-1].ShortVersionString = value;
                else if ( strcmp(name, ATTR_DSASIGNATURE) == 0 )
                    ctxt.items[size-1].DsaSignature = value;
                else if ( strcmp(name, ATTR_OS) == 0 )
                    ctxt.items[size-1].Os = value;
                else if ( strcmp(name, ATTR_ARGUMENTS) == 0 )
                    ctxt.items[size-1].InstallerArguments = value;
            }
        }
    }
}


/**
 * Returns true if item os is exactly "windows"
 *   or if item is "windows-x64" on 64bit
 *   or if item is "windows-x86" on 32bit
 *   and is above minimum version
 */
bool is_suitable_windows_item(const Appcast &item)
{
    if (!is_windows_version_acceptable(item))
        return false;

    if (item.Os == OS_MARKER)
        return true;

    if (item.Os.compare(0, OS_MARKER_LEN, OS_MARKER) != 0)
        return false;

    // Check suffix for matching bitness
#ifdef _WIN64
    return item.Os.compare(OS_MARKER_LEN, std::string::npos, "-x64") == 0;
#else
    return item.Os.compare(OS_MARKER_LEN, std::string::npos, "-x86") == 0;
#endif
}


void XMLCALL OnEndElement(void *data, const char *name)
{
    ContextData& ctxt = *static_cast<ContextData*>(data);

    if (ctxt.in_item)
    {
        if (strcmp(name, NODE_RELNOTES) == 0)
        {
            ctxt.in_relnotes--;
        }
        else if (strcmp(name, NODE_TITLE) == 0)
        {
            ctxt.in_title--;
        }
        else if (strcmp(name, NODE_DESCRIPTION) == 0)
        {
            ctxt.in_description--;
        }
        else if (strcmp(name, NODE_MIN_OS_VERSION) == 0)
        {
            ctxt.in_min_os_version--;
        }
        else if (strcmp(name, NODE_LINK) == 0)
        {
            ctxt.in_link--;
        }
        else if (strcmp(name, NODE_VERSION) == 0)
        {
            ctxt.in_version--;
        }
        else if (strcmp(name, NODE_SHORTVERSION) == 0)
        {
            ctxt.in_shortversion--;
        }
        else if (strcmp(name, NODE_DSASIGNATURE) == 0)
        {
            ctxt.in_dsasignature--;
        }
    }
    else if (ctxt.in_channel && strcmp(name, NODE_ITEM) == 0)
    {
        ctxt.in_item--;
        if (is_suitable_windows_item(ctxt.items[ctxt.items.size() - 1]))
            XML_StopParser(ctxt.parser, XML_TRUE);
    }
    else if (strcmp(name, NODE_CHANNEL) == 0 )
    {
        ctxt.in_channel--;
        // we've reached the end of <channel> element,
        // so we stop parsing
        XML_StopParser(ctxt.parser, XML_TRUE);
    }
}


void XMLCALL OnText(void *data, const char *s, int len)
{
    ContextData& ctxt = *static_cast<ContextData*>(data);
    const size_t size = ctxt.items.size();

    if ( ctxt.in_relnotes )
        ctxt.items[size-1].ReleaseNotesURL.append(s, len);
    else if ( ctxt.in_title )
        ctxt.items[size-1].Title.append(s, len);
    else if ( ctxt.in_description )
        ctxt.items[size-1].Description.append(s, len);
    else if ( ctxt.in_link )
        ctxt.items[size-1].WebBrowserURL.append(s, len);
    else if ( ctxt.in_version )
        ctxt.items[size-1].Version.append(s, len);
    else if ( ctxt.in_shortversion )
        ctxt.items[size-1].ShortVersionString.append(s, len);
    else if (ctxt.in_dsasignature)
        ctxt.items[size-1].DsaSignature.append(s, len);
    else if ( ctxt.in_min_os_version )
        ctxt.items[size-1].MinOSVersion.append(s, len);
}

} // anonymous namespace


/*--------------------------------------------------------------------------*
                               Appcast class
 *--------------------------------------------------------------------------*/

Appcast Appcast::Load(const std::string& xml)
{
    XML_Parser p = XML_ParserCreateNS(NULL, NS_SEP);
    if ( !p )
        throw std::runtime_error("Failed to create XML parser.");

    ContextData ctxt(p);

    XML_SetUserData(p, &ctxt);
    XML_SetElementHandler(p, OnStartElement, OnEndElement);
    XML_SetCharacterDataHandler(p, OnText);

    XML_Status st = XML_Parse(p, xml.c_str(), (int)xml.size(), XML_TRUE);

    if ( st == XML_STATUS_ERROR )
    {
        std::string msg("XML parser error: ");
        msg.append(XML_ErrorString(XML_GetErrorCode(p)));
        XML_ParserFree(p);
        throw std::runtime_error(msg);
    }

    XML_ParserFree(p);

    if (ctxt.items.empty())
        return Appcast(); // invalid

    /*
     * Search for first <item> which specifies with the attribute sparkle:os set to "windows"
     * or "windows-x64"/"windows-x86" based on this modules bitness and meets the minimum
     * os version, if set. If none, use the first item that meets the minimum os version, if set.
     */
    std::vector<Appcast>::iterator it = std::find_if(ctxt.items.begin(), ctxt.items.end(), is_suitable_windows_item);
    if (it != ctxt.items.end())
        return *it;
    else
    {
        it = std::find_if(ctxt.items.begin(), ctxt.items.end(), is_windows_version_acceptable);
        if (it != ctxt.items.end())
            return *it;
        else 
            return Appcast(); // There are no items that meet the set minimum os version
    }
}

} // namespace winsparkle
