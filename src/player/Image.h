//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#ifndef _Image_H_
#define _Image_H_

#include "../api.h"
#include "OGLTexture.h"

#include "../base/Point.h"
#include "../graphics/Bitmap.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class OGLSurface;
class SDLDisplayEngine;

class AVG_API Image
{
    public:
        enum State {NOT_AVAILABLE, CPU, GPU};

        Image(OGLSurface * pSurface, const std::string& sFilename);
        virtual ~Image();

        void setBitmap(const Bitmap * pBmp);

        virtual void moveToGPU(SDLDisplayEngine* pEngine);
        virtual void moveToCPU();

        void setFilename(const std::string& sFilename);
        const std::string& getFilename() const;
        
        BitmapPtr getBitmap();
        IntPoint getSize();
        PixelFormat getPixelFormat();
        OGLSurface* getSurface();
        State getState();
        SDLDisplayEngine* getEngine();

    private:
        void load();
        void setupSurface();
        PixelFormat calcSurfacePF(const Bitmap& Bmp);

        std::string m_sFilename;
        BitmapPtr m_pBmp;
        OGLSurface * m_pSurface;
        SDLDisplayEngine * m_pEngine;

        State m_State;
};

typedef boost::shared_ptr<Image> ImagePtr;

}

#endif

