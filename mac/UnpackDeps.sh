#!/bin/bash
pushd $AVG_PATH/deps
tar xjf ImageMagick-6.2.8-4.tar.bz2
tar xzf SDL-1.2.11.tar.gz
tar xzf automake-1.9.6.tar.gz
tar xjf boost_1_33_1.tar.bz2
tar xzf bzip2-1.0.3.tar.gz
tar xzf expat-2.0.0.tar.gz
tar xzf fontconfig-2.3.2.tar.gz
tar xjf freetype-2.1.10.tar.bz2
tar xzf gettext-0.14.6.tar.gz
tar xjf glib-2.10.0.tar.bz2
tar xzf jpegsrc.v6b.tar.gz
tar xzf libdc1394-2.0.0-rc4.tar.gz
tar xzf libiconv-1.11.tar.gz
tar xjf libpng-1.2.12.tar.bz2
tar xzf libtool-1.5.22.tar.gz
tar xzf libxml2-2.6.26.tar.gz
tar xjf pango-1.14.10.tar.bz2
tar xzf pkg-config-0.20.tar.gz
tar xzf tiff-3.8.2.tar.gz
tar xjf zlib-1.2.3.tar.bz2

cd fontconfig-2.3.2
patch fontconfig.pc.in ../../libavg/mac/fontconfig.pc.in.diff
patch -p1 <../../libavg/mac/fontconfig-2.3.2-noftinternals.patch
cd ..

cd libdc1394-2.0.0-rc4/dc1394/macosx/
patch -p0 <../../../../libavg/mac/libdc1394.patch
cd ..

popd
