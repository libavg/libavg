//
// $Id$
//

#ifndef _AVGFramerate_H_
#define _AVGFramerate_H_

class AVGFramerateManager
{
    public:
        AVGFramerateManager ();
        virtual ~AVGFramerateManager ();

        void SetRate(int Rate);
        int GetRate();
        void FrameWait();

    private:
        int getCurrentTicks();

        int m_NumFrames;
        int m_NumRegularFrames;
        int m_TimeSpentWaiting;
        int m_LastFrameTime;
        int m_Rate;
        int m_StartTime;
};

#endif //_AVGFramerate_H_
