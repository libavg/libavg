#!/bin/sh
g++ loader.cpp Singleton.cpp -o loader

gcc -c -fno-common HelloWorldPlugin.cpp -o HelloWorldPlugin.o
gcc -bundle -flat_namespace -undefined suppress -o HelloWorldPlugin.so HelloWorldPlugin.o
otool -hv HelloWorldPlugin.so
chmod +x loader
./loader
