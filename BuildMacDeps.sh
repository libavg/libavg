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

buildglib()
{
    cd glib-2.10.0 
    LDFLAGS="-framework CoreFoundation $LDFLAGS" ./configure  --prefix=${AVG_PATH} --disable-shared
    make clean
    LDFLAGS="-framework CoreFoundation $LDFLAGS" make
    make install
    cd ..
}

buildpango()
{
    cd pango-1.13.3
    LDFLAGS="-framework CoreFoundation $LDFLAGS" ./configure  --prefix=${AVG_PATH} --disable-shared
    make clean
    LDFLAGS="-framework CoreFoundation $LDFLAGS" make
    make install
    cd ..
}

fixpkgconfig()
{
  Filename=$1
  sed 's/PREFIX/prefix/' ${Filename} > ${Filename}.tmp
  mv ${Filename}.tmp ${Filename}
}

buildffmpeg()
{
    cd ffmpeg
    ./configure --prefix=${AVG_PATH} --disable-shared
    make clean
    make
    fixpkgconfig libavcodec.pc 
    fixpkgconfig libavformat.pc 
    fixpkgconfig libavutil.pc 
    make install
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
buildLib pkg-config-0.20
buildffmpeg
buildLib SDL-1.2.11 "--disable-shared --disable-audio --disable-cdrom --disable-threads --disable-file --disable-video-x11 --without-x"
buildLib gettext-0.14.6 "--disable-shared --with-included-gettext"
buildglib
buildLib freetype-2.1.10 --disable-shared
buildLib expat-2.0.0 --disable-shared 
buildLib fontconfig-2.3.1 "--disable-shared --with-add-fonts=/Library/Fonts,/Network/Library/Fonts,/System/Library/Fonts"
buildpango
buildLib boost_1_33_1 --with-libraries=python 

cd ../libavg
