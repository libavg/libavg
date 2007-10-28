#!/bin/bash

set -e
set -x

clean()
{
    rm -rf ${AVG_PATH}/bin/
    rm -rf ${AVG_PATH}/lib/
    rm -rf ${AVG_PATH}/include/

    mkdir ${AVG_PATH}/bin
    mkdir ${AVG_PATH}/lib
    mkdir ${AVG_PATH}/include
}

buildLib()
{
    LIBNAME=$1
    CONFIG_ARGS=$2

    cd ${LIBNAME}
    ./configure --prefix=${AVG_PATH} ${CONFIG_ARGS}
    make clean
    make -j3
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
    make -j3
    make install-lib
    make install-headers
    ranlib ../../lib/libjpeg.a
    cd ..
}

buildlibpng()
{
    cd libpng-1.2.12
    CFLAGS="-fno-common" ./configure --prefix=${AVG_PATH} --disable-shared
    make clean
    make -j3
    make install
    cd ..
}

buildglib()
{
    cd glib-2.14.1 
    LDFLAGS="-framework CoreFoundation $LDFLAGS" ./configure  --prefix=${AVG_PATH} --disable-shared
    make clean
    LDFLAGS="-framework CoreFoundation $LDFLAGS" make -j3
    make install
    cd ..
}

buildpango()
{
    cd pango-1.18.2
    LDFLAGS="-framework CoreFoundation -framework ApplicationServices $LDFLAGS" ./configure  --prefix=${AVG_PATH} --disable-shared --without-x --with-included-modules=yes
    make clean
    LDFLAGS="-framework CoreFoundation $LDFLAGS" make -j3
    make install
    cd ..
}

buildfontconfig()
{
    cd fontconfig-2.3.2
    LDFLAGS="-framework ApplicationServices ${LDFLAGS}" ./configure --prefix=${AVG_PATH} --disable-shared --with-add-fonts=/Library/Fonts,/System/Library/Fonts,~/fonts --with-confdir=/etc/fonts
    make clean
    make -j3
    sudo make install
    cd ..    
}

if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac_avg_env.sh' before calling this script.
    exit -1 
fi

clean

export CFLAGS=-O2

cd ../deps

buildLib libtool-1.5.22
buildLib automake-1.9.6
buildlibjpeg
buildLib tiff-3.8.2 --disable-shared 
buildLib zlib-1.2.3
buildlibpng
buildLib ImageMagick-6.2.8 "--without-x --without-fontconfig --without-freetype --without-perl --disable-delegate-build --without-modules --without-bzlib"
buildLib pkg-config-0.20
buildLib ffmpeg "--disable-shared --disable-debug"
buildLib SDL-1.2.11 "--disable-shared --disable-audio --disable-cdrom --disable-threads --disable-file --disable-video-x11 --without-x"
buildLib gettext-0.14.6 "--disable-shared --with-included-gettext --disable-csharp  --disable-libasprintf"
buildglib

buildLib freetype-2.3.5 "--disable-shared --with-old-mac-fonts"
buildLib expat-2.0.0 --disable-shared

buildfontconfig

buildpango
buildLib boost_1_33_1 --with-libraries=python,thread 
rm -f ../include/boost
ln -fs ../include/boost-1_33_1/boost/ ../include/boost

buildLib libdc1394-2.0.0-rc4 --disable-shared


cd ../libavg
