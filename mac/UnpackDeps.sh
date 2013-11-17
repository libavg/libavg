#!/bin/bash
if [[ x"${AVG_PATH}" == "x" ]]
then
    echo Please set AVG_PATH before calling this script.
    exit -1 
fi

cd $AVG_PATH/deps

for file in $(ls tarballs/*.gz) $(ls tarballs/*.bz2); do
    echo "  Unpacking $file."
    tar xf $file
done

echo "  Applying patches."
cd gettext-0.18.1.1
patch -p0 <  ../../libavg/mac/stpncpy.patch
cd ..
cd fontconfig-2.7.0
patch -R Makefile.am <../../libavg/mac/fontconfig-disablecache.patch
patch fontconfig.pc.in < ../../libavg/mac/fontconfig.pc.in.patch
cd ..
cd librsvg-2.34.0
patch Makefile.am < ../../libavg/mac/librsvg_makefile.patch
patch configure.in < ../../libavg/mac/librsvg_configure.patch
cd ..
cd glib-2.29.2/glib
patch -R gconvert.c < ../../../libavg/mac/glib.patch
cd ../..
cd freetype-2.5.0.1/
patch -p1 -R < ../../libavg/mac/freetype_linespacing.patch
cd ..

DARWINVER=`uname -r`
DARWINMAJORVER=${DARWINVER%%.*}
if [[ "${DARWINMAJORVER}" == "13" ]]
then
    cd SDL-1.2.15
    patch -p1 < ../../libavg/mac/libsdl_mavericks.patch
    cd ..
fi
cd pkg-config-0.20/glib-1.2.8/
patch -p0 -R glib.h ../../../libavg/mac/pkg-config-mavericks.patch
cd ../..
cd libdc1394-2.2.1
patch -p1 < ../../libavg/mac/dc1394_mavericks.patch
cd ..
echo "Done"
