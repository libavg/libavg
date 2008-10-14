#!/bin/sh
g++ -shared-libgcc -fPIC loader.cpp -o loader

gcc -fPIC -c -fno-common HelloWorldPlugin.cpp -o HelloWorldPlugin.o
gcc -fPIC -shared -shared-libgcc -o HelloWorldPlugin.so HelloWorldPlugin.o
otool -hv HelloWorldPlugin.so
chmod +x loader
./loader
