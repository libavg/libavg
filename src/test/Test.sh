#!/bin/bash
set -x
set -e

runTest()
{
    echo
    echo "---------- OGL, BPP=$1 POW2=$2, YCbCr=$3 RGB=$4 PBO=$5 -----------"
    echo
    ./Test.py OGL $1 $2 $3 $4 $5
}

testOGL()
{
# Test real-life OpenGL configurations:
#       BPP POW2  YCbCr  RGB   PBO
# Linux NVidia GeForce 6x00 and Mac OS X GeForce FX
runTest $1  false shader false true
# Older Linux NVidia
runTest $1  false none   false true
# MESA Matrox (not sure about pow2...)
runTest $1  false mesa   false false
# Mac OS X 10.2.6, NVidia GeForce2 MX
runTest $1  false apple  false false
# Mac OS X 10.2.6, ATi Rage 128 pro 
runTest $1  true  apple  false false
# A card that doesn't support anything at all.
runTest $1  true none    false false
}

cd ${0%/*}
testOGL 24
testOGL 16
if [[ `uname` = Linux ]]
then
    ./Test.py DFB 24
#    ./Test.py DFB 16
fi
#cd -
