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

#include "OGLSurface.h"
#include "Player.h"
#include "SDLDisplayEngine.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/Point.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

OGLSurface::OGLSurface(SDLDisplayEngine * pEngine)
    : m_pEngine(pEngine),
      m_bCreated(false),
      m_bBound(false),
      m_Size(-1,-1),
      m_NumTextures(-1, -1),
      m_MaxTileSize(-1,-1)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OGLSurface::~OGLSurface()
{
    if (m_bCreated) {
        unbind();
        deleteBuffers();
    }
    m_pEngine->deregisterSurface(this);
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLSurface::create(const IntPoint& Size, PixelFormat pf, bool bFastDownload)
{
//    cerr << "create: " << Size << ", " << Bitmap::getPixelFormatString(pf) << endl;
    if (m_bBound && m_Size == Size && m_pf == pf) {
        // If nothing's changed, we can ignore everything.
        return;
    }
    if (m_bCreated) {
        unbind();
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
        
    calcTileSizes();
    initTileVertices(m_TileVertices);
    m_bCreated = true;
}

void OGLSurface::createFromBits(IntPoint Size, PixelFormat pf,
        unsigned char * pBits, int Stride)
{
    unbind();
    m_MemoryMode = OGL;
    m_Size = Size;
    m_pf = pf;
    m_pBmps[0] = BitmapPtr(new Bitmap(Size, pf, pBits, Stride, false, ""));
    
    calcTileSizes();
}

BitmapPtr OGLSurface::lockBmp(int i)
{
//    cerr << "lockBmp " << i << endl;
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
#ifdef __APPLE__
                if ( m_pf == I8 || m_pf == YCbCr420p || m_pf == YCbCrJ420p || 
                        m_pf == YCbCr422) 
                {
                    // Under Mac OS X 10.5.0 and 10.5.1, the combination of 
                    // glTexSubImage2D, GL_ALPHA and PBO is broken if pStartPos is
                    // 0. So we use an offset. There's corresponding code in
                    // OGLTile that undoes this... bleagh.
                    pBuffer += 4;
                }
#endif

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

void OGLSurface::setMaxTileSize(const IntPoint& MaxTileSize)
{
    if (m_bBound) {
        unbind();
    }
    m_MaxTileSize = MaxTileSize;
    if (m_MaxTileSize.x != -1) {
        m_MaxTileSize.x = nextpow2(m_MaxTileSize.x/2+1);
    }
    if (m_MaxTileSize.y != -1) {
        m_MaxTileSize.y = nextpow2(m_MaxTileSize.y/2+1);
    }
    if (m_pBmps[0]) {
        calcTileSizes();
        initTileVertices(m_TileVertices);
    }
}

VertexGrid OGLSurface::getOrigVertexCoords()
{
    if (!m_bBound) {
        bind();
    }
    VertexGrid Grid;
    initTileVertices(Grid);
    return Grid;
}

VertexGrid OGLSurface::getWarpedVertexCoords()
{
    if (!m_bBound) {
        bind();
    }
    return m_TileVertices;
}

void OGLSurface::setWarpedVertexCoords(const VertexGrid& Grid)
{
    if (!m_bBound) {
        bind();
    }
    bool bGridOK = true;
    if (Grid.size() != (unsigned)(m_NumTiles.y+1)) {
        bGridOK = false;
    }
    for (unsigned i = 0; i< Grid.size(); ++i) {
        if (Grid[i].size() != (unsigned)(m_NumTiles.x+1)) {
            bGridOK = false;
        }
    }
    if (!bGridOK) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "setWarpedVertexCoords() called with incorrect grid size.");
    }
    m_TileVertices = Grid;
}

string getGlModeString(int Mode) 
{
    switch (Mode) {
        case GL_ALPHA:
            return "GL_ALPHA";
        case GL_RGB:
            return "GL_RGB";
        case GL_RGBA:
            return "GL_RGBA";
        case GL_BGR:
            return "GL_BGR";
        case GL_BGRA:
            return "GL_BGRA";
        case GL_YCBCR_422_APPLE:
            return "GL_YCBCR_422_APPLE";
        default:
            return "UNKNOWN";
    }
}

void OGLSurface::bind() 
{
    if (m_bBound) {
        rebind();
    } else {
        int Width = m_Size.x;
        int Height = m_Size.y;
        m_pTextures.clear();
        vector<OGLTexturePtr> v;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
        
        for (int y=0; y<m_NumTextures.y; y++) {
            m_pTextures.push_back(v);
            for (int x=0; x<m_NumTextures.x; x++) {
                IntPoint CurSize = m_TextureSize;
                if (y == m_NumTextures.y-1) {
                    CurSize.y = Height-y*m_TextureSize.y;
                }
                if (x == m_NumTextures.x-1) {
                    CurSize.x = Width-x*m_TextureSize.x;
                }
                Rect<int> CurExtent(x*m_TextureSize.x, y*m_TextureSize.y,
                        x*m_TextureSize.x+CurSize.x, y*m_TextureSize.y+CurSize.y);
                IntRect TileIndexExtents(
                        safeCeil(double(CurExtent.tl.x)/m_TileSize.x), 
                        safeCeil(double(CurExtent.tl.y)/m_TileSize.y),
                        safeCeil(double(CurExtent.br.x)/m_TileSize.x), 
                        safeCeil(double(CurExtent.br.y)/m_TileSize.y));
                if (m_pEngine->getTextureMode() == GL_TEXTURE_2D) {
                    CurSize.x = nextpow2(CurSize.x);
                    CurSize.y = nextpow2(CurSize.y);
                }
/*
                cerr << "Texture: " << x << ", " << y << endl;
                cerr << "CurExtent: " << CurExtent << endl;
                cerr << "m_TileSize: " << m_TileSize << endl;
                cerr << "TileIndexExtents: " << TileIndexExtents << endl;
*/                
                OGLTexturePtr pTexture = OGLTexturePtr(new OGLTexture(CurExtent, 
                        CurSize, m_TileSize, TileIndexExtents, m_Size.x, m_pf,
                        m_pEngine));
                m_pTextures[y].push_back(pTexture);
                if (m_MemoryMode == PBO) {
                    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
                        for (int i=0; i<3; i++) {
                            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 
                                    m_hPixelBuffers[i]);
                            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                                    "OGLSurface::bind: glBindBuffer()");
                            pTexture->downloadTexture(i, m_pBmps[i], m_Size.x, m_MemoryMode);
                        }
                    } else {
                        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[0]);
                        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                                "OGLSurface::bind: glBindBuffer()");
                        pTexture->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
                    }
                    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
                } else {
                    pTexture->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
                }
            }
        }
        if (m_MemoryMode == PBO) {
            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::bind: glBindBuffer(0)");
        }
        m_bBound = true;
    }
}

