cmake_minimum_required(VERSION 2.8.12)
project(wxWidgets)

if(POLICY CMP0059)
    cmake_policy(SET CMP0059 OLD)
endif()

set(SOURCE_DIR ${ROOT_DIR}/3rdparty)

set(INCLUDE_DIRS ${SOURCE_DIR}/wxWidgets_setup_h ${SOURCE_DIR}/wxWidgets/include)
include_directories(${INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(
  -D_CRT_SECURE_NO_WARNINGS
  -DWX_PRECOMP
  -DWXBUILDING
  -DWIN32
  -DNDEBUG
  -D_LIB
  -DUNICODE -D_UNICODE)

set(SOURCES
  ${SOURCE_DIR}/wxWidgets/src/common/any.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/appbase.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/appcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/arrstr.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/artprov.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/bmpbase.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/btncmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/checkboxcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/clntdata.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/cmdline.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/cmdproc.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/colourcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/config.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/containr.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/convauto.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/ctrlcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/ctrlsub.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dcbase.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dcbufcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dircmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dlgcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dobjcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dpycmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dynarray.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/dynlib.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/encconv.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/event.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/evtloopcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/file.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/filefn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/filename.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/fmapbase.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/fontcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/fontmap.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/framecmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/gaugecmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/gbsizer.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/gdicmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/hash.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/hashmap.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/iconbndl.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/image.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/init.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/intl.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/ipcbase.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/languageinfo.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/layout.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/list.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/longlong.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/matrix.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/menucmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/modalhook.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/module.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/mousemanager.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/msgout.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/numformatter.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/object.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/overlaycmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/panelcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/platinfo.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/powercmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/process.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/rendcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/rgncmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/settcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/sizer.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/statbmpcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/stattextcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/stdpbase.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/stockitem.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/strconv.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/string.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/stringimpl.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/stringops.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/strvararg.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/sysopt.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/textbuf.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/textcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/textmeasurecmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/threadinfo.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/time.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/timercmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/timerimpl.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/tokenzr.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/toplvcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/translation.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/unichar.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/uri.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/ustring.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/utilscmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/variant.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/wincmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/windowid.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/wrapsizer.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/wxcrt.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/ownerdrwcmn.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/msgdlgg.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/renderg.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/scrlwing.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/selstore.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/statbmpg.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/stattextg.cpp
  ${SOURCE_DIR}/wxWidgets/src/generic/vscroll.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/anybutton.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/app.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/artmsw.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/basemsw.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/bitmap.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/brush.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/button.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/checkbox.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/colour.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/control.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/cursor.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dc.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dcclient.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dcmemory.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dcscreen.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/debughlp.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dialog.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dib.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dir.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/display.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/dlmsw.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/evtloop.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/evtloopconsole.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/font.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/fontutil.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/frame.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/gauge.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/gdiimage.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/icon.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/imaglist.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/main.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/menu.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/menuitem.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/msgdlg.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/nativdlg.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/nativewin.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/nonownedwnd.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ole/activex.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ole/comimpl.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ole/dataobj.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ole/oleutils.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ole/safearray.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ole/uuid.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/pen.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/power.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/region.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/registry.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/renderer.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/settings.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/statbmp.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/stattext.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/stdpaths.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/textmeasure.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/thread.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/timer.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/toplevel.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/utils.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/utilsexc.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/utilsgui.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/utilswin.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/uxtheme.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/window.cpp
  ${SOURCE_DIR}/wxWidgets/src/msw/ownerdrw.cpp
  ${SOURCE_DIR}/wxWidgets/src/common/wxprintf.cpp)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})
set(wxWidgets_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(PCHSupport)
if(NOT PCHSupport_FOUND)
  message(FATAL_ERROR "Precompiled headers are not supported")
endif()

ADD_PRECOMPILED_HEADER(${PROJECT_NAME} ${SOURCE_DIR}/wxWidgets/include/wx/wxprec.h)
