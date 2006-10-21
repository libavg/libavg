#!/bin/bash

set -e
set -x

export VERSION=0.6.0
export INSTALL_PATH="/Library/Python/2.3/site-packages"

fixLib()
{
    install_name_tool -change ${AVG_PATH}/lib/$2.dylib $INSTALL_PATH/avg/$2.dylib avg/$1 
    install_name_tool -id $INSTALL_PATH/avg/$2.dylib avg/$2.dylib
}

distLib()
{
    cp -v ../lib/$1.dylib ./avg
    install_name_tool -change $AVG_PATH//lib/$1.dylib $INSTALL_PATH/avg/$1.dylib avg.0.so
    fixLib ../avg.0.so $1
}
 
if [[ x"${PKG_CONFIG_PATH}" == "x" ]]
then
    echo Please call 'source mac_avg_env.sh' before calling this script.
    exit -1 
fi

if [[ x$1 == x ]]
then
    echo Usage: MakeMacDist "<intel|ppc>"
    exit 1
fi

PLATFORM=$1

cd ../dist

rm -rf *
mkdir avg
mkdir avg/test

cp -Rv ../lib/python2.3/site-packages/libavg/ .

distLib libMagick++.10
distLib libWand.10
fixLib libMagick++.10.dylib libWand.10
distLib libMagick.10
fixLib libMagick++.10.dylib libMagick.10
fixLib libWand.10.dylib libMagick.10

cd ../libavg

cp src/avgrc ${AVG_PATH}/dist/avg/
cp ../bin/fc-cache ${AVG_PATH}/dist/avg/

cd src/test
cp -Rv Test.py *.avg *.png *.jpg *.tif *.py *.mov *.mpg *.avi *.h264 ${AVG_PATH}/dist/avg/test
mkdir ${AVG_PATH}/dist/avg/test/baseline
cp baseline/* ${AVG_PATH}/dist/avg/test/baseline
cd ../..

/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj mac/libavg.pmproj -v -p libavg.pkg
zip -ry libavg-mac-${PLATFORM}.${VERSION}.zip libavg.pkg