void OGLSurface::unbind() 
{
//    cerr << "OGLSurface::unbind()" << endl;
    if (m_bBound) {
        m_pTextures.clear();
    }
    m_bBound = false;
}

void OGLSurface::rebind()
{
//    cerr << "OGLSurface::rebind()" << endl;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    for (unsigned int y=0; y<m_pTextures.size(); y++) {
        for (unsigned int x=0; x<m_pTextures[y].size(); x++) {
            OGLTexturePtr pTexture = m_pTextures[y][x];
            if (m_MemoryMode == PBO) {
                if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
                    for (int i=0; i<3; i++) {
                        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[i]);
                        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                                "OGLSurface::rebind: glBindBuffer()");
                        pTexture->downloadTexture(i, m_pBmps[i], m_Size.x, m_MemoryMode);
                    }
                } else {
                    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[0]);
                    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                            "OGLSurface::rebind: glBindBuffer()");
                    pTexture->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
                }
                glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            } else {
                pTexture->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
            }
        }
    }
    if (m_MemoryMode == PBO) {
        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::rebind: glBindBuffer(0)");
    }
}

PixelFormat OGLSurface::getPixelFormat()
{
    return m_pf;
}
        
IntPoint OGLSurface::getSize()
{
    return m_Size;
}

void OGLSurface::blt(const DPoint& DestSize, 
        DisplayEngine::BlendMode Mode)
{
//    cerr << "OGLSurface::blt()" << endl;
    if (!m_bBound) {
        bind();
    }
    bltTexture(DestSize, Mode);
}

bool OGLSurface::isOneTexture(IntPoint Size)
{
    if (Size.x > m_pEngine->getMaxTexSize() || 
        Size.y > m_pEngine->getMaxTexSize() ||
        m_pEngine->getTextureMode() == GL_TEXTURE_2D)
    {
        return false;
    } else {
        return true;
    }
}

