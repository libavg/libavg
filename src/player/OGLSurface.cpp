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
#include "OGLHelper.h"
#include "Player.h"
#include "MathHelper.h"

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
      m_bBound(false),
      m_Size(-1,-1),
      m_MaxTileSize(-1,-1),
      m_NumHorizTextures(-1),
      m_NumVertTextures(-1)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    // Do an NVIDIA texture support query if it hasn't happened already.
//    getTextureMode();
}

OGLSurface::~OGLSurface()
{
    unbind();
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
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLSurface::create(const IntPoint& Size, PixelFormat pf, bool bFastDownload)
{
//    cerr << "create: " << Size << ", " << Bitmap::getPixelFormatString(pf) << endl;
    
    if (m_bBound && m_Size == Size && m_pf == pf) {
        // If nothing's changed, we can ignore everything.
        return;
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
        
    unbind();
    setupTiles();
    initTileVertices(m_TileVertices);
}

void OGLSurface::createFromBits(IntPoint Size, PixelFormat pf,
        unsigned char * pBits, int Stride)
{
    unbind();
    m_MemoryMode = OGL;
    m_Size = Size;
    m_pf = pf;
    m_pBmps[0] = BitmapPtr(new Bitmap(Size, pf, pBits, Stride, false, ""));
    
    setupTiles();
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
        setupTiles();
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
    if (Grid.size() != (unsigned)(m_NumVertTextures+1)) {
        bGridOK = false;
    }
    for (unsigned i = 0; i< Grid.size(); ++i) {
        if (Grid[i].size() != (unsigned)(m_NumHorizTextures+1)) {
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
        case GL_YCBCR_MESA:
            return "GL_YCBCR_MESA";
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
        m_pTiles.clear();
        vector<OGLTilePtr> v;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
        
        for (int y=0; y<m_NumVertTextures; y++) {
            m_pTiles.push_back(v);
            for (int x=0; x<m_NumHorizTextures; x++) {
                IntPoint CurSize = m_TileSize;
                if (y == m_NumVertTextures-1) {
                    CurSize.y = Height-y*m_TileSize.y;
                }
                if (x == m_NumHorizTextures-1) {
                    CurSize.x = Width-x*m_TileSize.x;
                }
                Rect<int> CurExtent(x*m_TileSize.x, y*m_TileSize.y,
                        x*m_TileSize.x+CurSize.x, y*m_TileSize.y+CurSize.y);
                if (m_pEngine->getTextureMode() == GL_TEXTURE_2D) {
                    CurSize.x = nextpow2(CurSize.x);
                    CurSize.y = nextpow2(CurSize.y);
                }
                
                OGLTilePtr pTile = OGLTilePtr(new OGLTile(CurExtent, CurSize,
                        m_Size.x, m_pf, m_pEngine));
                m_pTiles[y].push_back(pTile);
                if (m_MemoryMode == PBO) {
                    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
                        for (int i=0; i<3; i++) {
                            glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 
                                    m_hPixelBuffers[i]);
                            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                                    "OGLSurface::bind: glBindBuffer()");
                            pTile->downloadTexture(i, m_pBmps[i], m_Size.x, m_MemoryMode);
                        }
                    } else {
                        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[0]);
                        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                                "OGLSurface::bind: glBindBuffer()");
                        pTile->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
                    }
                    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
                } else {
                    pTile->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
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
        m_pTiles.clear();
    }
    m_bBound = false;
}

void OGLSurface::rebind()
{
//    cerr << "OGLSurface::rebind()" << endl;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    for (unsigned int y=0; y<m_pTiles.size(); y++) {
        for (unsigned int x=0; x<m_pTiles[y].size(); x++) {
            OGLTilePtr pTile = m_pTiles[y][x];
            if (m_MemoryMode == PBO) {
                if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
                    for (int i=0; i<3; i++) {
                        glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[i]);
                        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                                "OGLSurface::rebind: glBindBuffer()");
                        pTile->downloadTexture(i, m_pBmps[i], m_Size.x, m_MemoryMode);
                    }
                } else {
                    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffers[0]);
                    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                            "OGLSurface::rebind: glBindBuffer()");
                    pTile->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
                }
                glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            } else {
                pTile->downloadTexture(0, m_pBmps[0], m_Size.x, m_MemoryMode);
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

void OGLSurface::blt(const DRect* pDestRect, 
        double angle, const DPoint& pivot, 
        DisplayEngine::BlendMode Mode)
{
//    cerr << "OGLSurface::blt()" << endl;
    if (!m_bBound) {
        bind();
    }
    bltTexture(pDestRect, angle, pivot, Mode);
}

bool OGLSurface::wouldTile(IntPoint Size)
{
    if (m_MaxTileSize.x != -1 || m_MaxTileSize.y != -1 ||
        Size.x > m_pEngine->getMaxTexSize() || 
        Size.y > m_pEngine->getMaxTexSize() ||
        m_pEngine->getTextureMode() == GL_TEXTURE_2D)
    {
        return true;
    } else {
        return false;
    }
}

void OGLSurface::setupTiles()
{
    if (m_Size.x > m_pEngine->getMaxTexSize() || 
        m_Size.y > m_pEngine->getMaxTexSize()) 
    {
        m_TileSize = IntPoint(m_pEngine->getMaxTexSize(), m_pEngine->getMaxTexSize());
    } else {
        if (m_pEngine->getTextureMode() == GL_TEXTURE_2D) {
            if ((m_Size.x > 256 && nextpow2(m_Size.x) > m_Size.x*1.3) ||
                    (m_Size.y > 256 && nextpow2(m_Size.y) > m_Size.y*1.3)) 
            {
                m_TileSize = IntPoint(nextpow2(m_Size.x)/2, nextpow2(m_Size.y)/2);
            } else {
                m_TileSize = IntPoint(nextpow2(m_Size.x), nextpow2(m_Size.y));
            }
        } else {
            m_TileSize = m_Size;
        }
    }
    if (m_MaxTileSize.x != -1 && m_MaxTileSize.x < m_TileSize.x) {
        m_TileSize.x = m_MaxTileSize.x;
    }
    if (m_MaxTileSize.y != -1 && m_MaxTileSize.y < m_TileSize.y) {
        m_TileSize.y = m_MaxTileSize.y;
    }
    m_NumHorizTextures = int(ceil(float(m_Size.x)/m_TileSize.x));
    m_NumVertTextures = int(ceil(float(m_Size.y)/m_TileSize.y));

}

void OGLSurface::initTileVertices(VertexGrid& Grid)
{
    std::vector<DPoint> TileVerticesLine(m_NumHorizTextures+1);
    Grid = std::vector<std::vector<DPoint> >
                (m_NumVertTextures+1, TileVerticesLine);
    for (unsigned int y=0; y<Grid.size(); y++) {
        for (unsigned int x=0; x<Grid[y].size(); x++) {
            initTileVertex(x, y, Grid[y][x]);
        }
    }
}

void OGLSurface::initTileVertex (int x, int y, DPoint& Vertex) 
{
    if (x < m_NumHorizTextures) {
        Vertex.x = double(m_TileSize.x*x) / m_Size.x;
    } else {
        Vertex.x = 1;
    }
    if (y < m_NumVertTextures) {
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

void OGLSurface::bltTexture(const DRect* pDestRect, 
                double angle, const DPoint& pivot, 
                DisplayEngine::BlendMode Mode)
{
    if (fabs(angle) > 0.001) {
        DPoint center(pDestRect->tl.x+pivot.x,
                pDestRect->tl.y+pivot.y);

        glPushMatrix();
        glTranslated(center.x, center.y, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "bltTexture: glTranslated");
        glRotated(angle*180.0/PI, 0, 0, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "bltTexture: glRotated");
        glTranslated(-center.x, -center.y, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "bltTexture: glTranslated");
    }

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

    for (unsigned int y=0; y<m_pTiles.size(); y++) {
        for (unsigned int x=0; x<m_pTiles[y].size(); x++) {
            DPoint TLPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y][x]);
            DPoint TRPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y][x+1]);
            DPoint BLPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y+1][x]);
            DPoint BRPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y+1][x+1]);
            OGLTilePtr pCurTile = m_pTiles[y][x];
            pCurTile->blt(TLPoint, TRPoint, BLPoint, BRPoint); 
        }
    }

    AVG_TRACE(Logger::BLTS, "(" << pDestRect->tl.x << ", " 
            << pDestRect->tl.y << ")" << ", width:" << pDestRect->Width() 
            << ", height: " << pDestRect->Height() << ", m_pf: " 
            << Bitmap::getPixelFormatString(m_pf) << ", " 
            << getGlModeString(m_pEngine->getOGLSrcMode(m_pf)) << "-->" 
            << getGlModeString(m_pEngine->getOGLDestMode(m_pf)) << endl);
    if (fabs(angle) > 0.001) {
        glPopMatrix();
    }
}

DPoint OGLSurface::calcFinalVertex(const DRect* pDestRect,
        const DPoint & NormalizedVertex)
{
    DPoint Point;
    Point.x = pDestRect->tl.x+
        (pDestRect->Width()*NormalizedVertex.x);
    Point.y = pDestRect->tl.y+
        (pDestRect->Height()*NormalizedVertex.y);
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

}
