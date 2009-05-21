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

#include "OGLSurface.h"
#include "Player.h"
#include "SDLDisplayEngine.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

OGLSurface::OGLSurface(SDLDisplayEngine * pEngine, int texWrapSMode, int texWrapTMode)
    : m_pEngine(pEngine),
      m_bCreated(false),
      m_Size(-1,-1),
      m_TexWrapSMode(texWrapSMode),
      m_TexWrapTMode(texWrapTMode)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OGLSurface::~OGLSurface()
{
    if (m_bCreated) {
        deleteBuffers();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLSurface::create(const IntPoint& Size, PixelFormat pf, bool bFastDownload)
{
//    cerr << "OGLSurface::create: " << Size << ", " << Bitmap::getPixelFormatString(pf) << endl;
    if (m_bCreated && m_Size == Size && m_pf == pf) {
        // If nothing's changed, we can ignore everything.
        return;
    }
    if (m_bCreated) {
        deleteBuffers();
    }
    m_Size = Size;
    m_pf = pf;
    m_MemoryMode = OGL;
    if (bFastDownload) {
        m_MemoryMode = m_pEngine->getMemoryModeSupported();
    }
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        createBitmap(Size, I8, 0);
        IntPoint HalfSize(Size.x/2, Size.y/2);
        createBitmap(HalfSize, I8, 1);
        createBitmap(HalfSize, I8, 2);
    } else {
        createBitmap(Size, m_pf, 0);
    }
    m_pTexture = OGLTexturePtr(new OGLTexture(Size, pf, m_TexWrapSMode, m_TexWrapTMode, 
            getEngine()));
    
    m_bCreated = true;
}

BitmapPtr OGLSurface::lockBmp(int i)
{
//    cerr << "lockBmp " << i << endl;
    assert(m_bCreated);
    switch (m_MemoryMode) {
        case PBO:
            {
                glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[i]);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glBindBuffer()");
                unsigned char * pBuffer = (unsigned char *)
                    glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glMapBuffer()");
                glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glBindBuffer(0)");
                IntPoint Size;
                if (i == 0) {
                    Size = m_Size;
                } else {
                    Size = IntPoint(m_Size.x/2, m_Size.y/2);
                }
                PixelFormat pf;
                if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
                    pf = I8;
                } else {
                    pf = m_pf;
                }

                m_pBmps[i] = BitmapPtr(new Bitmap(Size, pf, pBuffer, 
                        Size.x*Bitmap::getBytesPerPixel(pf), false));
            }
            break;
        default:
            break;
    }
    return m_pBmps[i];
}

void OGLSurface::unlockBmps()
{
    assert(m_bCreated);
//    cerr << "unlockBmps" << endl;
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        for (int i=0; i<3; i++) {
            unlockBmp(i);
        }
    } else {
        m_pf = m_pBmps[0]->getPixelFormat();
        unlockBmp(0);
    }
}

void OGLSurface::bindPBO(int i) 
{
    assert(m_bCreated);
    assert(m_MemoryMode == PBO);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::bind: glBindBuffer()");
}

void OGLSurface::unbindPBO() 
{
    assert(m_bCreated);
    assert(m_MemoryMode == PBO);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::bind: glBindBuffer()");
}

void OGLSurface::downloadTexture()
{
    if (m_MemoryMode == PBO) {
        if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
            for (int i=0; i<3; i++) {
                bindPBO(i);
                m_pTexture->downloadTexture(i, m_pBmps[i], m_MemoryMode);
            }
        } else {
            bindPBO();
            m_pTexture->downloadTexture(0, m_pBmps[0], m_MemoryMode);
        }
        unbindPBO();
    } else {
        m_pTexture->downloadTexture(0, m_pBmps[0], m_MemoryMode);
    }
}

OGLTexturePtr OGLSurface::getTexture()
{
    return m_pTexture;
}

PixelFormat OGLSurface::getPixelFormat()
{
    return m_pf;
}
        
IntPoint OGLSurface::getSize()
{
    return m_Size;
}

SDLDisplayEngine * OGLSurface::getEngine()
{
    return m_pEngine;
}

BitmapPtr OGLSurface::getBmp(int i)
{
    return m_pBmps[i];
}

OGLMemoryMode OGLSurface::getMemMode() const
{
    return m_MemoryMode;
}


void OGLSurface::createBitmap(const IntPoint& Size, PixelFormat pf, int i)
{
    switch (m_MemoryMode) {
        case PBO:
            glproc::GenBuffers(1, &m_hPixelBuffers[i]);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glGenBuffers()");
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[i]);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glBindBuffer()");
            glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 
                    (Size.x+1)*(Size.y+1)*Bitmap::getBytesPerPixel(pf), NULL, 
                    GL_DYNAMIC_DRAW);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glBufferData()");
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::createBitmap: glBindBuffer(0)");
            m_pBmps[i] = BitmapPtr();
            break;
        default:
            break;
    }
    if (m_MemoryMode == OGL) {
        // Can't do this in the switch because memory allocation might fail.
        // In that case, this is needed as a fallback.
        // TODO: Huh? Really?
        m_pBmps[i] = BitmapPtr(new Bitmap(Size, pf));
    }
}

void OGLSurface::deleteBuffers()
{
    switch(m_MemoryMode) {
        case PBO:
            if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
                for (int i=0; i<3; i++) {
                    glproc::DeleteBuffers(1, &m_hPixelBuffers[i]);
                }
            } else {
                glproc::DeleteBuffers(1, &m_hPixelBuffers[0]);
            }
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::~OGLSurface: glDeleteBuffers()");
            break;
        default:
            break;
    }
    m_bCreated = false;
}

void OGLSurface::unlockBmp(int i) 
{
//    cerr << "unlockBmp" << endl;
    switch (m_MemoryMode) {
        case PBO:
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[i]);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::unlockBmp: glBindBuffer()");
            glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::unlockBmp: glUnmapBuffer()");
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::lockBmp: glBindBuffer(0)");
            m_pBmps[i] = BitmapPtr();
            break;
        default:
            break;
    }
}

}
