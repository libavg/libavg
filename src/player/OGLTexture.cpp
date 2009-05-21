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

#include <iostream>
#include <string>

namespace avg {

using namespace std;
    
OGLTexture::OGLTexture(IntPoint size, PixelFormat pf, int texWrapSMode, int texWrapTMode,
        SDLDisplayEngine * pEngine) 
    : m_Size(size),
      m_pf(pf),
      m_TexWrapSMode(texWrapSMode),
      m_TexWrapTMode(texWrapTMode),
      m_pEngine(pEngine)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_ActiveSize = m_Size;
    if (pEngine->usePOTTextures()) {
        m_Size.x = nextpow2(m_Size.x);
        m_Size.y = nextpow2(m_Size.y);
    }
    createTextures();
}

OGLTexture::~OGLTexture()
{
    deleteTextures();
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLTexture::activate() const
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        OGLShaderPtr pShader;
        if (m_pf == YCbCr420p) {
            pShader = m_pEngine->getYCbCr420pShader();
        } else {
            pShader = m_pEngine->getYCbCrJ420pShader();
        }
        pShader->activate();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::blt: glUseProgramObject()");
        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_TexID[0]);
        pShader->setUniformIntParam("YTexture", 0);
        glproc::ActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_TexID[1]);
        pShader->setUniformIntParam("CbTexture", 1);
        glproc::ActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_TexID[2]);
        pShader->setUniformIntParam("CrTexture", 2);
    } else {
        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_TexID[0]);
        if (m_pEngine->isUsingYCbCrShaders()) {
            glproc::UseProgramObject(0);
        }
    }
} 

void OGLTexture::deactivate() const
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        glproc::ActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glproc::ActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
        glproc::ActiveTexture(GL_TEXTURE0);
        glproc::UseProgramObject(0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::blt: glDisable(GL_TEXTURE_2D)");
    }
}
        
const IntPoint& OGLTexture::getTextureSize() const
{
    return m_Size;
}

void OGLTexture::createTextures()
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        m_TexID[0] = createTexture(m_Size, I8);
        m_TexID[1] = createTexture(m_Size/2, I8);
        m_TexID[2] = createTexture(m_Size/2, I8);
    } else {
        m_TexID[0] = createTexture(m_Size, m_pf);
    }
}

void OGLTexture::deleteTextures()
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        glDeleteTextures(3, m_TexID);
    } else {
        glDeleteTextures(1, m_TexID);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::~OGLTexture: glDeleteTextures()");    
}

static ProfilingZone TexSubImageProfilingZone("OGLTexture::texture download");

void OGLTexture::downloadTexture(int i, BitmapPtr pBmp, OGLMemoryMode MemoryMode) const
{
    PixelFormat pf;
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        pf = I8;
    } else {
        pf = m_pf;
    }
    IntPoint size = m_ActiveSize;
    if (i != 0) {
        size /= 2;
    }
    glBindTexture(GL_TEXTURE_2D, m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::downloadTexture: glBindTexture()");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::downloadTexture: GL_UNPACK_ALIGNMENT");
    unsigned char * pStartPos = 0;
    if (MemoryMode == OGL) {
        pStartPos += (ptrdiff_t)(pBmp->getPixels());
    }
    {
        ScopeTimer Timer(TexSubImageProfilingZone);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y,
                m_pEngine->getOGLSrcMode(pf), m_pEngine->getOGLPixelType(pf), 
                pStartPos);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::downloadTexture: glTexSubImage2D()");
}

unsigned OGLTexture::createTexture(IntPoint size, PixelFormat pf)
{
    unsigned texID;
    glGenTextures(1, &texID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::createTexture: glGenTextures()");
    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::createTexture: glBindTexture()");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_TexWrapSMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_TexWrapTMode);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::createTexture: glTexParameteri()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    char * pPixels = 0;
    if (m_pEngine->usePOTTextures()) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        int TexMemNeeded = size.x*size.y*Bitmap::getBytesPerPixel(pf);
        pPixels = new char[TexMemNeeded];
        memset(pPixels, 0, TexMemNeeded);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, m_pEngine->getOGLDestMode(pf), size.x, size.y, 
            0, m_pEngine->getOGLSrcMode(pf), m_pEngine->getOGLPixelType(pf), 
            pPixels);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::createTexture: glTexImage2D()");
    if (m_pEngine->usePOTTextures()) {
        free(pPixels);
    }
    return texID;
}

}
