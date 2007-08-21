#!/bin/bash
set -e

runTest()
{
    echo
    echo "---------- BPP=$1 POW2=$2, YCbCr=$3 PBO=$4 -----------"
    echo
    ./Test.py $1 $2 $3 $4
}

testOGL()
{
# Test real-life OpenGL configurations:
#       BPP POW2  YCbCr  PBO
# Linux NVidia GeForce 6x00 and Mac OS X GeForce FX
runTest $1  false shader true
if [[ `uname` == Linux ]]
then
    # Older Linux NVidia
    runTest $1  false none   true
    # MESA Matrox (not sure about pow2...)
    runTest $1  false mesa   false
else
    # Mac OS X 10.2.6, NVidia GeForce2 MX
    runTest $1  false apple  false
    # Mac OS X 10.2.6, ATi Rage 128 pro 
    runTest $1  true  apple  false
fi
# A card that doesn't support anything at all.
runTest $1  true none    false
}

cd ${0%/*}
testOGL 24
testOGL 16
cd -
