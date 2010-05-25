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
        bool bUseInputPBO, bool bUseOutputPBO, bool bMipmap)
    : m_pfExt(pfExternal),
      m_bUseInputPBO(bUseInputPBO),
      m_bUseOutputPBO(bUseOutputPBO),
      m_InputPBO(0),
      m_OutputPBO(0)
{
    m_pTex = GLTexturePtr(new GLTexture(size, pfInternal, bMipmap));
    if (m_bUseInputPBO) {
        m_InputPBO = createInputPBO();
    }
    if (m_bUseOutputPBO) {
        m_OutputPBO = createOutputPBO();
    }
}

PBOImage::~PBOImage()
{
    if (m_bUseInputPBO) {
        deletePBO(&m_InputPBO);
    }
    if (m_bUseOutputPBO) {
        deletePBO(&m_OutputPBO);
    }
}

void PBOImage::setImage(BitmapPtr pBmp)
{
    IntPoint size = m_pTex->getSize();
    AVG_ASSERT(pBmp->getSize() == size);
    AVG_ASSERT(pBmp->getPixelFormat() == m_pfExt);
    AVG_ASSERT(m_bUseInputPBO);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_InputPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage BindBuffer()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage MapBuffer()");
    Bitmap PBOBitmap(size, m_pfExt, (unsigned char *)pPBOPixels, 
            size.x*Bitmap::getBytesPerPixel(m_pfExt), false); 
    PBOBitmap.copyPixels(*pBmp);
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: UnmapBuffer()");

    m_pTex->activate(GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y,
            GLTexture::getGLFormat(m_pfExt), GLTexture::getGLType(m_pfExt), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::setImage: glTexSubImage2D()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
}

BitmapPtr PBOImage::getImage() const
{
    AVG_ASSERT(m_bUseOutputPBO);
    IntPoint size = m_pTex->getSize();
    BitmapPtr pBmp(new Bitmap(size, m_pfExt));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_OutputPBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage BindBuffer()");

    m_pTex->activate(GL_TEXTURE0);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    glGetTexImage(GL_TEXTURE_2D, 0, GLTexture::getGLFormat(m_pfExt), 
            GLTexture::getGLType(m_pfExt), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PBOImage::getImage: glGetTexImage()");
    
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage MapBuffer()");
    Bitmap PBOBitmap(size, m_pfExt, (unsigned char *)pPBOPixels, 
            size.x*Bitmap::getBytesPerPixel(m_pfExt), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::getImage: UnmapBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    
    return pBmp;
}

void PBOImage::activateTex(int textureUnit)
{
    m_pTex->activate(textureUnit);
}
    
PixelFormat PBOImage::getIntPF() const
{
    return m_pTex->getPF();
}

PixelFormat PBOImage::getExtPF() const
{
    return m_pfExt;
}

const IntPoint& PBOImage::getSize() const
{
    return m_pTex->getSize();
}

unsigned PBOImage::getTexID() const
{
    return m_pTex->getID();
}

unsigned PBOImage::getOutputPBO() const
{
    AVG_ASSERT (m_bUseOutputPBO);
    return m_OutputPBO;
}

unsigned PBOImage::createInputPBO() const
{
    unsigned PBO;

    glproc::GenBuffers(1, &PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage: BindBuffer()");
    IntPoint size = m_pTex->getSize();
    int MemNeeded = size.x*size.y*Bitmap::getBytesPerPixel(m_pfExt);
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
    IntPoint size = m_pTex->getSize();
    int MemNeeded = size.x*size.y*Bitmap::getBytesPerPixel(m_pfExt);
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
