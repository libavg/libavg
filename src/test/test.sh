#!/bin/bash
#export XPCOM_MEM_REFCNT_LOG=refcount.log
#export XPCOM_MEM_LEAK_LOG=2
#export XPCOM_MEM_LOG_CLASSES=IAVGPlayer,avgevent,AVGImage,AVGAVGNode
export TESTDIR=$PWD
export MOZILLA_FIVE_HOME=/usr/local/lib/mozilla-1.1
export LD_LIBRARY_PATH=$MOZILLA_FIVE_HOME:$LD_LIBRARY_PATH
$MOZILLA_FIVE_HOME/xpcshell -w -s $TESTDIR/test/test.js

