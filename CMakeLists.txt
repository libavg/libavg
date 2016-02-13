project(libavg)
cmake_minimum_required(VERSION 2.8)

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

find_package(Boost 1.46 REQUIRED system thread python)
find_package(PkgConfig REQUIRED)
pkg_search_module(LIBXML2 REQUIRED libxml-2.0 libxml2)
pkg_search_module(GDK_PIXBUF REQUIRED gdk-pixbuf-2.0)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(DC1394_2 libdc1394-2)
pkg_check_modules(FFMPEG libswscale libavformat libavcodec libavutil)
pkg_search_module(PYTHON REQUIRED python2 python-2.7 python)
pkg_search_module(PANGOFT2 REQUIRED pangoft2)
pkg_search_module(RSVG REQUIRED librsvg-2.0)
pkg_search_module(FONTCONFIG REQUIRED fontconfig)

# V4L2: this is the only dependency
find_path(VIDEODEV2_FOUND
    NAMES videodev2.h
    PATH_SUFFIXES linux
    DOC "Video for Linux Two header file include directory")



# options

set(AVG_ENABLE_EGL FALSE CACHE BOOL "include EGL support")
set(AVG_ENABLE_RPI FALSE CACHE BOOL "configure for RaspberryPi")

if(${DC1394_2_FOUND})
    set(AVG_ENABLE_1394_2 TRUE CACHE BOOL "compile support for firewire cameras")
endif()
if(${VIDEODEV2_FOUND})
    set(AVG_ENABLE_V4L2 TRUE CACHE BOOL "compile support for video4linux v2 video devices")
endif()



# platform choices

if(${PLATFORM_LINUX})
    if(NOT ${AVG_ENABLE_EGL})
        set(AVG_ENABLE_GLX_X11 TRUE)
    endif()
endif()

if(${PLATFORM_MACOSX})
    # TODO In the original automake setup there was no further distinction
    #  besides the frameworks and options listed here.
    # I (payload) decided to postpone the refactoring into features
    #  until later™.
    # PLATFORM_MACOSX_* is currently only added in target graphics.
    # I think this propagates to all other targets,
    #  but isn't the proper way.
    set(AVG_ENABLE_CGL_QUARTZ TRUE)
    pkg_check_module(PLATFORM_MACOSX REQUIRED
        MultitouchSupport Cocoa CoreAudio AudioUnit
        ForceFeedback CoreVideo IOKit)
    list(APPEND PLATFORM_MACOSX_CFLAGS
        "-F/System/Library/PrivateFrameworks ")
    list(APPEND PLATFORM_MACOSX_INCLUDE_DIRS
        "/System/Library/PrivateFrameworks")
    # TODO payload: @uzadow, this is a flag in src/wrapper/Makefile.am. Needed?
    #   -read_only_relocs suppress
    # add it with   target_link_libraries( ... )
endif()

if(${PLATFORM_WINDOWS})
    message(FATAL_ERROR
        "Windows is not supported by the CMake build setup right now. "
        "Use the prebuilt Vistual Studio file './win/libavg.sln'. ")
endif()

if(${AVG_ENABLE_RPI})
    set(ENV{PKG_CONFIG_PATH} ./pkg-config/rpi)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "-O3 -ffast-math -Wall -pipe -Wno-invalid-offsetof -Woverloaded-virtual -Wswitch -Wuninitialized -Wempty-body ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CFLAGS "-O3 -ffast-math -Wall -pipe ${CMAKE_C_FLAGS}")
    if ((${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64") OR
            (${CMAKE_SYSTEM_PROCESSOR} MATCHES "amd64") OR
            (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i386") OR
            (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686"))
        set(CMAKE_CXX_FLAGS "-msse2 ${CMAKE_CXX_FLAGS}")
        set(CMAKE_C_FLAGS "-msse2 ${CMAKE_C_FLAGS}")
    endif()
endif()
# TODO: Missing linker flag -rdynamic from autoconf

# feature configuration

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
configure_file(src/avgconfig.h.cmake.in src/avgconfig.h)
add_subdirectory(src)
