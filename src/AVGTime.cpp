//
// $Id$
//

#include "AVGTime.h"

#include <sys/time.h>
#include <unistd.h>

int GetCurrentTicks()
{
    struct timeval now;
    int ticks;

    gettimeofday(&now, NULL);
    ticks=now.tv_sec*1000+now.tv_usec/1000;
    return(ticks);
}



