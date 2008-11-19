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
    
OGLTexture::OGLTexture(IntRect TexExtent, IntPoint TexSize, IntPoint TileSize,
        IntRect TileIndexExtent, int Stride, PixelFormat pf, 
        SDLDisplayEngine * pEngine) 
    : m_TexExtent(TexExtent),
      m_TexSize(TexSize),
      m_TileSize(TileSize),
      m_TileIndexExtent(TileIndexExtent),
      m_pf(pf),
      m_pEngine(pEngine)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    createTextures(Stride);
    int numTiles = m_TileIndexExtent.width()*m_TileIndexExtent.height();
    m_pVertexes = new VertexArray(numTiles*4, numTiles*6);
    calcTexCoords();
}

OGLTexture::~OGLTexture()
{
    delete m_pVertexes;
    deleteTextures();
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLTexture::resize(IntRect TexExtent, IntPoint TexSize, IntPoint TileSize, 
        int Stride)
{
    deleteTextures();
    m_TexExtent = TexExtent;
    m_TexSize = TexSize;
    m_TileSize = TileSize;
    createTextures(Stride);
    calcTexCoords();
}

int OGLTexture::getTexID(int i) const
{
    return m_TexID[i];
}

void OGLTexture::blt(const VertexGrid* pVertexes) const
{
    int TextureMode = m_pEngine->getTextureMode();
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
        glBindTexture(TextureMode, m_TexID[0]);
        pShader->setUniformIntParam("YTexture", 0);
        glproc::ActiveTexture(GL_TEXTURE1);
        glBindTexture(TextureMode, m_TexID[1]);
        pShader->setUniformIntParam("CbTexture", 1);
        glproc::ActiveTexture(GL_TEXTURE2);
        glBindTexture(TextureMode, m_TexID[2]);
        pShader->setUniformIntParam("CrTexture", 2);
    } else {
        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(TextureMode, m_TexID[0]);
        if (m_pEngine->getYCbCrMode() == OGL_SHADER) {
            glproc::UseProgramObject(0);
        }
    }
    if (pVertexes) {
        int curVertex=0;
        int curIndex=0;
        for (int y=m_TileIndexExtent.tl.y; y<m_TileIndexExtent.br.y; y++) {
            for (int x=m_TileIndexExtent.tl.x; x<m_TileIndexExtent.br.x; x++) {
                int xoffset = x-m_TileIndexExtent.tl.x;
                int yoffset = y-m_TileIndexExtent.tl.y;
                m_pVertexes->setPos(curVertex, (*pVertexes)[y][x], 
                        m_TexCoords[yoffset][xoffset]); 
                m_pVertexes->setPos(curVertex+1, (*pVertexes)[y][x+1], 
                        m_TexCoords[yoffset][xoffset+1]); 
                m_pVertexes->setPos(curVertex+2, (*pVertexes)[y+1][x+1],
                        m_TexCoords[yoffset+1][xoffset+1]); 
                m_pVertexes->setPos(curVertex+3, (*pVertexes)[y+1][x],
                        m_TexCoords[yoffset+1][xoffset]); 
                m_pVertexes->setTriIndexes(curIndex, curVertex, curVertex+1, curVertex+2);
                m_pVertexes->setTriIndexes(curIndex+3, curVertex, curVertex+2, curVertex+3);
                curVertex+=4;
                curIndex+=6;
            }
        }
    }

    m_pVertexes->draw();
    
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        glproc::ActiveTexture(GL_TEXTURE1);
        glDisable(TextureMode);
        glproc::ActiveTexture(GL_TEXTURE2);
        glDisable(TextureMode);
        glproc::ActiveTexture(GL_TEXTURE0);
        glproc::UseProgramObject(0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::blt: glDisable(TextureMode)");
    }
}

void OGLTexture::createTextures(int Stride)
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        createTexture(0, m_TexSize, Stride, I8);
        createTexture(1, m_TexSize/2, Stride/2, I8);
        createTexture(2, m_TexSize/2, Stride/2, I8);
    } else {
        createTexture(0, m_TexSize, Stride, m_pf);
    }
}

