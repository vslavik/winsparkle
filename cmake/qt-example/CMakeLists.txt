if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
endif()

project(QtExampleApp)

cmake_minimum_required(VERSION 2.8.12)

find_package(WinSparkle REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})

get_filename_component(ROOT_DIR ${CMAKE_SOURCE_DIR}/../.. REALPATH)
set(SOURCE_DIR ${ROOT_DIR}/examples/qt)

include_directories(${SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(SOURCES
  ${SOURCE_DIR}/main.cpp
  ${SOURCE_DIR}/mainwindow.cpp)

set(HEADERS
  ${SOURCE_DIR}/mainwindow.h)

set(FORMS
  ${SOURCE_DIR}/mainwindow.ui)

qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
qt5_wrap_ui(UI_SOURCES ${FORMS})

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES})
target_link_libraries(${PROJECT_NAME} WinSparkle Qt5::Core Qt5::Gui Qt5::Widgets)
