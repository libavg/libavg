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
#include "VertexArray.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

PBOImage::PBOImage(const IntPoint& size, PixelFormat pf, int precision)
    : m_pf(pf),
      m_Size(size),
      m_Precision(precision)
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

    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Size.x);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, getInternalFormat(), size.x, size.y, 0,
            getFormat(pf), GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glTexImage2D()");

    // Create a minimal vertex array to be used for drawing.
    m_pVertexes = new VertexArray(1);
    m_pVertexes->setPos(0, 0, DPoint(0, 0), DPoint(0, m_Size.y));
    m_pVertexes->setPos(0, 1, DPoint(0, m_Size.y), DPoint(0, 0));
    m_pVertexes->setPos(0, 2, DPoint(m_Size.x, m_Size.y), DPoint(m_Size.x, 0));
    m_pVertexes->setPos(0, 3, DPoint(m_Size.x, 0), DPoint(m_Size.x, m_Size.y));
}

PBOImage::~PBOImage()
{
    delete m_pVertexes;
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDeleteTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: DeleteTextures()");
    glproc::DeleteBuffers(1, &m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: DeleteBuffers()");
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
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pBmp->getStride()/pBmp->getBytesPerPixel());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, getInternalFormat(), m_Size.x, m_Size.y, 0,
            getFormat(pBmp->getPixelFormat()), GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexImage2D()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
}

void PBOImage::setImage(float * pData)
{
    // This is meant for 1D, 1-component float textures to be used as shader
    // lookup tables.
    // We use a temporary PBO here.
    assert (m_Size.y == 1);
    assert (m_pf == I8);
    assert (m_Precision == GL_FLOAT);

    unsigned TempPBO;
    glproc::GenBuffers(1, &TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: BindBuffer()");
    int MemNeeded = m_Size.x*sizeof(float);
    glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, MemNeeded, 0, GL_STREAM_DRAW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage BufferData()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage MapBuffer()");
    memcpy(pPBOPixels, pData, m_Size.x*sizeof(float));
    
    if (!glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT)) {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Size.x);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, getInternalFormat(), m_Size.x, 1, 0,
            GL_LUMINANCE, GL_FLOAT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexImage2D()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);

    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    glproc::DeleteBuffers(1, &TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: DeleteBuffers()");
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
    glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, getFormat(m_pf), GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glGetTexImage()");

    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage MapBuffer()");
    memcpy(pBmp->getPixels(), pPBOPixels, pBmp->getMemNeeded());

    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage BindBuffer(0)");
    return pBmp;
}

void PBOImage::activateTex(int textureUnit)
{
    glproc::ActiveTexture(textureUnit);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::activate ActiveTexture()");
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::activate BindTexture()");
}
    
void PBOImage::draw()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, m_Size.x, m_Size.y);

    glDisable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::draw: glBindTexture()");
    m_pVertexes->draw();
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

int PBOImage::getFormat(PixelFormat pf) const
{
    if (pf == I8) {
        return GL_LUMINANCE;
    } else {
        return GL_RGBA;
    }
}

int PBOImage::getInternalFormat() const
{
    switch(m_Precision) {
        case GL_UNSIGNED_BYTE:
            if (m_pf == I8) {
                return GL_LUMINANCE;
            } else {
                return GL_RGBA;
            }
        case GL_FLOAT:
#ifdef linux
            // Actually, this is probably an nvidia driver restriction.
            if (m_pf == I8) {
                return GL_FLOAT_R_NV;
            } else {
                return GL_FLOAT_RGBA_NV;
            }
#else
            if (m_pf == I8) {
                return GL_LUMINANCE32F_ARB;
            } else {
                return GL_RGBA32F_ARB;
            }
#endif
        default:
            assert(false);
            return 0;
    }
}

}
