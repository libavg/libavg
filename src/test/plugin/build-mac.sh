#!/bin/sh
#g++ loader.cpp Singleton.cpp -o loader

gcc -c -fno-common -I. -I. -I../../ -I.. -I/Users/regular/dev/avg/include/libxml2 -I/Users/regular/dev/avg/include/freetype2 -I/Users/regular/dev/avg/include -I/Users/regular/dev/avg/include/pango-1.0 -I/Users/regular/dev/avg/include -I/Users/regular/dev/avg/include/freetype2 -I/Users/regular/dev/avg/include/glib-2.0 -I/Users/regular/dev/avg/lib/glib-2.0/include   -I/System/Library/Frameworks/Python.framework/Versions/2.5/include/python2.5  -I/Users/regular/dev/avg/include -I/Users/regular/dev/avg/include/ffmpeg -I/Users/regular/dev/avg/include -I/Users/regular/dev/avg/include/GraphicsMagick    -I/Users/regular/dev/avg/include -I/Users/regular/dev/avg/include -I/Users/regular/dev/avg/include ColorNode.cpp -o ColorNode.o
gcc -bundle -flat_namespace -undefined suppress -o ColorNode.so ColorNode.o


#gcc -c -fno-common HelloWorldPlugin.cpp -o HelloWorldPlugin.o
#gcc -bundle -flat_namespace -undefined suppress -o HelloWorldPlugin.so HelloWorldPlugin.o
#otool -hv HelloWorldPlugin.so
#chmod +x loader
#./loader
