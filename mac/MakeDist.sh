#!/bin/bash

set -e
set -x

cd ..

VERSION_MAJOR=$(grep VERSION_MAJOR m4/avg_version.m4 |
        sed 's/^.*\[ *\([0-9]*\) *\].*$/\1/')
VERSION_MINOR=$(grep VERSION_MINOR m4/avg_version.m4 |
        sed 's/^.*\[ *\([0-9]*\) *\].*$/\1/')
VERSION_MICRO=$(grep VERSION_MICRO m4/avg_version.m4 |
        sed 's/^.*\[ *\([0-9]*\) *\].*$/\1/')

export VERSION=$VERSION_MAJOR.$VERSION_MINOR.$VERSION_MICRO

fixLib()
{
    INSTALL_PATH=$3
    install_name_tool -change ${AVG_PATH}/lib/$2.dylib $INSTALL_PATH/avg/$2.dylib avg/$1 
    install_name_tool -id $INSTALL_PATH/avg/$2.dylib avg/$2.dylib
}

distLib()
{
    INSTALL_PATH=$2
    cp -v ../../lib/$1.dylib ./avg
    install_name_tool -change $AVG_PATH//lib/$1.dylib $INSTALL_PATH/avg/$1.dylib avg.0.so
    fixLib ../avg.0.so $1 $INSTALL_PATH
}

makeOneDist()
{
    INSTALL_PATH=$1
    PYTHON_VER=$2

    # Copy distribution files into staging area.
    cd $LIBAVGDIR/../dist
    rm -rf *
    mkdir libavg
    cd libavg
    mkdir avg
    mkdir avg/test
    mkdir avg/video
    mkdir avg/video/testfiles
    mkdir avg/graphics
    mkdir avg/graphics/testfiles
    cp -Rv ${BUILDDIR}/site-packages/libavg/ .
    strip -S avg.0.so
    cp ../../libavg/src/avgrc avg
    mkdir etc
    cp -R /etc/fonts etc/
    cd $LIBAVGDIR/src/test
    cp -Rv *.py *.avg *.png *.jpg *.tif *.svg ${AVG_PATH}/dist/libavg/avg/test
    mkdir ${AVG_PATH}/dist/libavg/avg/test/baseline
    cp baseline/* ${AVG_PATH}/dist/libavg/avg/test/baseline
    mkdir ${AVG_PATH}/dist/libavg/avg/test/testmediadir
    cp testmediadir/* ${AVG_PATH}/dist/libavg/avg/test/testmediadir
    mkdir ${AVG_PATH}/dist/libavg/avg/test/fonts
    cp fonts/* ${AVG_PATH}/dist/libavg/avg/test/fonts
    mkdir ${AVG_PATH}/dist/libavg/avg/test/plugin
    cp plugin/.libs/colorplugin.so ${AVG_PATH}/dist/libavg/avg/test/plugin
    cp plugin/.libs/colorplugin.0.so ${AVG_PATH}/dist/libavg/avg/test/plugin
    mkdir ${AVG_PATH}/dist/libavg/avg/test/extrafonts
    cp extrafonts/testaddfontdir.ttf ${AVG_PATH}/dist/libavg/avg/test/extrafonts

    
    cd $LIBAVGDIR/src/video/testfiles/
    cp -Rv *.mov *.mpg *.avi *.h264 *.wav *.aif *.ogg *.mp3 *.flv ${AVG_PATH}/dist/libavg/avg/video/testfiles
    cd $LIBAVGDIR/src/graphics/testfiles/
    cp -Rv *.png ${AVG_PATH}/dist/libavg/avg/graphics/testfiles

    cd $LIBAVGDIR/../bindist
    rm -rf *
    cp /usr/local/bin/avg_* .
}

if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac/avg_env.sh' before calling this script.
    exit -1 
fi

LIBAVGDIR=`pwd`
DARWINVER=`uname -r`
DARWINMAJORVER=${DARWINVER%%.*}

if [[ "${DARWINMAJORVER}" == "10" ]]
then
    PYTHONVERSION=2.6
    OSXVERSION=10.6
    BUILDDIR=/usr/local/lib/python2.6/
else
    PYTHONVERSION=2.5
    OSXVERSION=10.5
    BUILDDIR=/Library/Python/2.5
    sudo rm -rf ${BUILDDIR}/site-packages/libavg
    sudo make install
fi
makeOneDist /Library/Python/${PYTHONVERSION}/site-packages/libavg ${PYTHONVERSION} 
cd $LIBAVGDIR
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker --doc mac/libavg.${OSXVERSION}.pmdoc -v -o libavg.pkg
hdiutil create libavg-mac-${OSXVERSION}-${VERSION}.dmg -srcfolder libavg.pkg -ov 
hdiutil internet-enable -yes libavg-mac-${OSXVERSION}-${VERSION}.dmg
