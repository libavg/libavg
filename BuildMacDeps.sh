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
    cd glib-2.10.0 
    LDFLAGS="-framework CoreFoundation $LDFLAGS" ./configure  --prefix=${AVG_PATH} --disable-shared
    make clean
    LDFLAGS="-framework CoreFoundation $LDFLAGS" make -j3
    make install
    cd ..
}

buildpango()
{
    cd pango-1.13.3
    LDFLAGS="-framework CoreFoundation $LDFLAGS" ./configure  --prefix=${AVG_PATH} --disable-shared --without-x --with-included-modules=yes
    make clean
    LDFLAGS="-framework CoreFoundation $LDFLAGS" make -j3
    make install
    cd ..
}

buildffmpeg()
{
    cd ffmpeg
#    patch -p0 < ../../libavg/macpatches/ffmpeg-svn-mactel.patch
    ./configure --prefix=${AVG_PATH} --disable-shared --disable-debug --disable-encoders
    make clean
    make -j3
    make install
    ranlib ../../lib/libavformat.a
    ranlib ../../lib/libavcodec.a
    ranlib ../../lib/libavutil.a
    cd ..    
}

if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac_avg_env.sh' before calling this script.
    exit -1 
fi

rm -rf ${AVG_PATH}/bin/
rm -rf ${AVG_PATH}/lib/
rm -rf ${AVG_PATH}/include/

mkdir ${AVG_PATH}/bin
mkdir ${AVG_PATH}/lib
mkdir ${AVG_PATH}/include

export CFLAGS=-O2
#cp macpatches/gcc-fat.sh ${AVG_PATH}/bin
#export CC="sh gcc-fat.sh"

cd ../deps

buildLib libtool-1.5.22
buildLib automake-1.9.6
buildlibjpeg
buildLib tiff-3.8.2 --disable-shared 
#buildLib zlib-1.2.3
buildlibpng
buildLib ImageMagick-6.2.8 "--without-x --without-fontconfig --without-freetype --without-perl --disable-delegate-build --without-modules"
buildLib pkg-config-0.20
buildffmpeg
buildLib SDL-1.2.11 "--disable-shared --disable-audio --disable-cdrom --disable-threads --disable-file --disable-video-x11 --without-x"
buildLib gettext-0.14.6 "--disable-shared --with-included-gettext --disable-csharp  --disable-libasprintf"
buildglib
buildLib freetype-2.1.10 --disable-shared
buildLib expat-2.0.0 --disable-shared

#patch fontconfig-2.3.1/fontconfig.pc.in ../libavg/macpatches/fontconfig.pc.in.diff
buildLib fontconfig-2.3.1 "--disable-shared --with-add-fonts=/usr/share/fonts,/Library/Fonts,/System/Library/Fonts,~/fonts"

buildpango
buildLib boost_1_33_1 --with-libraries=python 
ln -s ../include/boost-1_33_1/boost/ ../include/boost

cd ../libavg
