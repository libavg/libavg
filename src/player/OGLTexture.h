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

#ifndef _OGLTexture_H_
#define _OGLTexture_H_

#include "../api.h"
#include "../graphics/Bitmap.h"
#include "../graphics/OGLHelper.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;
class VertexArray;

typedef std::vector<std::vector<DPoint> > VertexGrid;

class AVG_API OGLTexture {
    public:
        OGLTexture(IntPoint size, PixelFormat pf, int texWrapSMode, int texWrapTMode,
                SDLDisplayEngine * pEngine);
        virtual ~OGLTexture();

        void downloadTexture(int i, BitmapPtr pBmp, OGLMemoryMode MemoryMode) const;
        void activate() const;
        void deactivate() const;

        const IntPoint& getTextureSize() const;

    private:
        void createTextures();
        void deleteTextures();
        unsigned createTexture(IntPoint size, PixelFormat pf);

        IntPoint m_Size;
        IntPoint m_ActiveSize;
        PixelFormat m_pf;
        unsigned int m_TexID[3];
        int m_TexWrapSMode;
        int m_TexWrapTMode;
        SDLDisplayEngine * m_pEngine;
};

typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;

}

#endif

