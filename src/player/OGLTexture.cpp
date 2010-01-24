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

#include "OGLTexture.h"
#include "SDLDisplayEngine.h"
#include "../graphics/VertexArray.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <string>

namespace avg {

using namespace std;
    
OGLTexture::OGLTexture(IntPoint size, PixelFormat pf, const MaterialInfo& material,
        SDLDisplayEngine * pEngine, OGLMemoryMode memoryMode) 
    : m_pf(pf),
      m_Material(material),
      m_pEngine(pEngine),
      m_MemoryMode(memoryMode)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_ActiveSize = size;
    if (pEngine->usePOTTextures()) {
        m_Size.x = nextpow2(m_ActiveSize.x);
        m_Size.y = nextpow2(m_ActiveSize.y);
    } else {
        m_Size = m_ActiveSize;
    }
    createBitmap();
    createTexture();
}

OGLTexture::~OGLTexture()
{
    glDeleteTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::~OGLTexture: glDeleteTextures()");
    if (m_MemoryMode == PBO) {
        glproc::DeleteBuffers(1, &m_hWritePixelBuffer);
        glproc::DeleteBuffers(1, &m_hReadPixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTexture::~OGLTexture: glDeleteBuffers()");
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr OGLTexture::lockBmp()
{
    if (m_MemoryMode == PBO) {
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hWritePixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::lockBmp: glBindBuffer()");
        glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 
                (m_ActiveSize.x+1)*(m_ActiveSize.y+1)*Bitmap::getBytesPerPixel(m_pf),
                0, GL_DYNAMIC_DRAW);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::lockBmp: glBufferData()");
        unsigned char * pBuffer = (unsigned char *)
            glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::lockBmp: glMapBuffer()");
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::lockBmp: glBindBuffer(0)");

        m_pBmp = BitmapPtr(new Bitmap(m_ActiveSize, m_pf, pBuffer, 
                    m_ActiveSize.x*Bitmap::getBytesPerPixel(m_pf), false));
    }
    return m_pBmp;
}

void OGLTexture::unlockBmp()
{
    if (m_MemoryMode == PBO) {
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hWritePixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTexture::unlockBmp: glBindBuffer()");
        glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTexture::unlockBmp: glUnmapBuffer()");
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTexture::unlockBmp: glBindBuffer(0)");
        m_pBmp = BitmapPtr();
    }
}

BitmapPtr OGLTexture::readbackBmp()
{
    if (m_MemoryMode == PBO) {
        glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_hReadPixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glBindBuffer()");
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTexture::readbackBmp: GL_PACK_ALIGNMENT");
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glproc::BufferData(GL_PIXEL_PACK_BUFFER_EXT, 
                m_ActiveSize.x*m_ActiveSize.y*Bitmap::getBytesPerPixel(m_pf),
                0, GL_STREAM_READ);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glBufferData()");
        glBindTexture(GL_TEXTURE_2D, m_TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glBindTexture()");
        glGetTexImage(GL_TEXTURE_2D, 0, m_pEngine->getOGLSrcMode(m_pf), 
                m_pEngine->getOGLPixelType(m_pf), 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glGetTexImage()");
        unsigned char * pBuffer = (unsigned char *)
                glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glMapBuffer()");
        BitmapPtr pBmp = BitmapPtr(new Bitmap(m_ActiveSize, m_pf, pBuffer, 
                    m_ActiveSize.x*Bitmap::getBytesPerPixel(m_pf), true));
        glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glUnmapBuffer()");
        glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::readbackBmp: glBindBuffer(0)");
        return pBmp;
    } else {
        return BitmapPtr(new Bitmap(*m_pBmp));
    }
}

static ProfilingZone TexSubImageProfilingZone("OGLTexture::texture download");
static ProfilingZone MipmapProfilingZone("OGLTexture::mipmap generation");

void OGLTexture::download() const
{
    if (m_MemoryMode == PBO) {
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hWritePixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::download: glBindBuffer()");
    }

    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::download: glBindTexture()");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::download: GL_UNPACK_ALIGNMENT");
    unsigned char * pStartPos = 0;
    if (m_MemoryMode == OGL) {
        pStartPos += (ptrdiff_t)(m_pBmp->getPixels());
    }
    {
        ScopeTimer Timer(TexSubImageProfilingZone);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_ActiveSize.x, m_ActiveSize.y,
                m_pEngine->getOGLSrcMode(m_pf), m_pEngine->getOGLPixelType(m_pf), 
                pStartPos);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::download: glTexSubImage2D()");
    int texFilter;
    if (m_Material.getUseMipmaps()) {
        ScopeTimer Timer(MipmapProfilingZone);
        glproc::GenerateMipmap(GL_TEXTURE_2D);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTexture::download: GenerateMipmap()");
        texFilter = GL_LINEAR_MIPMAP_LINEAR;
    } else {
        texFilter = GL_LINEAR;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::download: glTexParameteri()");
    if (m_MemoryMode == PBO) {
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::download: glBindBuffer(0)");
    }
}

void OGLTexture::setMaterial(const MaterialInfo& material)
{
    m_Material = material;
}

const IntPoint& OGLTexture::getTextureSize() const
{
    return m_Size;
}

unsigned OGLTexture::getTexID() const 
{
    return m_TexID;
}

void OGLTexture::createBitmap()
{
    switch (m_MemoryMode) {
        case PBO:
            glproc::GenBuffers(1, &m_hReadPixelBuffer);
            glproc::GenBuffers(1, &m_hWritePixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glGenBuffers()");
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hWritePixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glBindBuffer()");
            glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 
                    (m_ActiveSize.x+1)*(m_ActiveSize.y+1)*Bitmap::getBytesPerPixel(m_pf),
                    0, GL_DYNAMIC_DRAW);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glBufferData()");
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glBindBuffer(0)");
            m_pBmp = BitmapPtr();
            break;
        case OGL:
            m_pBmp = BitmapPtr(new Bitmap(m_ActiveSize, m_pf));
            break;
        default:
            assert(0);
    }
}

void OGLTexture::createTexture()
{
    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::createTexture: glGenTextures()");
    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::createTexture: glBindTexture()");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_Material.getTexWrapSMode());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_Material.getTexWrapTMode());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::createTexture: glTexParameteri()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    char * pPixels = 0;
    if (m_pEngine->usePOTTextures()) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        int TexMemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pf);
        pPixels = new char[TexMemNeeded];
        memset(pPixels, 0, TexMemNeeded);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, m_pEngine->getOGLDestMode(m_pf), m_Size.x, m_Size.y, 
            0, m_pEngine->getOGLSrcMode(m_pf), m_pEngine->getOGLPixelType(m_pf), 
            pPixels);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::createTexture: glTexImage2D()");
    if (m_pEngine->usePOTTextures()) {
        free(pPixels);
    }
}

}
