//
// $Id$
//

#ifndef _AVGTime_H_ 
#define _AVGTime_H_

class AVGTimeSource {
public:
    static AVGTimeSource* get();
    ~AVGTimeSource();
   
    static long long getCurrentTicks();
    static __inline__ unsigned long long int rdtsc();

    void sleepUntil(long long ticks);

private:    
    AVGTimeSource();
    void tryOpenRTC();

    bool m_bUseRTC;
    int m_RTCFD;
    
    static AVGTimeSource* m_pTimeSource;

};

__inline__ unsigned long long int AVGTimeSource::rdtsc()
   {
     unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
   }

#endif
