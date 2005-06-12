//
// $Id$
//

#ifndef _ISurface_H_
#define _ISurface_H_

class PLBmpBase;
class PLPixelFormat;

namespace avg {

class ISurface {
    public:
        virtual ~ISurface(){};
        virtual void create(int Width, int Height, const PLPixelFormat& pf) = 0;
        virtual PLBmpBase* getBmp() = 0;

};

}

#endif

