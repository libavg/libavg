//
// $Id$
//

#include "AVGOGLSurface.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGException.h"
#include "AVGPoint.h"
#include "OGLHelper.h"
#include "MathHelper.h"

#include <paintlib/plstdpch.h>
#include <paintlib/plrect.h>
#include <paintlib/planybmp.h>
#include <paintlib/plsubbmp.h>
#include <paintlib/plpngenc.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <sstream>

using namespace std;

int AVGOGLSurface::s_TextureMode = 0;
int AVGOGLSurface::s_MaxTexSize = 0;

AVGOGLSurface::AVGOGLSurface()
    : m_bBound(false),
      m_pBmp(0),
      m_pSubBmp(0),
      m_MaxTileSize(PLPoint(-1,-1))
{
    // Do an NVIDIA texture support query if it hasn't happened already.
    getTextureMode();
}

AVGOGLSurface::~AVGOGLSurface()
{
    discardBmp();
    if (m_bBound) {
        unbind();
    }
}

void AVGOGLSurface::create(int Width, int Height, int bpp, 
                bool bHasAlpha)
{
    discardBmp();
    if (m_bBound) {
        unbind();
    }
    m_pBmp = new PLAnyBmp;
    dynamic_cast<PLAnyBmp*>(m_pBmp)->Create(Width, Height, bpp, 
            bHasAlpha, false);
    m_pSubBmp = 0;
    setupTiles();
    initTileVertices();
}

PLBmpBase* AVGOGLSurface::getBmp()
{
    return m_pBmp;
}

void AVGOGLSurface::setMaxTileSize(const PLPoint& MaxTileSize)
{
    if (m_bBound) {
        AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                "Ignoring setMaxTileSize because textures are already bound.");
        return;
    }
    m_MaxTileSize = MaxTileSize;
    if (m_MaxTileSize.x != -1) {
        m_MaxTileSize.x = nextpow2(m_MaxTileSize.x/2+1);
    }
    if (m_MaxTileSize.y != -1) {
        m_MaxTileSize.y = nextpow2(m_MaxTileSize.y/2+1);
    }
    setupTiles();
    initTileVertices();
}

int AVGOGLSurface::getNumVerticesX()
{
    return m_NumHorizTextures+1;
}

int AVGOGLSurface::getNumVerticesY() 
{
    return m_NumVertTextures+1;
}

AVGDPoint AVGOGLSurface::getOrigVertexCoord(int x, int y)
{
    AVGDPoint Vertex;
    initTileVertex(x, y, Vertex);
    return Vertex;
}

AVGDPoint AVGOGLSurface::getWarpedVertexCoord(int x, int y)
{
    return m_TileVertices[y][x];
}

void AVGOGLSurface::setWarpedVertexCoord(int x, int y, const AVGDPoint& Vertex)
{
    m_TileVertices[y][x] = Vertex;
}


void AVGOGLSurface::createFromBits(int Width, int Height, int bpp, 
        bool bHasAlpha, PLBYTE* pBits, int Stride)
{
    if (m_bBound && 
        (!m_pSubBmp ||
        Width != m_pBmp->GetWidth() || Height != m_pBmp->GetHeight() ||
        bpp != m_pBmp->GetBitsPerPixel() || bHasAlpha != m_pBmp->HasAlpha()))
    {
        unbind();
    }
    if (!m_pSubBmp) {
        discardBmp();
        m_pBmp = new PLSubBmp;
        m_pSubBmp = dynamic_cast<PLSubBmp*>(m_pBmp);
    }
    
    m_pSubBmp->Create(Width, Height, bpp, bHasAlpha, pBits, Stride);
    setupTiles();
}

