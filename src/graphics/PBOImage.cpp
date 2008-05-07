//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "PBOImage.h"
#include "OGLHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

namespace avg {

PBOImage::PBOImage(const IntPoint& size, PixelFormat pf)
    : m_pf(pf),
      m_Size(size)
{
    // Create a Pixel Buffer Object for data transfer and allocate its memory.
    glproc::GenBuffers(1, &m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BindBuffer()");
    int MemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pf);
    glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, MemNeeded, 0,
            GL_STREAM_DRAW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BufferData()");

    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glGenTextures()");

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glBindTexture()");

    int OGLMode = getOGLMode(pf);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, OGLMode, size.x, size.y, 0,
            OGLMode, getOGLPixelType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glTexImage2D()");
}

PBOImage::~PBOImage()
{
    glDeleteTextures(1, &m_TexID);
    glproc::DeleteBuffers(1, &m_PBO);
}

void PBOImage::setImage(BitmapPtr pBmp)
{
    assert (pBmp->getSize() == m_Size);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage BindBuffer()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage MapBuffer()");
    memcpy(pPBOPixels, pBmp->getPixels(), pBmp->getMemNeeded());
    
    if (!glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT)) {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pBmp->getStride()/pBmp->getBytesPerPixel());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    int OGLMode = getOGLMode(m_pf);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, OGLMode, m_Size.x, m_Size.y, 0,
            OGLMode, getOGLPixelType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexImage2D()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
}

BitmapPtr PBOImage::getImage() const
{
    BitmapPtr pBmp(new Bitmap(m_Size, m_pf));

    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage BindBuffer()");

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glBindTexture()");
    glPixelStorei(GL_PACK_ROW_LENGTH, pBmp->getStride()/pBmp->getBytesPerPixel());
    int OGLMode = getOGLMode(m_pf);
    glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, OGLMode, getOGLPixelType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glGetTexImage()");

    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage MapBuffer()");
    memcpy(pBmp->getPixels(), pPBOPixels, pBmp->getMemNeeded());

    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    return pBmp;
}

PixelFormat PBOImage::getPF() const
{
    return m_pf;
}

const IntPoint& PBOImage::getSize() const
{
    return m_Size;
}

unsigned PBOImage::getTexID() const
{
    return m_TexID;
}

int PBOImage::getOGLMode(PixelFormat pf) const
{
    switch (pf) {
        case B8G8R8X8:
        case B8G8R8A8:
            return GL_RGBA;
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported pixel format " << 
                    Bitmap::getPixelFormatString(pf) <<
                    " in PBOImage::getOGLMode()");
            assert(false);
    }
    return 0;
}

int PBOImage::getOGLPixelType(PixelFormat pf) const
{
    if (pf == I16) {
        return GL_UNSIGNED_SHORT;
    } else {
        return GL_UNSIGNED_BYTE;
    }
}

}
