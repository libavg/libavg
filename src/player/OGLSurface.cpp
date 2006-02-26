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
#include "MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../graphics/Point.h"

#include "GL/glu.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

PFNGLGENBUFFERSPROC OGLSurface::s_GenBuffersProc = 0;
PFNGLBUFFERDATAPROC OGLSurface::s_BufferDataProc = 0;
PFNGLDELETEBUFFERSPROC OGLSurface::s_DeleteBuffersProc = 0;
PFNGLBINDBUFFERPROC OGLSurface::s_BindBufferProc = 0;
PFNGLMAPBUFFERPROC OGLSurface::s_MapBufferProc = 0;
PFNGLUNMAPBUFFERPROC OGLSurface::s_UnmapBufferProc = 0;

#ifndef __APPLE__
PFNGLXALLOCATEMEMORYMESAPROC OGLSurface::s_AllocMemMESAProc = 0;
PFNGLXFREEMEMORYMESAPROC OGLSurface::s_FreeMemMESAProc = 0;
#endif

OGLSurface::OGLSurface(SDLDisplayEngine * pEngine)
    : m_pEngine(pEngine),
      m_bBound(false),
      m_MaxTileSize(-1,-1)
{
    // Do an NVIDIA texture support query if it hasn't happened already.
//    getTextureMode();
}

