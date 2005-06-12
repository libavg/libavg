//
// $Id$
//

#ifndef _Framerate_H_
#define _Framerate_H_

namespace avg {

class FramerateManager
{
    public:
        FramerateManager ();
        virtual ~FramerateManager ();

        void SetRate(double Rate);
        double GetRate();
        void FrameWait();
        void CheckJitter();

    private:
        int getCurrentTicks();

        int m_NumFrames;
        int m_FramesTooLate;
        int m_NumRegularFrames;
        long long m_TimeSpentWaiting;
        long long m_LastFrameTime;
        double m_Rate;
        bool m_bLastFrameLate;
        long long m_TotalJitter;
        long long m_StartTime;
        
};

}

#endif //_Framerate_H_
