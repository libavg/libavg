//
// $Id$
//

#ifndef _AVGTime_H_ 
#define _AVGTime_H_


typedef unsigned long long CycleCount;

class AVGTimeSource {
public:
    static AVGTimeSource* get();
    ~AVGTimeSource();
   
    static long long getCurrentTicks();
    static __inline__ CycleCount rdtsc();

    void sleepUntil(long long ticks);

private:    
    AVGTimeSource();
    void tryOpenRTC();

    bool m_bUseRTC;
    int m_RTCFD;
    
    static AVGTimeSource* m_pTimeSource;

};

__inline__ CycleCount AVGTimeSource::rdtsc()
   {
     CycleCount x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
   }

#endif