void OGLTexture::createTexture(int i, IntPoint Size, int Stride, PixelFormat pf)
{
    glGenTextures(1, &m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::createTexture: glGenTextures()");
    glproc::ActiveTexture(GL_TEXTURE0+i);
    int TextureMode = m_pEngine->getTextureMode();
    glBindTexture(TextureMode, m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTexture::createTexture: glBindTexture()");
    glTexParameteri(TextureMode, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(TextureMode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TextureMode, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(TextureMode, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::createTexture: glTexParameteri()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, Size.x);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::createTexture: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    
    GLenum DestMode = m_pEngine->getOGLDestMode(pf);
#if defined(__APPLE__) && !defined(__i386__)
    // XXX: Hack to work around broken Mac OS X GL_ALPHA/GL_UNPACK_ROW_LENGTH on 
    // PPC macs. If this is gone, the Stride parameter can be removed too :-).
    if (Stride != Size.x && DestMode == GL_ALPHA &&
            (m_pf == YCbCr420p || m_pf == YCbCrJ420p)) 
    {
        DestMode = GL_RGBA;
    }
#endif
    char * pPixels = 0;
    if (TextureMode == GL_TEXTURE_2D) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        int TexMemNeeded = Size.x*Size.y*Bitmap::getBytesPerPixel(pf);
        pPixels = new char[TexMemNeeded];
        memset(pPixels, 0, TexMemNeeded);
    }
    glTexImage2D(TextureMode, 0, DestMode, Size.x, Size.y, 0,
            m_pEngine->getOGLSrcMode(pf), m_pEngine->getOGLPixelType(pf), pPixels);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::createTexture: glTexImage2D()");
    if (TextureMode == GL_TEXTURE_2D) {
        free(pPixels);
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

void OGLTexture::downloadTexture(int i, BitmapPtr pBmp, int stride, 
                OGLMemoryMode MemoryMode) const
{
    PixelFormat pf;
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        pf = I8;
    } else {
        pf = m_pf;
    }
    IntRect Extent = m_TexExtent;
    if (i != 0) {
        stride /= 2;
        Extent = IntRect(m_TexExtent.tl/2.0, m_TexExtent.br/2.0);
    }
    int TextureMode = m_pEngine->getTextureMode();
    glBindTexture(TextureMode, m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::downloadTexture: glBindTexture()");
    int bpp = Bitmap::getBytesPerPixel(pf);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::downloadTexture: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
//    cerr << "OGLTexture::downloadTexture(" << pBmp << ", stride=" << stride 
//        << ", Extent= " << m_TexExtent << ", pf= " << Bitmap::getPixelFormatString(m_pf)
//        << ", bpp= " << bpp << endl;
    unsigned char * pStartPos = (unsigned char *)
            (ptrdiff_t)(Extent.tl.y*stride*bpp + Extent.tl.x*bpp);
    if (MemoryMode == OGL) {
        pStartPos += (ptrdiff_t)(pBmp->getPixels());
    }
#ifdef __APPLE__
    // Under Mac OS X 10.5.0 and 10.5.1, the combination of glTexSubImage2D, 
    // GL_ALPHA and PBO is broken if pStartPos is 0. So we use an offset. 
    // There's corresponding code in OGLSurface that undoes this... bleagh.
    if (MemoryMode == PBO && 
            (m_pf == I8 || m_pf == YCbCr420p || m_pf == YCbCrJ420p || m_pf == YCbCr422)) 
    {
        pStartPos += 4;
    }
#endif
    {
        ScopeTimer Timer(TexSubImageProfilingZone);
        glTexSubImage2D(TextureMode, 0, 0, 0, Extent.width(), Extent.height(),
                m_pEngine->getOGLSrcMode(pf), m_pEngine->getOGLPixelType(pf), 
                pStartPos);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTexture::downloadTexture: glTexSubImage2D()");
}

void OGLTexture::calcTexCoords()
{
    double TexWidth;
    double TexHeight;
    double TexWidthPerTile;
    double TexHeightPerTile;
    int TextureMode = m_pEngine->getTextureMode();
/*   
    cerr << "----calcTexCoords" << endl;
    cerr << "m_TexExtent: " << m_TexExtent << endl;
    cerr << "m_TileSize: " << m_TileSize << endl;
    cerr << "m_TexSize: " << m_TexSize << endl;
*/    
    if (TextureMode == GL_TEXTURE_2D) {
        TexWidth = double(m_TexExtent.width())/m_TexSize.x;
        TexHeight = double(m_TexExtent.height())/m_TexSize.y;
        TexWidthPerTile=double(m_TileSize.x)/m_TexSize.x;
        TexHeightPerTile=double(m_TileSize.y)/m_TexSize.y;
    } else {
        TexWidth = m_TexSize.x;
        TexHeight = m_TexSize.y;
        TexWidthPerTile=m_TileSize.x;
        TexHeightPerTile=m_TileSize.y;
    }
/*
    cerr << "TexSize: " << TexWidth << ", " << TexHeight << endl;
    cerr << "TexTileSize: " << TexWidthPerTile << ", " << TexHeightPerTile << endl;
*/
    vector<DPoint> TexCoordLine(m_TileIndexExtent.width()+1);
    m_TexCoords = std::vector<std::vector<DPoint> > 
            (m_TileIndexExtent.height()+1, TexCoordLine);
    for (unsigned int y=0; y<m_TexCoords.size(); y++) {
        for (unsigned int x=0; x<m_TexCoords[y].size(); x++) {
            if (y == m_TexCoords.size()-1) {
                m_TexCoords[y][x].y = TexHeight;
            } else {
                m_TexCoords[y][x].y = TexHeightPerTile*y;
            }
            if (x == m_TexCoords[y].size()-1) {
                m_TexCoords[y][x].x = TexWidth;
            } else {
                m_TexCoords[y][x].x = TexWidthPerTile*x;
            }
        }
    }
//    cerr << endl;
}

const IntRect& OGLTexture::getTileIndexExtent() const
{
    return m_TileIndexExtent;
}

const int OGLTexture::getTexMemDim()
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        return int(m_TexSize.x * m_TexSize.y * 1.5);
    }
    else {
        return m_TexSize.x * m_TexSize.y * Bitmap::getBytesPerPixel(m_pf);
    }
}

const PixelFormat OGLTexture::getPixelFormat() const
{
    return m_pf;
}

}
