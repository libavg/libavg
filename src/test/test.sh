#!/bin/bash
#export XPCOM_MEM_REFCNT_LOG=refcount.log
#export XPCOM_MEM_LEAK_LOG=2
#export XPCOM_MEM_LOG_CLASSES=IAVGPlayer,avgevent,AVGImage,AVGAVGNode
#export MOZILLA_FIVE_HOME=`mozilla-config --prefix`/lib/mozilla/
export MOZILLA_FIVE_HOME=`mozilla-config --prefix`/lib
export LD_LIBRARY_PATH=$MOZILLA_FIVE_HOME:$LD_LIBRARY_PATH
export AVG_FONT_PATH=/usr/X11R6/lib/X11/fonts/truetype
cd src/test
../xpshell $PWD/test.js

