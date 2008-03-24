#!/bin/bash
if [[ x"${AVG_PATH}" == "x" ]]
then
    echo Please set AVG_PATH before calling this script.
    exit -1 
fi

DEST_PATH=`pwd`
cd $AVG_PATH/deps
tar cjf $DEST_PATH/macdependencies.tar.bz2 tarballs

echo "Done"
