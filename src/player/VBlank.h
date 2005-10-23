//
// $Id$
// 

#ifndef _VBlank_H_
#define _VBlank_H_

#include <string>

// The DRI stuff was mostly taken from MythTV VBlank handling. Thanks, guys.

namespace avg {

class VBlank
{
    public:
        VBlank();
        virtual ~VBlank();

        void init();
        void wait();
        bool isActive() const;

    private:
        
        int m_dri_fd;
        typedef enum Method { VB_DRI, VB_NVIDIA, VB_APPLE, VB_KAPUTT };
        Method m_Method;
};

}

#endif 
