//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#ifndef _PBOTexture_H_
#define _PBOTexture_H_

#include "../api.h"

#include "MaterialInfo.h"

#include "../graphics/OGLHelper.h"
#include "../graphics/PixelFormat.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class VertexArray;
class TextureMover;
typedef boost::shared_ptr<TextureMover> TextureMoverPtr;
class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;
class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;

class AVG_API PBOTexture {
public:
    PBOTexture(IntPoint size, PixelFormat pf, const MaterialInfo& material);
    virtual ~PBOTexture();

    BitmapPtr lockBmp();
    void unlockBmp();
    BitmapPtr readbackBmp();
    void download() const;
    void setTex(GLTexturePtr pTex);
    void activate(int textureUnit=GL_TEXTURE0);

    const IntPoint& getTextureSize() const;

private:
    void createMover();

    PixelFormat m_pf;
    MaterialInfo m_Material;
    
    GLTexturePtr m_pTex;
    TextureMoverPtr m_pWriteMover;
    TextureMoverPtr m_pReadMover;
    
    OGLMemoryMode m_MemoryMode;
};

typedef boost::shared_ptr<PBOTexture> PBOTexturePtr;

}

#endif

