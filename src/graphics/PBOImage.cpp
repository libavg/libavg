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

#include "PBOImage.h"
#include "VertexArray.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <iostream>
#include <cstring>

using namespace std;

namespace avg {

PBOImage::PBOImage(const IntPoint& size, PixelFormat pf, int precision, 
        bool bUseInputPBO, bool bUseOutputPBO)
    : m_pf(pf),
      m_Size(size),
      m_Precision(precision),
      m_bUseInputPBO(bUseInputPBO),
      m_bUseOutputPBO(bUseOutputPBO),
      m_InputPBO(0),
      m_OutputPBO(0)
{
    if (m_bUseInputPBO) {
        m_InputPBO = createInputPBO();
    }
    if (m_bUseOutputPBO) {
        m_OutputPBO = createOutputPBO();
    }
    
    // Create the texture and set it's size.
    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glGenTextures()");
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Size.x);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, getInternalFormat(), size.x, size.y, 0,
            getFormat(pf), GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: glTexImage2D()");

    // Create a minimal vertex array to be used for drawing.
    m_pVertexes = new VertexArray(4, 6);
    m_pVertexes->appendPos(DPoint(0, 0), DPoint(0, m_Size.y));
    m_pVertexes->appendPos(DPoint(0, m_Size.y), DPoint(0, 0));
    m_pVertexes->appendPos(DPoint(m_Size.x, m_Size.y), DPoint(m_Size.x, 0));
    m_pVertexes->appendPos(DPoint(m_Size.x, 0), DPoint(m_Size.x, m_Size.y));
    m_pVertexes->appendQuadIndexes(1, 0, 2, 3);
}

PBOImage::~PBOImage()
{
    delete m_pVertexes;
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDeleteTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: DeleteTextures()");
    if (m_bUseInputPBO) {
        deletePBO(&m_InputPBO);
    }
    if (m_bUseOutputPBO) {
        deletePBO(&m_OutputPBO);
    }
}

void PBOImage::setImage(BitmapPtr pBmp)
{
    assert(pBmp->getSize() == m_Size);
    assert(pBmp->getPixelFormat() == m_pf);
    assert(m_bUseInputPBO);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_InputPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage BindBuffer()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage MapBuffer()");
    Bitmap PBOBitmap(m_Size, m_pf, (unsigned char *)pPBOPixels, 
            m_Size.x*Bitmap::getBytesPerPixel(m_pf), false); 
    PBOBitmap.copyPixels(*pBmp);
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: UnmapBuffer()");

    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, m_Size.x, m_Size.y,
            getFormat(pBmp->getPixelFormat()), GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexSubImage2D()");
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
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: UnmapBuffer()");
    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Size.x);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, getInternalFormat(), m_Size.x, 1, 0,
            GL_LUMINANCE, GL_FLOAT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexImage2D()");

    glproc::DeleteBuffers(1, &TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: DeleteBuffers()");
}

BitmapPtr PBOImage::getImage() const
{
    assert(m_bUseOutputPBO);
    BitmapPtr pBmp(new Bitmap(m_Size, m_pf));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_OutputPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage BindBuffer()");

    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glBindTexture()");
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, getFormat(m_pf), GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glGetTexImage()");
/*
    glproc::GetBufferSubData(GL_PIXEL_PACK_BUFFER_EXT, 0, 
            m_Size.y*m_Size.x*Bitmap::getBytesPerPixel(m_pf), pBmp->getPixels());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage GetBufferSubData()");
*/    
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage MapBuffer()");
    Bitmap PBOBitmap(m_Size, m_pf, (unsigned char *)pPBOPixels, 
            m_Size.x*Bitmap::getBytesPerPixel(m_pf), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage: UnmapBuffer()");
    
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
    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::draw: glBindTexture()");
    m_pVertexes->draw();
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

unsigned PBOImage::createInputPBO() const
{
    unsigned PBO;

    glproc::GenBuffers(1, &PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BindBuffer()");
    int MemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pf);
    glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, MemNeeded, 0,
            GL_DYNAMIC_DRAW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BufferData()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);

    return PBO;
}

unsigned PBOImage::createOutputPBO() const
{
    unsigned PBO;

    glproc::GenBuffers(1, &PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BindBuffer()");
    int MemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pf);
    glproc::BufferData(GL_PIXEL_PACK_BUFFER_EXT, MemNeeded, 0,
            GL_DYNAMIC_READ);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BufferData()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);

    return PBO;
}

void PBOImage::deletePBO(unsigned* pPBO)
{
    glproc::DeleteBuffers(1, pPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: DeleteBuffers()");
}

}
