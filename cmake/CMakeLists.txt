# CMAKE_BUILD_TYPE is required for PCHSupport.cmake
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
endif()

project(WinSparkle)

cmake_minimum_required(VERSION 2.8.12)

if(NOT MINGW)
  message(FATAL_ERROR "Can be used only with MinGW")
endif()

cmake_policy(SET CMP0022 NEW)

get_filename_component(ROOT_DIR ${CMAKE_SOURCE_DIR}/.. REALPATH)

# Parse version header to always have actual version of winsparkle
file(READ "${ROOT_DIR}/include/winsparkle-version.h" VERSION_FILE)
string(REGEX REPLACE "^.*#[ \t]*define[ \t]+WIN_SPARKLE_VERSION_MAJOR[ \t]+([0-9]+).*$" "\\1" LIB_MAJOR_VERSION ${VERSION_FILE})
string(REGEX REPLACE "^.*#[ \t]*define[ \t]+WIN_SPARKLE_VERSION_MINOR[ \t]+([0-9]+).*$" "\\1" LIB_MINOR_VERSION ${VERSION_FILE})
string(REGEX REPLACE "^.*#[ \t]*define[ \t]+WIN_SPARKLE_VERSION_MICRO[ \t]+([0-9]+).*$" "\\1" LIB_PATCH_VERSION ${VERSION_FILE})

set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
# Use the same path for shared and static library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )

set(BIN_INSTALL_DIR "bin")
set(LIB_INSTALL_DIR "lib")
set(INCLUDE_INSTALL_DIR "include")
set(CONFIG_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}")
set(CMAKE_DEBUG_POSTFIX "d")

set(EXPAT_INCLUDE_DIRS "${ROOT_DIR}/3rdparty/expat/lib")

# bundled expat
add_subdirectory(expat)

# bundled wxWidgets
add_subdirectory(wxWidgets)

# WinSparkle
set(SOURCE_DIR ${ROOT_DIR}/src)

include_directories(${ROOT_DIR}/include)
include_directories(${EXPAT_INCLUDE_DIRS})
include_directories(${wxWidgets_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(
  -DWINVER=0x0600
  -DNTDDI_VERSION=0x06000000
  -DBUILDING_WIN_SPARKLE
  -DXML_STATIC
  -DUNICODE -D_UNICODE)

set(SOURCES
  ${SOURCE_DIR}/appcast.cpp
  ${SOURCE_DIR}/appcontroller.cpp
  ${SOURCE_DIR}/dll_api.cpp
  ${SOURCE_DIR}/dllmain.cpp
  ${SOURCE_DIR}/download.cpp
  ${SOURCE_DIR}/error.cpp
  ${SOURCE_DIR}/settings.cpp
  ${SOURCE_DIR}/signatureverifier.cpp
  ${SOURCE_DIR}/threads.cpp
  ${SOURCE_DIR}/ui.cpp
  ${SOURCE_DIR}/updatechecker.cpp
  ${SOURCE_DIR}/updatedownloader.cpp)

set(PUBLIC_HEADERS
  ${ROOT_DIR}/include/winsparkle.h
  ${ROOT_DIR}/include/winsparkle-version.h)

set(RESOURCES winsparkle translations)

# resource compilation for MinGW
foreach(RESOURCE ${RESOURCES})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE}.o
                     COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE}.rc
                             -o ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE}.o)
endforeach()

add_library(${PROJECT_NAME} SHARED ${SOURCES} $<TARGET_OBJECTS:wxWidgets> $<TARGET_OBJECTS:expat>)

target_link_libraries(${PROJECT_NAME} wininet version rpcrt4 comctl32 crypt32)

set_target_properties(${PROJECT_NAME} PROPERTIES
                      VERSION ${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}
                      PUBLIC_HEADER "${PUBLIC_HEADERS}"
                      SOVERSION ${LIB_MAJOR_VERSION}
                      PREFIX "")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-export
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}" INCLUDES DESTINATION "${INCLUDE_INSTALL_DIR}")

# cmake-modules
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${PROJECT_NAME}Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
  PATH_VARS BIN_INSTALL_DIR
            LIB_INSTALL_DIR
            INCLUDE_INSTALL_DIR)

write_basic_config_version_file(WinSparkleConfigVersion.cmake VERSION ${LIB_VERSION_STRING} COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION "${CONFIG_INSTALL_DIR}")

install(EXPORT ${PROJECT_NAME}-export DESTINATION "${CONFIG_INSTALL_DIR}" FILE ${PROJECT_NAME}Targets.cmake)
