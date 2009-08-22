#!/bin/bash

set -e
set -x

export VERSION=0.9.0.pre3
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
    mkdir avg/video
    mkdir avg/video/testfiles
    cp -Rv /Library/Python/$PYTHON_VER/site-packages/libavg/ .
    strip -S avg.0.so
    cp ../../libavg/src/avgrc avg
    mkdir etc
    cp -R /etc/fonts etc/
    cd $LIBAVGDIR/src/test
    cp -Rv Test.sh *.py *.avg *.png *.jpg *.tif ${AVG_PATH}/dist/libavg/avg/test
    mkdir ${AVG_PATH}/dist/libavg/avg/test/baseline
    cp baseline/* ${AVG_PATH}/dist/libavg/avg/test/baseline
    mkdir ${AVG_PATH}/dist/libavg/avg/test/testmediadir
    cp testmediadir/* ${AVG_PATH}/dist/libavg/avg/test/testmediadir
    mkdir ${AVG_PATH}/dist/libavg/avg/test/fonts
    cp fonts/* ${AVG_PATH}/dist/libavg/avg/test/fonts
    mkdir ${AVG_PATH}/dist/libavg/avg/test/plugin
    cp plugin/.libs/libColorNode.so ${AVG_PATH}/dist/libavg/avg/test/plugin
    cp plugin/.libs/libColorNode.0.so ${AVG_PATH}/dist/libavg/avg/test/plugin
    mkdir ${AVG_PATH}/dist/libavg/avg/test/extrafonts
    cp extrafonts/testaddfontdir.ttf ${AVG_PATH}/dist/libavg/avg/test/extrafonts

    
    cd $LIBAVGDIR/src/video/testfiles/
    cp -Rv *.mov *.mpg *.avi *.h264 *.wav *.aif *.ogg *.mp3 ${AVG_PATH}/dist/libavg/avg/video/testfiles

#    cd $LIBAVGDIR/../dist/libavg
    # Fix up library references and build one version of the package.
#    distLib libMagick++.10 $INSTALL_PATH
#    distLib libWand.10 $INSTALL_PATH
#    fixLib libMagick++.10.dylib libWand.10 $INSTALL_PATH 
#    distLib libMagick.10 $INSTALL_PATH 
#    fixLib libMagick++.10.dylib libMagick.10 $INSTALL_PATH 
#    fixLib libWand.10.dylib libMagick.10 $INSTALL_PATH 

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

makeOneDist $INSTALL_PATH_10_5 2.5
cd $LIBAVGDIR
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker --doc mac/libavg.10.5.pmdoc -v -o libavg.mpkg
hdiutil create libavg-mac-${VERSION}.dmg -srcfolder libavg.mpkg -ov 
hdiutil internet-enable -yes libavg-mac-${VERSION}.dmg
