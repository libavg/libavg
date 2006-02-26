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

#include "OGLTile.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <iostream>
#include <string>

namespace avg {

using namespace std;
    
OGLTile::OGLTile(IntRect Extent, IntPoint TexSize, PixelFormat pf, 
        SDLDisplayEngine * pEngine) 
    : m_Extent(Extent),
      m_TexSize(TexSize),
      m_pf(pf),
      m_pEngine(pEngine)
{
    glGenTextures(1, &m_TexID[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::OGLTile: glGenTextures()");
    glBindTexture(m_pEngine->getTextureMode(), m_TexID[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::OGLTile: glBindTexture()");

    glTexImage2D(m_pEngine->getTextureMode(), 0,
            m_pEngine->getOGLDestMode(m_pf), m_TexSize.x, m_TexSize.y, 0,
            m_pEngine->getOGLSrcMode(m_pf), m_pEngine->getOGLPixelType(m_pf), 0); 
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::OGLTile: glTexImage2D()");
}

OGLTile::~OGLTile()
{
    glDeleteTextures(1, &m_TexID[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::~OGLTile: glDeleteTextures()");
}

const IntRect& OGLTile::getExtent() const
{
    return m_Extent;
}

const IntPoint& OGLTile::getTexSize() const
{
    return m_TexSize;
}

int OGLTile::getTexID(int i) const
{
    return m_TexID[i];
}

static ProfilingZone TexSubImageProfilingZone("    OGLTile::texture download");

void OGLTile::downloadTextures(BitmapPtr pBmp, int width, 
        OGLMemoryMode MemoryMode) const
{
    int TextureMode = m_pEngine->getTextureMode();
    glBindTexture(TextureMode, m_TexID[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::downloadTextures: glBindTexture()");
    int bpp = Bitmap::getBytesPerPixel(m_pf);
    unsigned char * pStartPos = (unsigned char *) 
        (m_Extent.tl.y*width*bpp + m_Extent.tl.x*bpp);
    if (MemoryMode == OGL) {
        pStartPos += (unsigned int)(pBmp->getPixels());
    }
    {
        ScopeTimer Timer(TexSubImageProfilingZone);
        glTexSubImage2D(TextureMode, 0, 0, 0, m_Extent.Width(), m_Extent.Height(),
                m_pEngine->getOGLSrcMode(m_pf), m_pEngine->getOGLPixelType(m_pf), 
                pStartPos);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::downloadTextures: glTexSubImage2D()");

}

void OGLTile::blt(const DPoint& TLPoint, const DPoint& TRPoint,
        const DPoint& BLPoint, const DPoint& BRPoint) const
{
    double TexWidth;
    double TexHeight;
    int TextureMode = m_pEngine->getTextureMode();

    if (TextureMode == GL_TEXTURE_2D) {
        TexWidth = double(m_Extent.Width())/m_TexSize.x;
        TexHeight = double(m_Extent.Height())/m_TexSize.y;
    } else {
        TexWidth = m_TexSize.x;
        TexHeight = m_TexSize.y;
    }
    
    glBindTexture(TextureMode, m_TexID[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "GOGLSurface::bltTile: glBindTexture()");
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3d (TLPoint.x, TLPoint.y, 0.0);
    glTexCoord2d(TexWidth, 0.0);
    glVertex3d (TRPoint.x, TRPoint.y, 0.0);
    glTexCoord2d(TexWidth, TexHeight);
    glVertex3d (BRPoint.x, BRPoint.y, 0.0);
    glTexCoord2d(0.0, TexHeight);
    glVertex3d (BLPoint.x, BLPoint.y, 0.0);
    glEnd();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLSurface::bltTile: glEnd()");
}

}