OGLSurface::~OGLSurface()
{
    unbind();
    switch(m_MemoryMode) {
        case PBO:
            s_DeleteBuffersProc(1, &m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::~OGLSurface: glDeleteBuffers()");
            break;
        default:
            break;
    }
}

void OGLSurface::create(const IntPoint& Size, PixelFormat pf, bool bFastDownload)
{
    if (m_bBound && m_Size == Size && m_pf == pf) {
        // If nothing's changed, we can ignore everything.
        return;
    }
    m_Size = Size;
    m_pf = pf;
    m_MemoryMode = OGL;
    if (bFastDownload) {
        m_MemoryMode = getMemoryModeSupported();
    }
    switch (m_MemoryMode) {
        case PBO:
            s_GenBuffersProc(1, &m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::create: glGenBuffers()");
            s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::create: glBindBuffer()");
            s_BufferDataProc(GL_PIXEL_UNPACK_BUFFER_EXT, 
                    (Size.x+1)*(Size.y+1)*Bitmap::getBytesPerPixel(pf), NULL, 
                    GL_DYNAMIC_DRAW);
//                    GL_STREAM_DRAW);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::create: glBufferData()");
            s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::create: glBindBuffer(0)");
            m_pBmp = BitmapPtr();
            break;
#ifndef __APPLE__
        case MESA:
            {
                Display * display = XOpenDisplay(0);
                int stride = m_Size.x*Bitmap::getBytesPerPixel(m_pf);
                m_pMESABuffer = s_AllocMemMESAProc(display, DefaultScreen(display),
                        stride*(Size.y+1), 0, 1.0 ,0);
                if (!m_pMESABuffer) {
                    AVG_TRACE(Logger::WARNING, "Failed to allocate MESA memory");
                    m_MemoryMode = OGL;
                }
                m_pBmp = BitmapPtr();
            }
            break;
#endif
        default:
            break;
    }
    if (m_MemoryMode == OGL) {
        // Can't do this in the switch because memory allocation might fail.
        // In that case, this is needed as a fallback.
        m_pBmp = BitmapPtr(new Bitmap(Size, pf));
    }
        
    unbind();
    setupTiles();
    initTileVertices();
}

void OGLSurface::createFromBits(IntPoint Size, PixelFormat pf,
        unsigned char * pBits, int Stride)
{
    unbind();
    m_MemoryMode = OGL;
    m_Size = Size;
    m_pf = pf;
    m_pBmp = BitmapPtr(new Bitmap(Size, pf, pBits, Stride, false, ""));
    
    setupTiles();
}

BitmapPtr OGLSurface::lockBmp()
{
    switch (m_MemoryMode) {
        case PBO:
            {
                s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glBindBuffer()");
                unsigned char * pBuffer = (unsigned char *)
                    s_MapBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glMapBuffer()");
                s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glBindBuffer(0)");
                m_pBmp = BitmapPtr(new Bitmap(m_Size, m_pf, pBuffer, 
                            m_Size.x*Bitmap::getBytesPerPixel(m_pf), false));
            }
            break;
#ifndef __APPLE__
        case MESA:
            {
                int stride = m_Size.x*Bitmap::getBytesPerPixel(m_pf);
                m_pBmp = BitmapPtr(new Bitmap(m_Size, m_pf, 
                        (unsigned char *)m_pMESABuffer, stride, false));
            }
#endif
        default:
            break;
    }
    return m_pBmp;
}

void OGLSurface::unlockBmp()
{
    m_pf = m_pBmp->getPixelFormat();
    switch (m_MemoryMode) {
        case PBO:
            s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::unlockBmp: glBindBuffer()");
            s_UnmapBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::unlockBmp: glUnmapBuffer()");
            s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::lockBmp: glBindBuffer(0)");
            m_pBmp = BitmapPtr();
            break;
        default:
            break;
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
    if (m_pBmp) {
        setupTiles();
        initTileVertices();
    }
}

int OGLSurface::getNumVerticesX()
{
    return m_NumHorizTextures+1;
}

int OGLSurface::getNumVerticesY() 
{
    return m_NumVertTextures+1;
}

DPoint OGLSurface::getOrigVertexCoord(int x, int y)
{
    if (!m_bBound) {
        AVG_TRACE(Logger::WARNING, 
                "getOrigVertexCoord called, but image not available.");
        return DPoint(0,0);
    }
    if (x < 0 || x >= m_NumHorizTextures || y < 0 || y >= m_NumVertTextures) {
        AVG_TRACE(Logger::WARNING, 
                "getOrigVertexCoord called, but coordinate out of bounds.");
        return DPoint(0,0);
    }
    DPoint Vertex;
    initTileVertex(x, y, Vertex);
    return Vertex;
}

DPoint OGLSurface::getWarpedVertexCoord(int x, int y)
{
    if (!m_bBound) {
        AVG_TRACE(Logger::WARNING, 
                "getWarpedVertexCoord called, but image not available.");
        return DPoint(0,0);
    }
    if (x < 0 || x >= m_NumHorizTextures || y < 0 || y >= m_NumVertTextures) {
        AVG_TRACE(Logger::WARNING, 
                "getWarpedVertexCoord called, but coordinate out of bounds.");
        return DPoint(0,0);
    }
    return m_TileVertices[y][x];
}

void OGLSurface::setWarpedVertexCoord(int x, int y, const DPoint& Vertex)
{
    if (!m_bBound) {
        AVG_TRACE(Logger::WARNING, 
                "setWarpedVertexCoord called, but image not available.");
        return;
    }
    if (x < 0 || x >= m_NumHorizTextures || y < 0 || y >= m_NumVertTextures) {
        AVG_TRACE(Logger::WARNING, 
                "setWarpedVertexCoord called, but coordinate out of bounds.");
        return;
    }
    m_TileVertices[y][x] = Vertex;
}

void OGLSurface::discardBmp()
{
    m_pBmp = BitmapPtr();
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
        default:
            return "UNKNOWN";
    }
}

void OGLSurface::bind() 
{
//    cerr << "OGLSurface::bind()" << endl;
    if (m_bBound) {
        rebind();
    } else {
        if (m_MemoryMode == PBO) {
            s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::bind: glBindBuffer()");
        }
        int TextureMode = m_pEngine->getTextureMode();
        int Width = m_Size.x;
        int Height = m_Size.y;
        m_pTiles.clear();
        vector<OGLTilePtr> v;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
        
        glTexParameteri(TextureMode, 
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(TextureMode, 
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(TextureMode, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(TextureMode, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glTexParameteri()");

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
                
                OGLTilePtr pTile = OGLTilePtr(new OGLTile(CurExtent, CurSize, m_pf,
                            m_pEngine));
                m_pTiles[y].push_back(pTile);
                pTile->downloadTextures(m_pBmp, m_Size.x, m_MemoryMode);
            }
        }
        if (m_MemoryMode == PBO) {
            s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
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
    int Width = m_Size.x;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    if (m_MemoryMode == PBO) {
        s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::rebind: glBindBuffer()");
    }
    for (unsigned int y=0; y<m_pTiles.size(); y++) {
        for (unsigned int x=0; x<m_pTiles[y].size(); x++) {
            m_pTiles[y][x]->downloadTextures(m_pBmp, m_Size.x, m_MemoryMode);
        }
    }
    if (m_MemoryMode == PBO) {
        s_BindBufferProc(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::rebind: glBindBuffer(0)");
    }
}

void OGLSurface::blt(const DRect* pDestRect, 
        double opacity, double angle, const DPoint& pivot, 
        DisplayEngine::BlendMode Mode)
{
    if (!m_bBound) {
        bind();
    }
    bltTexture(pDestRect, angle, pivot, Mode);
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

void OGLSurface::initTileVertices()
{
    std::vector<DPoint> TileVerticesLine(m_NumHorizTextures+1);
    m_TileVertices = std::vector<std::vector<DPoint> >
                (m_NumVertTextures+1, TileVerticesLine);
    for (unsigned int y=0; y<m_TileVertices.size(); y++) {
        for (unsigned int x=0; x<m_TileVertices[y].size(); x++) {
            initTileVertex(x, y, m_TileVertices[y][x]);
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
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("blend");
            break;
        case DisplayEngine::BLEND_ADD:
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            checkBlendModeError("add");
            break;
        case DisplayEngine::BLEND_MIN:
            glBlendEquation(GL_MIN);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("min");
            break;
        case DisplayEngine::BLEND_MAX:
            glBlendEquation(GL_MAX);
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
            << ", height: " << pDestRect->Height() << ", " 
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

OGLMemoryMode OGLSurface::getMemoryModeSupported()
{
    static bool s_bChecked = false;
    static OGLMemoryMode s_MemoryMode;
    if (!s_bChecked) {
        if (queryOGLExtension("GL_ARB_pixel_buffer_object") || 
            queryOGLExtension("GL_EXT_pixel_buffer_object"))
        {
            s_MemoryMode = PBO;
            s_GenBuffersProc = (PFNGLGENBUFFERSPROC)getFuzzyProcAddress("glGenBuffers");
            s_BufferDataProc = (PFNGLBUFFERDATAPROC)getFuzzyProcAddress("glBufferData");
            s_DeleteBuffersProc = (PFNGLDELETEBUFFERSPROC)getFuzzyProcAddress("glDeleteBuffers");
            s_BindBufferProc = (PFNGLBINDBUFFERPROC)getFuzzyProcAddress("glBindBuffer");
            s_MapBufferProc = (PFNGLMAPBUFFERPROC)getFuzzyProcAddress("glMapBuffer");
            s_UnmapBufferProc = (PFNGLUNMAPBUFFERPROC)getFuzzyProcAddress("glUnmapBuffer");
            AVG_TRACE(Logger::CONFIG, "Using pixel buffer objects.");
#ifndef __APPLE__
        } else if (queryGLXExtension("GLX_MESA_allocate_memoryx")) {
            // Disabled because it's buggy.
            s_MemoryMode = MESA;
            s_AllocMemMESAProc = (PFNGLXALLOCATEMEMORYMESAPROC)
                    glXGetProcAddressARB((const GLubyte*)"glXAllocateMemoryMESA");
            s_FreeMemMESAProc = (PFNGLXFREEMEMORYMESAPROC)
                    glXGetProcAddressARB((const GLubyte*)"glXFreeMemoryMESA");
            AVG_TRACE(Logger::CONFIG, "Using MESA extension to allocate AGP memory.");
#endif
        } else {
            s_MemoryMode = OGL;
            AVG_TRACE(Logger::CONFIG, "Not using GL memory extensions.");
        }
        s_bChecked = true;
    }
    return s_MemoryMode;
}

}
