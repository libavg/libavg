gcc -c -fno-common HelloWorldPlugin.cpp -o HelloWorldPlugin.o
gcc -bundle -flat_namespace -undefined suppress -o HelloWorldPlugin.so HelloWorldPlugin.o
otool -hv HelloWorldPlugin.so
