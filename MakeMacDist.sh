#!/bin/bash

set -e
set -x

export VERSION=0.8.0.pre1
export INSTALL_PATH_10_4="/Library/Python/2.3/site-packages/libavg"
export INSTALL_PATH_10_5="/Library/Python/2.5/site-packages/libavg"

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
    cp -Rv ../../lib/python/$PYTHON_VER/site-packages/libavg/ .
    cp ../../libavg/src/avgrc avg
    mkdir etc
    cp -R /etc/fonts etc/
    cd $LIBAVGDIR/src/test
    cp -Rv Test.py *.avg *.png *.jpg *.tif *.py *.mov *.mpg *.avi *.h264 ${AVG_PATH}/dist/libavg/avg/test
    mkdir ${AVG_PATH}/dist/libavg/avg/test/baseline
    cp baseline/* ${AVG_PATH}/dist/libavg/avg/test/baseline

    cd $LIBAVGDIR/../dist/libavg
    # Fix up library references and build one version of the package.
    distLib libMagick++.10 $INSTALL_PATH
    distLib libWand.10 $INSTALL_PATH
    fixLib libMagick++.10.dylib libWand.10 $INSTALL_PATH 
    distLib libMagick.10 $INSTALL_PATH 
    fixLib libMagick++.10.dylib libMagick.10 $INSTALL_PATH 
    fixLib libWand.10.dylib libMagick.10 $INSTALL_PATH 
}

if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac_avg_env.sh' before calling this script.
    exit -1 
fi

if [[ x$2 == x ]]
then
    echo Usage: MakeMacDist "<intel|ppc> <10.4|10.5>"
    exit 1
fi

PLATFORM=$1
LIBAVGDIR=`pwd`

if [[ $2 == 10.4 ]]
then
    makeOneDist $INSTALL_PATH_10_4 2.3
    cd $LIBAVGDIR
    /Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj mac/libavg.10.4.pmproj -v -p libavg.pkg
    hdiutil create libavg-mac-tiger-${PLATFORM}.${VERSION}.dmg -srcfolder libavg.pkg -ov 
    hdiutil internet-enable -yes libavg-mac-tiger-${PLATFORM}.${VERSION}.dmg
else
    makeOneDist $INSTALL_PATH_10_5 2.5
    cd $LIBAVGDIR
    /Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker --doc mac/libavg.10.5.pmdoc -v -o libavg.pkg
    hdiutil create libavg-mac-leopard-${PLATFORM}.${VERSION}.dmg -srcfolder libavg.pkg -ov 
    hdiutil internet-enable -yes libavg-mac-leopard-${PLATFORM}.${VERSION}.dmg
fi
