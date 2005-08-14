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
        virtual void create(const IntPoint& Size, PixelFormat PF) = 0;
        virtual BitmapPtr getBmp() = 0;

};

}

#endif

