//
// $Id$
//

#ifndef _AVGTime_H_ 
#define _AVGTime_H_

class AVGTimeSource {
public:
    static AVGTimeSource* get();
    ~AVGTimeSource();
   
    static int getCurrentTicks();

    void sleepUntil(int ticks);

private:    
    AVGTimeSource();
    void tryOpenRTC();

    bool m_bUseRTC;
    int m_RTCFD;
    
    static AVGTimeSource* m_pTimeSource;

};
#endif
