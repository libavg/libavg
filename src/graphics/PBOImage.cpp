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

PBOImage::PBOImage(const IntPoint& size, PixelFormat pfInternal, PixelFormat pfExternal,
        bool bUseInputPBO, bool bUseOutputPBO)
    : m_pfInt(pfInternal),
      m_pfExt(pfExternal),
      m_Size(size),
      m_bUseInputPBO(bUseInputPBO),
      m_bUseOutputPBO(bUseOutputPBO),
      m_InputPBO(0),
      m_OutputPBO(0)
{
    assert(getFormat(m_pfInt) == getFormat(m_pfExt));
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
            getFormat(m_pfExt), getType(m_pfExt), 0);
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
    assert(pBmp->getPixelFormat() == m_pfExt);
    assert(m_bUseInputPBO);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_InputPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage BindBuffer()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage MapBuffer()");
    Bitmap PBOBitmap(m_Size, m_pfExt, (unsigned char *)pPBOPixels, 
            m_Size.x*Bitmap::getBytesPerPixel(m_pfExt), false); 
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
            getFormat(m_pfExt), getType(m_pfExt), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexSubImage2D()");
}

void PBOImage::setImage(float * pData)
{
    // We use a temporary PBO here.
    assert (getType(m_pfExt) == GL_FLOAT);

    unsigned TempPBO;
    glproc::GenBuffers(1, &TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: BindBuffer()");
    int memNeeded = m_Size.x*m_Size.y * Bitmap::getBytesPerPixel(m_pfExt);
    glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, memNeeded, 0, GL_STREAM_DRAW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage BufferData()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage MapBuffer()");
    memcpy(pPBOPixels, pData, memNeeded);
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: UnmapBuffer()");
    
    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Size.x);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, getInternalFormat(), m_Size.x, m_Size.y, 0,
        getFormat(m_pfExt), getType(m_pfExt), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexImage2D()");
    /*
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, m_Size.x, m_Size.y,
            getFormat(m_pfExt), getType(m_pfExt), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexSubImage2D()");
*/
    glproc::DeleteBuffers(1, &TempPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: DeleteBuffers()");
}

BitmapPtr PBOImage::getImage() const
{
    assert(m_bUseOutputPBO);
    BitmapPtr pBmp(new Bitmap(m_Size, m_pfExt));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_OutputPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage BindBuffer()");

    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glBindTexture()");
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, getFormat(m_pfExt), getType(m_pfExt), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glGetTexImage()");
/*
    glproc::GetBufferSubData(GL_PIXEL_PACK_BUFFER_EXT, 0, 
            m_Size.y*m_Size.x*Bitmap::getBytesPerPixel(m_pf), pBmp->getPixels());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage GetBufferSubData()");
*/    
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage MapBuffer()");
    Bitmap PBOBitmap(m_Size, m_pfExt, (unsigned char *)pPBOPixels, 
            m_Size.x*Bitmap::getBytesPerPixel(m_pfExt), false);
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

PixelFormat PBOImage::getIntPF() const
{
    return m_pfInt;
}
PixelFormat PBOImage::getExtPF() const
{
    return m_pfExt;
}

const IntPoint& PBOImage::getSize() const
{
    return m_Size;
}

unsigned PBOImage::getTexID() const
{
    return m_TexID;
}

unsigned PBOImage::getOutputPBO() const
{
    assert (m_bUseOutputPBO);
    return m_OutputPBO;
}

int PBOImage::getType(PixelFormat pf) const
{
    switch (pf) {
        case I8:
        case R8G8B8A8:
        case R8G8B8X8:
        case B8G8R8A8:
        case B8G8R8X8:
            return GL_UNSIGNED_BYTE;
        case R32G32B32A32F:
        case I32F:
            return GL_FLOAT;
        default:
            assert(false);
            return 0;
    }
}
int PBOImage::getFormat(PixelFormat pf) const
{
    switch (pf) {
        case I8:
        case I32F:
            return GL_LUMINANCE;
        case R8G8B8A8:
        case R8G8B8X8:
        case B8G8R8A8:
        case B8G8R8X8:
        case R32G32B32A32F:
            return GL_RGBA;
        default:
            assert(false);
            return 0;
    }
}

int PBOImage::getInternalFormat() const
{
#ifdef linux
// Actually, this is probably an nvidia driver restriction.
#   define MY_GL_LUMINANCE32F   GL_FLOAT_R_NV
#   define MY_GL_RGBA32F        GL_FLOAT_RGBA_NV
#else
#   define MY_GL_LUMINANCE32F   GL_LUMINANCE32F_ARB
#   define MY_GL_RGBA32F        GL_RGBA32F_ARB;
#endif
    switch (m_pfInt) {
        case I8:
            return GL_LUMINANCE;
        case I32F:
            return MY_GL_LUMINANCE32F;
        case R8G8B8A8:
        case R8G8B8X8:
        case B8G8R8A8:
        case B8G8R8X8:
            return GL_RGBA;
        case R32G32B32A32F:
            return MY_GL_RGBA32F;
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
    int MemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pfExt);
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
    int MemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pfExt);
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
