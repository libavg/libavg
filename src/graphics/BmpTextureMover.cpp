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

#include "BmpTextureMover.h"

#include "Bitmap.h"
#include "GLTexture.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <iostream>
#include <cstring>

using namespace std;
using namespace boost;

namespace avg {
    
BmpTextureMover::BmpTextureMover(const IntPoint& size, PixelFormat pf)
    : TextureMover(size, pf)
{
    m_pBmp = BitmapPtr(new Bitmap(size, pf));
}

BmpTextureMover::~BmpTextureMover()
{
}

void BmpTextureMover::moveBmpToTexture(BitmapPtr pBmp, GLTexturePtr pTex)
{
    AVG_ASSERT(pBmp->getSize() == pTex->getSize());
    AVG_ASSERT(getSize() == pBmp->getSize());
    AVG_ASSERT(pBmp->getPixelFormat() == getPF());
    pTex->activate();
    unsigned char * pStartPos = pBmp->getPixels();
    IntPoint size = pTex->getSize();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y,
            pTex->getGLFormat(getPF()), pTex->getGLType(getPF()), 
            pStartPos);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "BmpTextureMover::moveBmpToTexture: glTexSubImage2D()");
}

BitmapPtr BmpTextureMover::moveTextureToBmp(GLTexturePtr pTex)
{
    AVG_ASSERT(getSize() == pTex->getGLSize());
    BitmapPtr pBmp(new Bitmap(pTex->getGLSize(), getPF()));

    pTex->activate(GL_TEXTURE0);

    unsigned char * pStartPos = pBmp->getPixels();
    glGetTexImage(GL_TEXTURE_2D, 0, GLTexture::getGLFormat(getPF()), 
            GLTexture::getGLType(getPF()), pStartPos);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "BmpTextureMover::moveTextureToBmp: glGetTexImage()");
    
    IntPoint activeSize = pTex->getSize();
    if (activeSize != pTex->getGLSize()) {
        BitmapPtr pTempBmp = pBmp;
        pBmp = BitmapPtr(new Bitmap(activeSize, getPF(), pStartPos, 
                pTempBmp->getStride(), true)); 
    }
    
    return pBmp;
}

BitmapPtr BmpTextureMover::lock()
{
    return m_pBmp;
}

void BmpTextureMover::unlock()
{
}

void BmpTextureMover::moveToTexture(GLTexturePtr pTex)
{
    moveBmpToTexture(m_pBmp, pTex);
}

}
