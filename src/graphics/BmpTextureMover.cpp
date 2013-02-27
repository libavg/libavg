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
#include "FBO.h"
#include "Filterfliprgb.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "GLContext.h"

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

void BmpTextureMover::moveBmpToTexture(BitmapPtr pBmp, GLTexture& tex)
{
    AVG_ASSERT(pBmp->getSize() == tex.getSize());
    AVG_ASSERT(getSize() == pBmp->getSize());
    AVG_ASSERT(pBmp->getPixelFormat() == getPF());
    tex.activate();
    unsigned char * pStartPos = pBmp->getPixels();
    IntPoint size = tex.getSize();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y,
            tex.getGLFormat(getPF()), tex.getGLType(getPF()), 
            pStartPos);
    tex.setDirty();
    tex.generateMipmaps();
    GLContext::checkError("BmpTextureMover::moveBmpToTexture: glTexSubImage2D()");
}

BitmapPtr BmpTextureMover::moveTextureToBmp(GLTexture& tex, int mipmapLevel)
{
    GLContext* pContext = GLContext::getCurrent();
    unsigned fbo = pContext->genFBO();
    glproc::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    glproc::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            tex.getID(), mipmapLevel);
    FBO::checkError("BmpTextureMover::moveTextureToBmp");
    IntPoint size = tex.getMipmapSize(mipmapLevel);
    BitmapPtr pBmp(new Bitmap(size, getPF()));
    if (GLContext::getMain()->isGLES() && getPF() == B5G6R5) {
        BitmapPtr pTmpBmp(new Bitmap(size, R8G8B8A8));
        glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, pTmpBmp->getPixels());
        FilterFlipRGB().applyInPlace(pTmpBmp);
        pBmp->copyPixels(*pTmpBmp);
    } else {
        int glPixelFormat = tex.getGLFormat(getPF());
        glReadPixels(0, 0, size.x, size.y, glPixelFormat, tex.getGLType(getPF()), 
                pBmp->getPixels());
    }
    GLContext::checkError("BmpTextureMover::moveTextureToBmp: glReadPixels()");
    glproc::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
            0, 0);
    pContext->returnFBOToCache(fbo);
    glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
    return pBmp;

}

BitmapPtr BmpTextureMover::lock()
{
    return m_pBmp;
}

void BmpTextureMover::unlock()
{
}

void BmpTextureMover::moveToTexture(GLTexture& tex)
{
    moveBmpToTexture(m_pBmp, tex);
}

}
