#include "errorcodes.h"

#include <limits>

namespace winsparkle
{
const int SUCCESS_ERROR = 0;

const int WIN32_ERROR = 1;

const int XML_PARSER_ERROR = 2;
const int XML_PARSER_CREATION_ERROR = 3;

const int FILE_NOT_FOUND_ERROR = 4;

const int NO_TRANSLATIONS_FOUND_ERROR = 5;

const int APPCAST_URL_NOT_SPECIFIED_ERROR = 6;

//UpdateDownloaderErrors:
const int UPDATE_FILE_ALREADY_SET_ERROR = 7;
const int UPDATE_UNABLE_TO_SAVE_FILE_ERROR = 8;
const int UPDATE_FILE_NOT_SET_ERROR = 9;

const int WIN_SPARKLE_SILENT_MODE_NOT_INITIALIZED_ERROR = 10;

const int UNABLE_TO_LAUNCH_INSTALLER_ERROR = 11;

const int UNKNOWN_ERROR = INT_MAX;
} //namespace winsparkle