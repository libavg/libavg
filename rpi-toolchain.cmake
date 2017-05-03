# This file is supposed to be used for cross compiling libavg for Raspberry Pi following
# this guide: https://www.libavg.de/site/projects/libavg/wiki/RaspberryPISourceInstall

if(NOT DEFINED ENV{AVG_RPI_PATH})
    message(FATAL_ERROR "AVG_RPI_PATH env var not set!")
endif()

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# cross compiler for 64 bit host system
set(CMAKE_C_COMPILER $ENV{AVG_RPI_PATH}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER $ENV{AVG_RPI_PATH}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-g++)
# cross compiler for 32 bit host system
#set(CMAKE_C_COMPILER $ENV{AVG_RPI_PATH}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc)
#set(CMAKE_CXX_COMPILER $ENV{AVG_RPI_PATH}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-g++)

set(CMAKE_SYSROOT $ENV{AVG_RPI_PATH}/rootfs)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig:${CMAKE_SYSROOT}/usr/local/lib/pkgconfig)
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})

include_directories(${CMAKE_SYSROOT}/usr/local/include)

set(CMAKE_INSTALL_PREFIX $ENV{AVG_RPI_PATH}/staging/usr/local CACHE INTERNAL "")
set(PYTHON_CUSTOM_INSTALL_OPTIONS --prefix=${CMAKE_INSTALL_PREFIX})
