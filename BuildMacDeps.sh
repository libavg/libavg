#!/bin/bash

set -e
set -x

clean()
{
    rm -rf ${AVG_PATH}/bin/
    rm -rf ${AVG_PATH}/lib/
    sudo rm -rf ${AVG_PATH}/include/

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
    cp ${AVG_PATH}/share/libtool/config.sub .
    cp ${AVG_PATH}/share/libtool/config.guess .
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
    cd fontconfig-2.5.0
    automake
    LDFLAGS="-framework ApplicationServices ${LDFLAGS}" ./configure --prefix=${AVG_PATH} --disable-shared --with-add-fonts=/Library/Fonts,/System/Library/Fonts,~/Library/Fonts --with-confdir=/etc/fonts --with-cache-dir=~/.fontconfig --with-cachedir=~/.fontconfig/
    make clean
    make -j3
    sudo make install
    cd ..    
}

buildlibdc1394()
{
    cd libdc1394
    autoreconf -i -s
    ./configure --prefix=${AVG_PATH} --disable-shared --disable-doxygen-doc --without-x
    make clean
    make -j3
    make install
    cd ..

}

if [[ x"${AVG_PATH}" == "x" ]]
then
    echo Please set AVG_PATH and call 'source mac/avg_env.sh' before calling this script.
    exit -1 
fi

if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac/avg_env.sh' before calling this script.
    exit -1 
fi

clean

cd ../deps

buildLib libtool-1.5.22
buildLib automake-1.9.6
buildlibjpeg
buildLib tiff-3.8.2 --disable-shared 
buildLib zlib-1.2.3
buildlibpng
buildLib GraphicsMagick-1.1.10 "--without-x --without-perl --disable-shared --disable-delegate-build --without-modules --without-bzlib --without-dps --without-gslib --without-wmf --without-xml --without-ttf --with-quantum-depth=8
buildLib pkg-config-0.20
buildLib ffmpeg "--disable-shared --disable-debug --disable-vhook"
buildLib SDL-1.2.11 "--disable-shared --disable-audio --disable-cdrom --disable-threads --disable-file --disable-video-x11 --without-x"
buildLib gettext-0.14.6 "--disable-shared --with-included-gettext --disable-csharp  --disable-libasprintf"
buildglib

buildLib freetype-2.3.5 "--disable-shared --with-old-mac-fonts"
buildLib expat-2.0.0 --disable-shared

buildfontconfig

buildpango
buildLib boost_1_34_1 "--with-libraries=python,thread"
rm -f ../include/boost
ln -fs ../include/boost-1_34_1/boost/ ../include/boost
ln -fs ../lib/libboost_thread-mt.a ../lib/libboost_thread.a

buildlibdc1394


cd ../libavg
