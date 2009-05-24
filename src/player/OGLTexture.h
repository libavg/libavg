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

#include "MaterialInfo.h"

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
    OGLTexture(IntPoint size, PixelFormat pf, const MaterialInfo& material,
            SDLDisplayEngine * pEngine, OGLMemoryMode memoryMode);
    virtual ~OGLTexture();

    BitmapPtr lockBmp();
    void unlockBmp();
    void download() const;

    void setMaterial(const MaterialInfo& material);
    const IntPoint& getTextureSize() const;
    unsigned getTexID() const;

private:
    void createBitmap();
    void createTexture();

    IntPoint m_Size;
    IntPoint m_ActiveSize;
    PixelFormat m_pf;
    MaterialInfo m_Material;
    
    unsigned m_TexID;
    GLuint m_hPixelBuffer;
    BitmapPtr m_pBmp;
    
    SDLDisplayEngine * m_pEngine;
    OGLMemoryMode m_MemoryMode;
};

typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;

}

#endif