void AVGOGLSurface::discardBmp()
{
    if (m_pBmp) {
        delete m_pBmp;
        m_pBmp = 0;
    }
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

void AVGOGLSurface::bind() 
{
    if (m_bBound) {
//        unbind();
        rebind();
    }  else {
        int DestMode = getDestMode();
        int SrcMode = getSrcMode();
        int Width = m_pBmp->GetWidth();
        int Height = m_pBmp->GetHeight();
        m_Tiles.clear();
        vector<TextureTile> v;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGOGLSurface::bind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGOGLSurface::bind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");

        for (int y=0; y<m_NumVertTextures; y++) {
            m_Tiles.push_back(v);
            for (int x=0; x<m_NumHorizTextures; x++) {
                PLPoint CurSize = m_TileSize;
                if (y == m_NumVertTextures-1) {
                    CurSize.y = Height-y*m_TileSize.y;
                }
                if (x == m_NumHorizTextures-1) {
                    CurSize.x = Width-x*m_TileSize.x;
                }
                PLRect CurExtent(x*m_TileSize.x, y*m_TileSize.y,
                        x*m_TileSize.x+CurSize.x, y*m_TileSize.y+CurSize.y);
                TextureTile Tile;
                Tile.m_Extent = CurExtent;
                Tile.m_TexWidth = CurSize.x;
                Tile.m_TexHeight = CurSize.y;
                if (getTextureMode() == GL_TEXTURE_2D) {
                    Tile.m_TexWidth = nextpow2(CurSize.x);
                    Tile.m_TexHeight = nextpow2(CurSize.y);
                }
                glGenTextures(1, &Tile.m_TexID);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "AVGOGLSurface::bind: glGenTextures()");
                m_Tiles[y].push_back(Tile);

                glBindTexture(s_TextureMode, Tile.m_TexID);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                        "AVGOGLSurface::bind: glBindTexture()");

                glTexParameteri(s_TextureMode, 
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(s_TextureMode, 
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(s_TextureMode, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(s_TextureMode, GL_TEXTURE_WRAP_T, GL_CLAMP);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "AVGOGLSurface::bind: glTexParameteri()");

                glTexImage2D(s_TextureMode, 0,
                        DestMode, Tile.m_TexWidth, Tile.m_TexHeight, 0,
                        SrcMode, GL_UNSIGNED_BYTE, 0); 
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "AVGOGLSurface::bind: glTexImage2D()");
                PLBYTE * pStartPos = 
                        m_pBmp->GetLineArray()[Tile.m_Extent.tl.y]
                        + Tile.m_Extent.tl.x*m_pBmp->GetBitsPerPixel()/8;
                glTexSubImage2D(s_TextureMode, 0, 0, 0, 
                        Tile.m_Extent.Width(), Tile.m_Extent.Height(),
                        SrcMode, GL_UNSIGNED_BYTE, pStartPos);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "AVGOGLSurface::bind: glTexSubImage2D()");
            }
        }
        m_bBound = true;
    }
}

void AVGOGLSurface::unbind() 
{
    if (m_bBound) {
        for (int y=0; y<m_Tiles.size(); y++) {
            for (int x=0; x<m_Tiles[y].size(); x++) {
                glDeleteTextures(1, &m_Tiles[y][x].m_TexID);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "AVGOGLSurface::unbind: glDeleteTextures()");
            }
        }
        m_Tiles.clear();
    }
    m_bBound = false;
}

void AVGOGLSurface::rebind()
{
    int Width = m_pBmp->GetWidth();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");

    for (int y=0; y<m_Tiles.size(); y++) {
        for (int x=0; x<m_Tiles[y].size(); x++) {
            glBindTexture(s_TextureMode, m_Tiles[y][x].m_TexID);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "AVGOGLSurface::rebind: glBindTexture()");
            TextureTile Tile = m_Tiles[y][x];
            PLBYTE * pStartPos = 
                m_pBmp->GetLineArray()[Tile.m_Extent.tl.y]
                + Tile.m_Extent.tl.x*m_pBmp->GetBitsPerPixel()/8;
            glTexSubImage2D(s_TextureMode, 0, 0, 0, 
                    Tile.m_Extent.Width(), Tile.m_Extent.Height(),
                    getSrcMode(), GL_UNSIGNED_BYTE, pStartPos);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "AVGOGLSurface::rebind: glTexSubImage2D()");
        }
    }
}

void AVGOGLSurface::blt(const AVGDRect* pDestRect, double opacity, 
                double angle, const AVGDPoint& pivot)
{
    if (!m_bBound) {
        bind();
    }
    bltTexture(pDestRect, angle, pivot);
}

unsigned int AVGOGLSurface::getTexID()
{
    if (m_Tiles.size() != 1 || m_Tiles[0].size() != 1) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "AVGOGLSurface::getTexID() called for tiled surface. Aborting");
        exit(-1);
    }
    return m_Tiles[0][0].m_TexID;
}

int AVGOGLSurface::getTextureMode()
{
     if (s_TextureMode == 0) {
        // TODO: Change to GL_TEXTURE_RECTANGLE_EXT so we don't depend on 
        // proprietary NVidia stuff
        if (queryOGLExtension("GL_NV_texture_rectangle")) {
            s_TextureMode = GL_TEXTURE_RECTANGLE_NV;
            AVG_TRACE(AVGPlayer::DEBUG_CONFIG, 
                    "Using NVidia texture rectangle extension.");
        } else {
            s_TextureMode = GL_TEXTURE_2D;
            AVG_TRACE(AVGPlayer::DEBUG_CONFIG, 
                    "Using power of 2 textures.");
        }
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s_MaxTexSize);
        AVG_TRACE(AVGPlayer::DEBUG_CONFIG,
                "Max. texture size is " << s_MaxTexSize);
    }
    return s_TextureMode;
}

void AVGOGLSurface::setupTiles()
{
    int Width = m_pBmp->GetWidth();
    int Height = m_pBmp->GetHeight();
    if (Width > s_MaxTexSize || Height > s_MaxTexSize) {
        m_TileSize = PLPoint(s_MaxTexSize/2, s_MaxTexSize/2);
    } else {
        if (getTextureMode() == GL_TEXTURE_2D) {
            if ((Width > 256 && nextpow2(Width) > Width*1.3) ||
                    (Height > 256 && nextpow2(Height) > Height*1.3)) 
            {
                m_TileSize = PLPoint(nextpow2(Width)/2, nextpow2(Height)/2);
            } else {
                m_TileSize = PLPoint(nextpow2(Width), nextpow2(Height));
            }
        } else {
            m_TileSize = PLPoint(Width, Height);
        }
    }
    if (m_MaxTileSize.x != -1 && m_MaxTileSize.x < m_TileSize.x) {
        m_TileSize.x = m_MaxTileSize.x;
    }
    if (m_MaxTileSize.y != -1 && m_MaxTileSize.y < m_TileSize.y) {
        m_TileSize.y = m_MaxTileSize.y;
    }
    m_NumHorizTextures = int(ceil(float(Width)/m_TileSize.x));
    m_NumVertTextures = int(ceil(float(Height)/m_TileSize.y));

}

