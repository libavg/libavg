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

    void sleepUntil(long long ticks);

private:    
    AVGTimeSource();
    void tryOpenRTC();

    bool m_bUseRTC;
    int m_RTCFD;
    
    static AVGTimeSource* m_pTimeSource;

};
#endif
