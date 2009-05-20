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

#ifndef _OGLTextureTile_H_
#define _OGLTextureTile_H_

#include "../api.h"
#include "../graphics/Bitmap.h"
#include "../graphics/OGLHelper.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;
class VertexArray;

typedef std::vector<std::vector<DPoint> > VertexGrid;

class AVG_API OGLTextureTile {
    public:
        OGLTextureTile(IntPoint size, PixelFormat pf, SDLDisplayEngine * pEngine);
        virtual ~OGLTextureTile();

        void downloadTexture(int i, BitmapPtr pBmp, OGLMemoryMode MemoryMode) const;
        void activate() const;
        void deactivate() const;

        const IntPoint& getTextureSize() const;

    private:
        void createTextures();
        void deleteTextures();

        IntPoint m_Size;
        IntPoint m_ActiveSize;
        PixelFormat m_pf;
        SDLDisplayEngine * m_pEngine;
        unsigned int m_TexID[3];
};

typedef boost::shared_ptr<OGLTextureTile> OGLTextureTilePtr;

}

#endif

