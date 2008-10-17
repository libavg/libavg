#!/bin/sh
#g++ -shared-libgcc -fPIC -export-dynamic -ldl loader.cpp Singleton.cpp -o loader

g++ -fPIC -c -fno-common  -I. -I../../../src -I.. \
-I/usr/include/libxml2 -I/usr/include/freetype2 \
-I/usr/include/pango-1.0 -I/usr/include/freetype2 \
-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
-I/usr/include/python2.5 -pthread -I/usr/include/ffmpeg \
-I/usr/include/GraphicsMagick \
ColorNode.cpp -o ColorNode.o
g++ -fPIC -shared -shared-libgcc -o ColorNode.so ColorNode.o  ../../wrapper/.libs/avg.so

#gcc -fPIC -shared -o ColorNode.so ColorNode.o

#chmod +x loader
#./loader
