#!/bin/bash
#export XPCOM_MEM_REFCNT_LOG=refcount.log
#export XPCOM_MEM_LEAK_LOG=2
#export XPCOM_MEM_LOG_CLASSES=IAVGPlayer,avgevent,AVGImage,AVGAVGNode
export TESTDIR=$PWD
export MOZILLA_FIVE_HOME=$PRO/extern/mozilla/bin
#export MOZILLA_FIVE_HOME=$PRO/extern/mozilla/bin
export LD_LIBRARY_PATH=$PRO/extern/mozilla/lib/:$LD_LIBRARY_PATH
$MOZILLA_FIVE_HOME/xpcshell -w -s $TESTDIR/test/test.js

