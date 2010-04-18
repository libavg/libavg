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

#ifndef _OGLSurface_H_
#define _OGLSurface_H_

#include "../api.h"
#include "OGLTexture.h"

#include "../base/Point.h"

#include "../graphics/Bitmap.h"
#include "../graphics/OGLHelper.h"

#include <vector>
#include <string>

namespace avg {

class SDLDisplayEngine;

class AVG_API OGLSurface {
public:
    OGLSurface(const MaterialInfo& material);
    virtual ~OGLSurface();

    void attach(SDLDisplayEngine * pEngine);
    virtual void create(const IntPoint& size, PixelFormat pf);
    void createMask(const IntPoint& size);
    virtual void destroy();
    void activate(const IntPoint& logicalSize = IntPoint(1,1)) const;
    void deactivate() const;

    BitmapPtr lockBmp(int i=0);
    void unlockBmps();
    BitmapPtr readbackBmp();
    void setTexID(unsigned id);

    BitmapPtr lockMaskBmp();
    void unlockMaskBmp();
    const MaterialInfo& getMaterial() const;
    void setMaterial(const MaterialInfo& material);

    void downloadTexture();

    PixelFormat getPixelFormat();
    IntPoint getSize();
    IntPoint getTextureSize();
    bool isCreated() const;

protected:
    SDLDisplayEngine * getEngine() const;

private:
    bool useShader() const;

    OGLTexturePtr m_pTextures[3];
    IntPoint m_Size;
    PixelFormat m_pf;
    OGLTexturePtr m_pMaskTexture;
    IntPoint m_MaskSize;
    bool m_bUseForeignTexture;

    MaterialInfo m_Material;

    SDLDisplayEngine * m_pEngine;
    OGLMemoryMode m_MemoryMode;
};

}

#endif

