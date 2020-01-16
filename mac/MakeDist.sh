#!/bin/bash

set -e
set -x

cd ..

VERSION_MAJOR=$(grep VERSION_MAJOR m4/avg_version.m4 |
        sed 's/^.*\[ *\([A-Za-z0-9\.]*\) *\].*$/\1/')
VERSION_MINOR=$(grep VERSION_MINOR m4/avg_version.m4 |
        sed 's/^.*\[ *\([A-Za-z0-9\.]*\) *\].*$/\1/')
VERSION_MICRO=$(grep VERSION_MICRO m4/avg_version.m4 |
        sed 's/^.*\[ *\([A-Za-z0-9\.]*\) *\].*$/\1/')

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
    cp -Rv ${BUILDDIR}/site-packages/libavg/ .
    strip -S avg.0.so
    cp ../../libavg/src/avgrc avg
    mkdir etc
    cp -R /etc/fonts etc/

    cd $LIBAVGDIR/../bindist
    rm -rf *
    cp /usr/local/bin/avg_* .

    cd $LIBAVGDIR/../sharedist
    rm -rf *
    cp -Rv /usr/local/share/python-libavg/* .

}

if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac/avg_env.sh' before calling this script.
    exit -1 
fi

LIBAVGDIR=`pwd`
DARWINVER=`uname -r`
DARWINMAJORVER=${DARWINVER%%.*}

PYTHONVERSION=2.7
OSXVERSION=10.6
BUILDDIR=/usr/local/lib/python2.7/

makeOneDist /Library/Python/${PYTHONVERSION}/site-packages/libavg ${PYTHONVERSION} 
cd $LIBAVGDIR
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker --doc mac/libavg.10.6.pmdoc -v -o libavg.pkg
hdiutil create libavg-mac-${VERSION}.dmg -srcfolder libavg.pkg -ov 
hdiutil internet-enable -yes libavg-mac-${VERSION}.dmg
