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

#include "PBO.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <iostream>
#include <cstring>

using namespace std;

namespace avg {

PBO::PBO(const IntPoint& size, PixelFormat pf, unsigned usage)
    : m_Size(size),
      m_pf(pf),
      m_Usage(usage)
{
    glproc::GenBuffers(1, &m_PBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO: GenBuffers()");
    unsigned target = getTarget();
    glproc::BindBuffer(target, m_PBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO: BindBuffer()");
    int MemNeeded = size.x*size.y*Bitmap::getBytesPerPixel(m_pf);
    glproc::BufferData(target, MemNeeded, 0, usage);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO: BufferData()");
    glproc::BindBuffer(target, 0);
}

PBO::~PBO()
{
    glproc::DeleteBuffers(1, &m_PBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO: DeleteBuffers()");
}

void PBO::activate()
{
    glproc::BindBuffer(getTarget(), m_PBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::activate()");
    
}

void PBO::deactivate()
{
    glproc::BindBuffer(getTarget(), 0);
}

void PBO::moveBmpToTexture(BitmapPtr pBmp, GLTexturePtr pTex)
{

    AVG_ASSERT(pBmp->getSize() == pTex->getSize());
    AVG_ASSERT(m_Size == pBmp->getSize());
    AVG_ASSERT(pBmp->getPixelFormat() == m_pf);
    AVG_ASSERT(!isReadPBO());
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_PBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::moveBmpToTexture BindBuffer()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::moveBmpToTexture MapBuffer()");
    Bitmap PBOBitmap(m_Size, m_pf, (unsigned char *)pPBOPixels, 
            m_Size.x*Bitmap::getBytesPerPixel(m_pf), false); 
    PBOBitmap.copyPixels(*pBmp);
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::setImage: UnmapBuffer()");

    pTex->activate(GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBO::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Size.x, m_Size.y,
            GLTexture::getGLFormat(m_pf), GLTexture::getGLType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::setImage: glTexSubImage2D()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
}

BitmapPtr PBO::moveTextureToBmp(GLTexturePtr pTex) const
{
    AVG_ASSERT(isReadPBO());
    AVG_ASSERT(m_Size == pTex->getSize());
    BitmapPtr pBmp(new Bitmap(m_Size, m_pf));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_PBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::getImage BindBuffer()");

    pTex->activate(GL_TEXTURE0);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    glGetTexImage(GL_TEXTURE_2D, 0, GLTexture::getGLFormat(m_pf), 
            GLTexture::getGLType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::getImage: glGetTexImage()");
    
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::getImage MapBuffer()");
    Bitmap PBOBitmap(m_Size, m_pf, (unsigned char *)pPBOPixels, 
            m_Size.x*Bitmap::getBytesPerPixel(m_pf), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO::getImage: UnmapBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    
    return pBmp;
}

PixelFormat PBO::getPF() const
{
    return m_pf;
}

const IntPoint& PBO::getSize() const
{
    return m_Size;
}

bool PBO::isReadPBO() const
{
    switch (m_Usage) {
        case GL_STATIC_DRAW:
        case GL_DYNAMIC_DRAW:
        case GL_STREAM_DRAW:
            return false;
        case GL_STATIC_READ:
        case GL_DYNAMIC_READ:
        case GL_STREAM_READ:
            return true;
        default:
            AVG_ASSERT(false);
            return false;
    }
}

unsigned PBO::getTarget() const
{
    if (isReadPBO()) {
        return GL_PIXEL_PACK_BUFFER_EXT;
    } else {
        return GL_PIXEL_UNPACK_BUFFER_EXT;
    }
}

}
