//
// $Id$
// 

#include "MathHelper.h"

bool ispow2(int n) {
    return (n & (n-1) == 0);
}

int nextpow2(int n) {
    int ret=1;
    while (ret<n) {
        ret *= 2;
    }
    return ret;
/* TODO: Fix this fast version :-).
    int RetVal = 1;
    __asm__ __volatile__(
        "xorl %%ecx, %%ecx\n\t"
        "bsrl %1, %%ecx\n\t"
        "incl %%ecx\n\t"
        "shlb %%cl, %0\n\t"
        : "=m" (RetVal)
        : "m" (n)
        : "cc", "ecx"
        );
    return RetVal;
*/    
}

