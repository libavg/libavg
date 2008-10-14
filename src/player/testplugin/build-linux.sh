#!/bin/sh
g++ -shared-libgcc -fPIC -export-dynamic -ldl loader.cpp -o loader

gcc -fPIC -c -fno-common HelloWorldPlugin.cpp -o HelloWorldPlugin.o
gcc -fPIC -shared -shared-libgcc -o HelloWorldPlugin.so HelloWorldPlugin.o

chmod +x loader
./loader
