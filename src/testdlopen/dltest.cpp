
#include <string>
#include <iostream>
#include "dlfcn.h"

using namespace std;

main() {
    std:: string myLibFile = "../.libs/libavg.so";
    
    void *myHandle = dlopen(myLibFile.c_str(),RTLD_NOW);
    if (!myHandle) {
        cerr << "dlopen(" << myLibFile << ") failed, reason = " << dlerror() << endl;
        exit(1);
    }
    const char * mySymbol = "NSGetModule";
  
    typedef void * (*FuncPtr)(void *, void *, void **);
    
    FuncPtr myFunc = (FuncPtr)dlsym(myHandle, mySymbol);
    char * error;
    if ((error = dlerror()) != NULL)  {
        cerr << "dlsym(" << mySymbol << ") failed, reason = " << error << endl;
        exit(1);
    }
    void * myResult;
    void * myReturnValue = myFunc(0,0,&myResult);
    cerr << "myResult = " << myResult << ", myReturnValue = myReturnValue" << endl;
    dlclose(myHandle);
    return 0;
}
