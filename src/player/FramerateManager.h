//
// $Id$
//

#ifndef _Framerate_H_
#define _Framerate_H_

namespace avg {

class FramerateManager
{
    public:
        FramerateManager();
        virtual ~FramerateManager();
        void Init();
        void Deinit();

        void SetRate(double Rate);
        bool SetVBlankRate(int Rate);
        double GetRate();
        static double GetRefreshRate();
        void FrameWait();
        void CheckJitter();

    private:
        void initVBlank();
        static void calcRefreshRate();

        int m_NumFrames;
        int m_FramesTooLate;
        long long m_TimeSpentWaiting;
        double m_Rate;
        bool m_bInitialized;
        long long m_StartTime;
        long long m_FrameWaitStartTime;
        long long m_TargetTime;
        long long m_LastFrameTime;

        typedef enum VBMethod {VB_SGI, VB_APPLE, VB_NONE };
        VBMethod m_VBMethod;
        int m_VBlankRate;
        int m_VBMod;
        int m_LastVBCount;
        bool m_bFirstVBFrame;

        static double s_RefreshRate;
};

}

#endif //_Framerate_H_
