#!/bin/bash

set -e

export VERSION=0.5.5
export AVG_PATH="/Users/uzadow/libavg/"
export INSTALL_PATH="/Library/Python/2.3/site-packages"

fixLib()
{
    install_name_tool -change ${AVG_PATH}lib/$2.dylib $INSTALL_PATH/avg_libs/$2.dylib avg_libs/$1 
    install_name_tool -id $INSTALL_PATH/avg_libs/$2.dylib avg_libs/$2.dylib
}

distLib()
{
    cp -v ../lib/$1.dylib ./avg_libs
    install_name_tool -change $AVG_PATH/lib/$1.dylib $INSTALL_PATH/avg_libs/$1.dylib avg.0.so
    fixLib ../avg.0.so $1
}

cd ../dist

rm -rf *
mkdir avg_libs

cp -RP ../lib/python2.3/site-packages/libavg/ .

distLib libSDL-1.2.0
distLib libMagick++.10
distLib libWand.10
fixLib libMagick++.10.dylib libWand.10
distLib libMagick.10
fixLib libMagick++.10.dylib libMagick.10
fixLib libWand.10.dylib libMagick.10
distLib libtiff.3
distLib libpng.3
distLib libavformat
distLib libavcodec
fixLib libavformat.dylib libavcodec
distLib libavutil
fixLib libavformat.dylib libavutil
fixLib libavcodec.dylib libavutil
distLib libpangoft2-1.0.0
distLib libpango-1.0.0
fixLib libpangoft2-1.0.0.dylib libpango-1.0.0
distLib libxml2.2
fixLib libMagick++.10.dylib libxml2.2
fixLib libMagick.10.dylib libxml2.2
fixLib libWand.10.dylib libxml2.2
distLib libtiff.3
fixLib libMagick++.10.dylib libtiff.3
fixLib libMagick.10.dylib libtiff.3
fixLib libWand.10.dylib libtiff.3
distLib libpng.3
fixLib libMagick++.10.dylib libpng.3
fixLib libMagick.10.dylib libpng.3
fixLib libWand.10.dylib libpng.3
distLib libfontconfig.1
fixLib libpangoft2-1.0.0.dylib libfontconfig.1
distLib libexpat.1
fixLib libpangoft2-1.0.0.dylib libexpat.1
fixLib libfontconfig.1.dylib libexpat.1
distLib libfreetype.6
fixLib libpangoft2-1.0.0.dylib libfreetype.6
fixLib libfontconfig.1.dylib libfreetype.6
distLib libgobject-2.0.0
fixLib libpangoft2-1.0.0.dylib libgobject-2.0.0
fixLib libpango-1.0.0.dylib libgobject-2.0.0
distLib libgmodule-2.0.0
fixLib libpango-1.0.0.dylib libgmodule-2.0.0
fixLib libpangoft2-1.0.0.dylib libgmodule-2.0.0
distLib libglib-2.0.0
fixLib libpangoft2-1.0.0.dylib libglib-2.0.0
fixLib libpango-1.0.0.dylib libglib-2.0.0
fixLib libgobject-2.0.0.dylib libglib-2.0.0
fixLib libgmodule-2.0.0.dylib libglib-2.0.0
distLib libintl.3
fixLib libpangoft2-1.0.0.dylib libintl.3
fixLib libpango-1.0.0.dylib libintl.3
fixLib libgobject-2.0.0.dylib libintl.3
fixLib libglib-2.0.0.dylib libintl.3
fixLib libgmodule-2.0.0.dylib libintl.3

cd ../libavg

/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj libavg.pmproj -v -p libavg.pkg
zip -ry libavg-mac.${VERSION}.zip libavg.pkg
