#!/bin/bash
export TESTDIR=$PWD
export MOZILLA_FIVE_HOME=/usr/local/lib/mozilla-1.1
export LD_LIBRARY_PATH=$MOZILLA_FIVE_HOME:$LD_LIBRARY_PATH
$MOZILLA_FIVE_HOME/xpcshell -w -s $TESTDIR/test/test.js

