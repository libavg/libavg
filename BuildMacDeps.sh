#!/bin/bash

set -e
set -x

buildLib()
{
    LIBNAME=$1
    CONFIG_ARGS=$2

    cd ${LIBNAME}
    ./configure --prefix=${AVG_PATH} $2
    make clean
    make
    make install
    cd ..
    
}

buildlibjpeg()
{
    cd jpeg-6b
    cp /usr/share/libtool/config.sub .
    cp /usr/share/libtool/config.guess .
    ./configure --prefix=${AVG_PATH}
    make clean
    make
    make install-lib
    make install-headers
    ranlib ../../lib/libjpeg.a
    cd ..
}

if [[ x"${AVG_PATH}" == x"" ]]
then
    echo Please set AVG_PATH
    exit -1 
fi

cd ../deps

rm -rf ${AVG_PATH}/bin/*
rm -rf ${AVG_PATH}/lib/*
rm -rf ${AVG_PATH}/include/*

buildlibjpeg
buildLib tiff-3.8.2 --disable-shared 
buildLib zlib-1.2.3
buildLib libpng-1.2.12 --disable-shared 
buildLib ImageMagick-6.2.8 "--disable-shared --without-x --without-fontconfig --without-freetype --without-perl --without-xml"
buildLib ffmpeg --disable-shared
buildLib SDL-1.2.11 "--disable-shared --disable-audio --disable-cdrom --disable-threads --disable-file --disable-video-x11 --without-x"
buildiLib pkg-config-0.20
buildiLib gettext-0.14.5 --disable-shared 
buildiLib glib-2.10.0 --disable-shared

cd ../libavg
