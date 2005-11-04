//
// $Id$
//

#ifndef _Framerate_H_
#define _Framerate_H_

#include "VBlank.h"

namespace avg {

class FramerateManager
{
    public:
        FramerateManager();
        virtual ~FramerateManager();

        void SetRate(double Rate, bool bSyncToVBlank);
        double GetRate();
        double GetRefreshRate();
        void FrameWait();
        void CheckJitter();

    private:
        int getCurrentTicks();
        void calcRefreshRate();

        int m_NumFrames;
        int m_FramesTooLate;
        int m_NumRegularFrames;
        long long m_TimeSpentWaiting;
        long long m_LastFrameTime;
        double m_Rate;
        bool m_bLastFrameLate;
        long long m_TotalJitter;
        long long m_StartTime;
        long long m_FrameWaitStartTime;

        double m_RefreshRate;

        VBlank m_VBlank;
};

}

#endif //_Framerate_H_
