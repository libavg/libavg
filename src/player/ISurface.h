//
// $Id$
//

#ifndef _ISurface_H_
#define _ISurface_H_

#include "../graphics/Bitmap.h"

#include <string>

namespace avg {

class ISurface {
    public:
        virtual ~ISurface(){};
        virtual void create(const IntPoint& Size, PixelFormat PF, 
                bool bFastDownload) = 0;
        virtual BitmapPtr lockBmp() = 0;
        virtual void unlockBmp() {};

};

}

#endif

