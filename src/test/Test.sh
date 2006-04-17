#!/bin/bash

testOGL()
{
# Test real-life OpenGL configurations:
#             BPP POW2  YCbCr  RGB   PBO
# Linux NVidia GeForce 6x00 and Mac OS X GeForce FX
./Test.py OGL $1  false shader false true
# Older Linux NVidia
./Test.py OGL $1  false none   false true
# MESA Matrox (not sure about pow2...)
./Test.py OGL $1  false mesa   false false
# Mac OS X 10.2.6, NVidia GeForce2 MX
./Test.py OGL $1  false apple  false false
# Mac OS X 10.2.6, ATi Rage 128 pro 
./Test.py OGL $1  true  apple  false false
# A card that doesn't support anything at all.
./Test.py OGL $1  true none    false false
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
