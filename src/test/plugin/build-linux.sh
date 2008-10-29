#!/bin/sh
g++ -fPIC -c -fno-common  -I. -I../../../src -I.. \
-I/usr/include/libxml2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
-I/usr/include/python2.5 -pthread \
ColorNode.cpp -o ColorNode.o
g++ -fPIC -shared -shared-libgcc -o ColorNode.so ColorNode.o  ../../wrapper/.libs/avg.so


