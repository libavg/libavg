#!/bin/bash

cd ${0%/*}
./Test.py OGL 24
./Test.py OGL 16
./Test.py DFB 24
./Test.py DFB 16
#cd -
