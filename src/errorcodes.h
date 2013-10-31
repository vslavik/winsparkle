#pragma once
#ifndef _errorcodes_h_
#define _errorcodes_h_
namespace winsparkle
{
//Not really an "error"code but whatever...
extern const int SUCCESS_ERROR;

extern const int WIN32_ERROR;

extern const int XML_PARSER_ERROR;
extern const int XML_PARSER_CREATION_ERROR;

extern const int FILE_NOT_FOUND_ERROR;

extern const int NO_TRANSLATIONS_FOUND_ERROR;

extern const int APPCAST_URL_NOT_SPECIFIED_ERROR;

//UpdateDownloaderErrors:
extern const int UPDATE_FILE_ALREADY_SET_ERROR;
extern const int UPDATE_UNABLE_TO_SAVE_FILE_ERROR;
extern const int UPDATE_FILE_NOT_SET_ERROR;

extern const int WIN_SPARKLE_SILENT_MODE_NOT_INITIALIZED_ERROR;

extern const int UNABLE_TO_LAUNCH_INSTALLER_ERROR;

extern const int UNKNOWN_ERROR;

} //namespace winsparkle
#endif //_errorcodes_h_