void AVGOGLSurface::initTileVertices()
{
    std::vector<AVGDPoint> TileVerticesLine(m_NumHorizTextures+1);
    m_TileVertices = std::vector<std::vector<AVGDPoint> >
                (m_NumVertTextures+1, TileVerticesLine);
    for (int y=0; y<m_TileVertices.size(); y++) {
        for (int x=0; x<m_TileVertices[y].size(); x++) {
            initTileVertex(x, y, m_TileVertices[y][x]);
        }
    }
}

void AVGOGLSurface::initTileVertex (int x, int y, AVGDPoint& Vertex) 
{
    if (x < m_NumHorizTextures) {
        Vertex.x = double(m_TileSize.x*x) / m_pBmp->GetWidth();
    } else {
        Vertex.x = 1;
    }
    if (y < m_NumVertTextures) {
        Vertex.y = double(m_TileSize.y*y) / m_pBmp->GetHeight();
    } else {
        Vertex.y = 1;
    }
}

void AVGOGLSurface::bltTexture(const AVGDRect* pDestRect, 
                double angle, const AVGDPoint& pivot)
{
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    AVGDPoint center(pDestRect->tl.x+pivot.x,
            pDestRect->tl.y+pivot.y);
    
    glPushMatrix();
    glTranslated(center.x, center.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "bltTexture: glTranslated");
    glRotated(angle, 0, 0, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "bltTexture: glRotated");
    glTranslated(-center.x, -center.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "bltTexture: glTranslated");

    for (int y=0; y<m_Tiles.size(); y++) {
        for (int x=0; x<m_Tiles[y].size(); x++) {
            TextureTile& CurTile = m_Tiles[y][x];
            AVGDPoint TLPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y][x]);
            AVGDPoint TRPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y][x+1]);
            AVGDPoint BLPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y+1][x]);
            AVGDPoint BRPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y+1][x+1]);
            bltTile(CurTile, TLPoint, TRPoint, BLPoint, BRPoint); 
        }
    }

    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "(" << pDestRect->tl.x << ", " 
            << pDestRect->tl.y << ")" << ", width:" << pDestRect->Width() 
            << ", height: " << pDestRect->Height() << ", " 
            << getGlModeString(getSrcMode()) << "-->" 
            << getGlModeString(getDestMode()) << endl);
    glPopMatrix();
    
}

AVGDPoint AVGOGLSurface::calcFinalVertex(const AVGDRect* pDestRect,
        const AVGDPoint & NormalizedVertex)
{
    AVGDPoint Point;
    Point.x = pDestRect->tl.x+
        (pDestRect->Width()*NormalizedVertex.x);
    Point.y = pDestRect->tl.y+
        (pDestRect->Height()*NormalizedVertex.y);
    return Point;
}

int AVGOGLSurface::bltTile(const TextureTile& Tile,
        const AVGDPoint& TLPoint, const AVGDPoint& TRPoint,
        const AVGDPoint& BLPoint, const AVGDPoint& BRPoint)
{
    double TexWidth;
    double TexHeight;
    if (getTextureMode() == GL_TEXTURE_2D) {
        TexWidth = double(Tile.m_Extent.Width())/Tile.m_TexWidth;
        TexHeight = double(Tile.m_Extent.Height())/Tile.m_TexHeight;
    } else {
        TexWidth = Tile.m_TexWidth;
        TexHeight = Tile.m_TexHeight;
    }
    
    glBindTexture(s_TextureMode, Tile.m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::bltTile: glBindTexture()");
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
            "AVGSDLDisplayEngine::bltTile: glEnd()");
}

int AVGOGLSurface::getDestMode()
{
    int bpp = m_pBmp->GetBitsPerPixel();
    switch (bpp) {
        case 8:
            return GL_ALPHA;
            break;
        case 24:
            return GL_RGB;
            break;
        case 32:
            if (m_pBmp->HasAlpha()) {
                return GL_RGBA;
            } else {
                return GL_RGB;    
            }
            break;
        default:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Unsupported bpp " << 
                    bpp << " in AVGOGLSurface::bind()");
    }
}    

int AVGOGLSurface::getSrcMode()
{
    switch (m_pBmp->GetBitsPerPixel()) {
        case 8:
            return GL_ALPHA;
        case 24:
            return GL_RGB;
        case 32:
            return GL_RGBA;
        default:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Unsupported bpp " << 
                    m_pBmp->GetBitsPerPixel() <<
                    " in AVGOGLSurface::getSrcMode()");
    }
}

