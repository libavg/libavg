//
// $Id$
//

#ifndef _IAVGSurface_H_
#define _IAVGSurface_H_

class PLBmpBase;

class IAVGSurface {
    public:
        virtual void create(int Width, int Height, int bpp, 
                bool bHasAlpha) = 0;

        virtual PLBmpBase* getBmp() = 0;
};

#endif