void OGLSurface::calcTileSizes()
{
    if (m_pEngine->getTextureMode() == GL_TEXTURE_2D) {
        if ((m_Size.x > 256 && nextpow2(m_Size.x) > m_Size.x*1.3) ||
                (m_Size.y > 256 && nextpow2(m_Size.y) > m_Size.y*1.3)) 
        {
            m_TextureSize = IntPoint(nextpow2(m_Size.x)/2, nextpow2(m_Size.y)/2);
        } else {
            m_TextureSize = IntPoint(nextpow2(m_Size.x), nextpow2(m_Size.y));
        }
    } else {
        m_TextureSize = m_Size;
    }
    if (m_Size.x > m_pEngine->getMaxTexSize()) {
        m_TextureSize.x = m_pEngine->getMaxTexSize();
    }
    if (m_Size.y > m_pEngine->getMaxTexSize()) {
        m_TextureSize.y = m_pEngine->getMaxTexSize();
    }
    m_NumTextures.x = safeCeil(double(m_Size.x)/m_TextureSize.x);
    m_NumTextures.y = safeCeil(double(m_Size.y)/m_TextureSize.y);

    m_TileSize = m_TextureSize;
    if (m_MaxTileSize.x != -1 && m_MaxTileSize.x < m_TextureSize.x) {
        m_TileSize.x = m_MaxTileSize.x;
    }
    if (m_MaxTileSize.y != -1 && m_MaxTileSize.y < m_TextureSize.y) {
        m_TileSize.y = m_MaxTileSize.y;
    }
    m_NumTiles.x = safeCeil(double(m_Size.x)/m_TileSize.x);
    m_NumTiles.y = safeCeil(double(m_Size.y)/m_TileSize.y);
/*    
    cerr << "----calcTileSizes: " << endl;
    cerr << "TextureSize: " << m_TextureSize << ", NumTextures: " << m_NumTextures << endl;
    cerr << "TileSize: " << m_TileSize << ", NumTiles: " << m_NumTiles << endl;
*/    
}

void OGLSurface::initTileVertices(VertexGrid& Grid)
{
    std::vector<DPoint> TileVerticesLine(m_NumTiles.x+1);
    Grid = std::vector<std::vector<DPoint> >
                (m_NumTiles.y+1, TileVerticesLine);
    for (unsigned int y=0; y<Grid.size(); y++) {
        for (unsigned int x=0; x<Grid[y].size(); x++) {
            initTileVertex(x, y, Grid[y][x]);
        }
    }
}

void OGLSurface::initTileVertex (int x, int y, DPoint& Vertex) 
{
    if (x < m_NumTiles.x) {
        Vertex.x = double(m_TileSize.x*x) / m_Size.x;
    } else {
        Vertex.x = 1;
    }
    if (y < m_NumTiles.y) {
        Vertex.y = double(m_TileSize.y*y) / m_Size.y;
    } else {
        Vertex.y = 1;
    }
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
//                    GL_STREAM_DRAW);
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

void OGLSurface::bltTexture(const DPoint& DestSize, 
        DisplayEngine::BlendMode Mode)
{
    switch(Mode) {
        case DisplayEngine::BLEND_BLEND:
            glproc::BlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("blend");
            break;
        case DisplayEngine::BLEND_ADD:
            glproc::BlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            checkBlendModeError("add");
            break;
        case DisplayEngine::BLEND_MIN:
            glproc::BlendEquation(GL_MIN);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("min");
            break;
        case DisplayEngine::BLEND_MAX:
            glproc::BlendEquation(GL_MAX);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("max");
            break;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    std::vector<DPoint> VertexesLine(m_NumTiles.x+1);
    VertexGrid FinalVertexes = std::vector<std::vector<DPoint> >
                (m_NumTiles.y+1, VertexesLine);
    for (unsigned int y=0; y<FinalVertexes.size(); y++) {
        for (unsigned int x=0; x<FinalVertexes[y].size(); x++) {
            FinalVertexes[y][x] = calcFinalVertex(DestSize, m_TileVertices[y][x]);
        }
    }

    for (unsigned int y=0; y<m_pTextures.size(); y++) {
        for (unsigned int x=0; x<m_pTextures[y].size(); x++) {
            m_pTextures[y][x]->blt(&FinalVertexes); 
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY); 
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    AVG_TRACE(Logger::BLTS, "(" << DestSize.x << ", " 
            << DestSize.y << ")" << ", m_pf: " 
            << Bitmap::getPixelFormatString(m_pf) << ", " 
            << getGlModeString(m_pEngine->getOGLSrcMode(m_pf)) << "-->" 
            << getGlModeString(m_pEngine->getOGLDestMode(m_pf)));
}

DPoint OGLSurface::calcFinalVertex(const DPoint& Size,
        const DPoint & NormalizedVertex)
{
    DPoint Point;
    Point.x = Size.x*NormalizedVertex.x;
    Point.y = Size.y*NormalizedVertex.y;
    return Point;
}

void OGLSurface::checkBlendModeError(string sMode) 
{    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        static bool bErrorReported = false;
        if (!bErrorReported) {
            AVG_TRACE(Logger::WARNING, "Blendmode "+sMode+
                    " not supported by OpenGL implementation.");
            bErrorReported = true;
        }
    }
}

int OGLSurface::getTotalTexMemory()
{
    int iAmount = 0;
    if (m_bBound) {
        for (int y=0; y<m_NumTextures.y; y++) {
            for (int x=0; x<m_NumTextures.x; x++) {
                iAmount += m_pTextures[y][x]->getTexMemDim();
            }
        }
    }
    return iAmount;
}

}
