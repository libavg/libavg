cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
include(CheckCXXCompilerFlag)

function(set_cxx_version)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" PARENT_SCOPE)
    else()
        check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
        if(COMPILER_SUPPORTS_CXX0X)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" PARENT_SCOPE)
        else()
            message(FATAL_ERROR
                "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
        endif()
    endif()
endfunction(set_cxx_version)

project(libavg)

# libavg release version
set(AVG_VERSION_MAJOR 1)
set(AVG_VERSION_MINOR 8)
set(AVG_VERSION_MICRO 2)
set(AVG_VERSION_RELEASE "${AVG_VERSION_MAJOR}.${AVG_VERSION_MINOR}.${AVG_VERSION_MICRO}")

message(STATUS "libavg v${AVG_VERSION_RELEASE}")

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    set(PLATFORM_LINUX TRUE)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    set(PLATFORM_MACOSX TRUE)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set(PLATFORM_WINDOWS TRUE)
endif()


# Find dependencies
if(${PLATFORM_MACOSX})
    set(BOOST_ROOT "$ENV{AVG_PATH}")
endif()
set(Boost_PYTHON_VERSION "2.7")
find_package(Boost 1.46 REQUIRED system thread python)
find_package(PkgConfig REQUIRED)
if(${PLATFORM_MACOSX})
    set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:$ENV{AVG_PATH}/libavg/pkg-config/mac")
endif()
pkg_search_module(LIBXML2 REQUIRED libxml-2.0 libxml2)
pkg_search_module(GDK_PIXBUF REQUIRED gdk-pixbuf-2.0)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(DC1394_2 libdc1394-2)
pkg_check_modules(FFMPEG libswscale libavformat libavcodec libavutil libswresample)
set(Python_ADDITIONAL_VERSIONS 2.7)
find_package(PythonLibs REQUIRED)
pkg_search_module(PANGOFT2 REQUIRED pangoft2)
pkg_search_module(RSVG REQUIRED librsvg-2.0)
pkg_search_module(FONTCONFIG REQUIRED fontconfig)
find_package(RT)

find_path(VIDEODEV2_FOUND
    NAMES videodev2.h
    PATH_SUFFIXES linux
    DOC "Video4Linux2 header file include directory")


# Options
set(AVG_ENABLE_EGL FALSE CACHE BOOL "include EGL support")
set(AVG_ENABLE_RPI FALSE CACHE BOOL "configure for RaspberryPi")

if(${DC1394_2_FOUND})
    set(AVG_ENABLE_1394_2 TRUE CACHE BOOL "compile support for firewire cameras")
endif()
if(VIDEODEV2_FOUND)
    set(AVG_ENABLE_V4L2 TRUE CACHE BOOL "Compile support for video4linux v2 video devices")
    find_package(JPEG REQUIRED)
endif()

# Python virtualenv handling: If a virtualenv is active, install everything there.
set(virtual_env $ENV{VIRTUAL_ENV})
if (virtual_env)
    message(STATUS "Python virtualenv in use, setting install prefix to ${virtual_env}.")
    set(CMAKE_INSTALL_PREFIX ${virtual_env})
endif()

# Platform-specific configuration
if(${PLATFORM_LINUX})
    if(NOT ${AVG_ENABLE_EGL})
        set(AVG_ENABLE_GLX_X11 TRUE)
    endif()
endif()

if(${PLATFORM_MACOSX})
    set(CMAKE_MACOSX_RPATH 0)
    set(AVG_ENABLE_CGL_QUARTZ TRUE)
    find_library(COCOA Cocoa)
    find_library(COREAUDIO CoreAudio)
    find_library(AUDIOUNIT AudioUnit)
    find_library(FORCEFEEDBACK ForceFeedback)
    find_library(COREVIDEO CoreVideo)
    find_library(IOKIT IOKit)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(OPENGL OpenGL)
    find_library(CARBON Carbon)
    set(PLATFORM_MACOSX_LIBS
            ${COCOA} ${COREAUDIO} ${AUDIOUNIT} ${FORCEFEEDBACK} ${COREVIDEO}
            ${IOKIT} ${COREFOUNDATION} ${OPENGL} ${CARBON})
    # The following lines work around broken pkg-config scripts on mac.
    string (REPLACE ";" " " FFMPEG_LDFLAGS "${FFMPEG_LDFLAGS}")
    set(RSVG_LDFLAGS "${RSVG_LDFLAGS} -lpixman-1 -lpangocairo-1.0")
    set(GDK_PIXBUF_LDFLAGS "${GDK_PIXBUF_LDFLAGS} -lgmodule-2.0 -lgio-2.0 -lresolv")
    link_libraries(${PLATFORM_MACOSX_LIBS})
endif()

if(${PLATFORM_WINDOWS})
    message(FATAL_ERROR
        "Windows is not supported by the CMake build setup right now. "
        "Use the prebuilt Visual Studio file './win/libavg.sln'. ")
endif()

if(${AVG_ENABLE_RPI})
    set(ENV{PKG_CONFIG_PATH} ./pkg-config/rpi)
endif()

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING
       "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
       FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU")
    set(warnings_cxx "-Wall -Wno-invalid-offsetof -Woverloaded-virtual -Wswitch -Wuninitialized -Wempty-body -Wno-unused-local-typedefs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -fPIC -ffast-math ${warnings_cxx}")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pipe -fPIC -ffast-math -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_C_FLAGS_RELEASE   "-O3")
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64|amd64|i386|i686")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
    endif()
    set_cxx_version()
endif()

# Feature configuration
if(${AVG_ENABLE_GLX_X11})
    pkg_check_modules(GLX_X11 REQUIRED x11 gl)
endif()
if(${AVG_ENABLE_EGL})
    pkg_check_modules(EGL REQUIRED egl glesv2)
endif()
if(${AVG_ENABLE_RPI})
    pkg_check_module(BCM REQUIRED bcm)
endif()

enable_testing()


# Enable make check that dumps test output on failure
if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure 
        --build-config "$<CONFIGURATION>")
else()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure)
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/src/avgconfig.h.cmake.in"
    "${PROJECT_BINARY_DIR}/src/avgconfig.h")
include_directories("${PROJECT_BINARY_DIR}/src")
add_subdirectory(src)
add_subdirectory(python)

