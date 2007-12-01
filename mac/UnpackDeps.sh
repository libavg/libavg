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

echo "  Applying patches."
cd libdc1394-2.0.0-rc4/dc1394/macosx/
patch -p0 <../../../../libavg/mac/libdc1394.patch
cd ../../..

cd ffmpeg
patch -p0 <../../libavg/mac/ffmpeg.patch

echo "Done"
