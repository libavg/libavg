#!/bin/bash
if [[ x"${AVG_PATH}" == "x" ]]
then
    echo Please set AVG_PATH before calling this script.
    exit -1 
fi

cd $AVG_PATH/deps

echo "Unpacking libavg dependencies."
for file in $(ls tarballs/*.bz2); do
    echo "  Unpacking $file."
    tar xjf $file
done

for file in $(ls tarballs/*.gz); do
    echo "  Unpacking $file."
    tar xzf $file
done

echo "  Copying ffmpeg."
rm -rf ffmpeg
cp -pR tarballs/ffmpeg ffmpeg

echo "  Copying libdc1394."
rm -rf libdc1394
cp -pR tarballs/libdc1394 libdc1394

echo "  Applying patches."
cd fontconfig-2.5.0
patch -R Makefile.am <../../libavg/mac/fontconfig-disablecache.patch
patch fontconfig.pc.in < ../../libavg/mac/fontconfig.pc.in.patch
cd ..

cd ffmpeg
patch -p0 <../../libavg/mac/ffmpeg.patch

echo "Done"
