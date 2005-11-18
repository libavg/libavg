//
// $Id$
// 

#ifndef _VBlank_H_
#define _VBlank_H_

#include <string>

namespace avg {

class VBlank
{
    public:
        VBlank();
        virtual ~VBlank();

        bool init(int Rate);
        void wait();
        bool isActive() const;

    private:
        typedef enum Method {VB_SGI, VB_APPLE, VB_KAPUTT };
        int m_Rate;
        Method m_Method;
        int m_Mod;
        int m_LastFrame;
        bool m_bFirstFrame;
};

}

#endif 
