//
// $Id$
//

#include "Player.h"
#include "MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../graphics/Point.h"
#include "OGLSurface.h"
#include "OGLHelper.h"

#include "GL/glu.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

int OGLSurface::s_TextureMode = 0;
int OGLSurface::s_MaxTexSize = 0;
PFNGLXALLOCATEMEMORYMESAPROC OGLSurface::s_AllocMemMESAProc = 0;
PFNGLXFREEMEMORYMESAPROC OGLSurface::s_FreeMemMESAProc = 0;

OGLSurface::OGLSurface()
    : m_bBound(false),
      m_MaxTileSize(-1,-1)
{
    // Do an NVIDIA texture support query if it hasn't happened already.
    getTextureMode();
}

OGLSurface::~OGLSurface()
{
    unbind();
    switch(m_MemoryMode) {
        case PBO:
            glDeleteBuffers(1, &m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::~OGLSurface: glDeleteBuffers()");
            break;
        case MESA:
            {
                Display * display = XOpenDisplay(0);
                s_FreeMemMESAProc(display, DefaultScreen(display), m_pMESABuffer);
            }
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
            glGenBuffers(1, &m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::create: glGenBuffers()");
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::rebind: glBindBuffer()");
            glBufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 
                    (Size.x+1)*(Size.y+1)*Bitmap::getBytesPerPixel(pf), NULL, 
                    GL_STREAM_DRAW);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::create: glBufferData()");
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::rebind: glBindBuffer(0)");
            m_pBmp = BitmapPtr();
            break;
        case MESA:
            {
                Display * display = XOpenDisplay(0);
                m_pMESABuffer = s_AllocMemMESAProc(display, DefaultScreen(display),
                        (Size.x+1)*(Size.y+1)*Bitmap::getBytesPerPixel(pf), 0, 1.0 ,0);
                if (!m_pMESABuffer) {
                    AVG_TRACE(Logger::WARNING, "Failed to allocate MESA memory");
                    m_MemoryMode = OGL;
                }
                m_pBmp = BitmapPtr();
            }
            break;
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

BitmapPtr OGLSurface::lockBmp()
{
    switch (m_MemoryMode) {
        case PBO:
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glBindBuffer()");
                unsigned char * pBuffer = (unsigned char *)
                    glMapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glMapBuffer()");
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::lockBmp: glBindBuffer(0)");
                m_pBmp = BitmapPtr(new Bitmap(m_Size, m_pf, pBuffer, 
                            m_Size.x*Bitmap::getBytesPerPixel(m_pf), false));
            }
            break;
        case MESA:
            m_pBmp = BitmapPtr(new Bitmap(m_Size, m_pf, (unsigned char *)m_pMESABuffer, 
                    m_Size.x*Bitmap::getBytesPerPixel(m_pf), false));
        default:
            break;
    }
    return m_pBmp;
}

void OGLSurface::unlockBmp()
{
    switch (m_MemoryMode) {
        case PBO:
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::unlockBmp: glBindBuffer()");
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::unlockBmp: glUnmapBuffer()");
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::lockBmp: glBindBuffer(0)");
            m_pBmp = BitmapPtr();
            break;
        case MESA:
            m_pBmp = BitmapPtr();
            break;
        default:
            break;
    }
}

void OGLSurface::setMaxTileSize(const IntPoint& MaxTileSize)
{
    if (m_bBound) {
        AVG_TRACE(Logger::WARNING, 
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
    DPoint Vertex;
    initTileVertex(x, y, Vertex);
    return Vertex;
}

DPoint OGLSurface::getWarpedVertexCoord(int x, int y)
{
    return m_TileVertices[y][x];
}

void OGLSurface::setWarpedVertexCoord(int x, int y, const DPoint& Vertex)
{
    m_TileVertices[y][x] = Vertex;
}

void OGLSurface::createFromBits(IntPoint Size, PixelFormat pf,
        unsigned char * pBits, int Stride)
{
    if (m_MemoryMode != OGL) {
        AVG_TRACE(Logger::ERROR, 
                "createFromBits called while memory mode wasn't standard OpenGL.");
    }

    if (Size != m_pBmp->getSize() || pf != m_pBmp->getPixelFormat())
    {
        unbind();
    }
    if (!m_pBmp || m_pBmp->ownsBits()) {
        m_pBmp = BitmapPtr(new Bitmap(Size, pf, pBits, Stride, false, ""));
    }
    
    setupTiles();
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
    if (m_bBound) {
        rebind();
    } else {
        if (m_MemoryMode == PBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::bind: glBindBuffer()");
        }
        int DestMode = getDestMode();
        int SrcMode = getSrcMode();
        int PixelType = getPixelType();
        int Width = m_Size.x;
        int Height = m_Size.y;
        m_Tiles.clear();
        vector<TextureTile> v;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::bind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");

        for (int y=0; y<m_NumVertTextures; y++) {
            m_Tiles.push_back(v);
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
                        "OGLSurface::bind: glGenTextures()");
                m_Tiles[y].push_back(Tile);

                glBindTexture(s_TextureMode, Tile.m_TexID);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                        "OGLSurface::bind: glBindTexture()");

                glTexParameteri(s_TextureMode, 
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(s_TextureMode, 
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(s_TextureMode, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(s_TextureMode, GL_TEXTURE_WRAP_T, GL_CLAMP);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::bind: glTexParameteri()");

                glTexImage2D(s_TextureMode, 0,
                        DestMode, Tile.m_TexWidth, Tile.m_TexHeight, 0,
                        SrcMode, PixelType, 0); 
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::bind: glTexImage2D()");
                int bpp = Bitmap::getBytesPerPixel(m_pf);
                unsigned char * pStartPos = (unsigned char *) 
                        (Tile.m_Extent.tl.y*m_Size.x*bpp+
                        + Tile.m_Extent.tl.x*bpp);
                switch (m_MemoryMode) {
                    case OGL:
                        pStartPos += (unsigned int)(m_pBmp->getPixels());
                        break;
                    case MESA:
                        pStartPos += (unsigned int)(m_pMESABuffer);
                        break;
                    default:
                        break;
                }
                glTexSubImage2D(s_TextureMode, 0, 0, 0, 
                        Tile.m_Extent.Width(), Tile.m_Extent.Height(),
                        SrcMode, PixelType, pStartPos);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::bind: glTexSubImage2D()");
            }
        }
        if (m_MemoryMode == PBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::bind: glBindBuffer(0)");
        }
        m_bBound = true;
    }
}

void OGLSurface::unbind() 
{
    if (m_bBound) {
        for (unsigned int y=0; y<m_Tiles.size(); y++) {
            for (unsigned int x=0; x<m_Tiles[y].size(); x++) {
                glDeleteTextures(1, &m_Tiles[y][x].m_TexID);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "OGLSurface::unbind: glDeleteTextures()");
            }
        }
        m_Tiles.clear();
    }
    m_bBound = false;
}

static ProfilingZone TexSubImageProfilingZone("    OGLSurface::texture upload");

void OGLSurface::rebind()
{
    int Width = m_Size.x;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    if (m_MemoryMode == PBO) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_hPixelBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::rebind: glBindBuffer()");
    }
    for (unsigned int y=0; y<m_Tiles.size(); y++) {
        for (unsigned int x=0; x<m_Tiles[y].size(); x++) {
            TextureTile Tile = m_Tiles[y][x];
            glBindTexture(s_TextureMode, m_Tiles[y][x].m_TexID);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::rebind: glBindTexture()");
            int bpp = Bitmap::getBytesPerPixel(m_pf);
            unsigned char * pStartPos = (unsigned char *) 
                    (Tile.m_Extent.tl.y*m_Size.x*bpp+
                    + Tile.m_Extent.tl.x*bpp);
            switch (m_MemoryMode) {
                case OGL:
                    pStartPos += (unsigned int)(m_pBmp->getPixels());
                    break;
                case MESA:
                    pStartPos += (unsigned int)(m_pMESABuffer);
                    break;
                default:
                    break;
            }
            {
                ScopeTimer Timer(TexSubImageProfilingZone);
                glTexSubImage2D(s_TextureMode, 0, 0, 0, 
                        Tile.m_Extent.Width(), Tile.m_Extent.Height(),
                        getSrcMode(), getPixelType(), pStartPos);
            }
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "OGLSurface::rebind: glTexSubImage2D()");
        }
    }
    if (m_MemoryMode == PBO) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLSurface::rebind: glBindBuffer(0)");
    }
}

void OGLSurface::blt(const DRect* pDestRect, double opacity, 
                double angle, const DPoint& pivot, 
                DisplayEngine::BlendMode Mode)
{
    if (!m_bBound) {
        bind();
    }
    bltTexture(pDestRect, angle, pivot, Mode);
}

unsigned int OGLSurface::getTexID()
{
    if (m_Tiles.size() != 1 || m_Tiles[0].size() != 1) {
        AVG_TRACE(Logger::ERROR, 
                "OGLSurface::getTexID() called for tiled surface. Aborting");
        exit(-1);
    }
    return m_Tiles[0][0].m_TexID;
}

int OGLSurface::getTextureMode()
{
     if (s_TextureMode == 0) {
        // TODO: Support GL_TEXTURE_RECTANGLE_EXT so we don't depend on 
        // proprietary NVidia stuff
        if (queryOGLExtension("GL_NV_texture_rectangle")) {
            s_TextureMode = GL_TEXTURE_RECTANGLE_NV;
            AVG_TRACE(Logger::CONFIG, 
                    "Using NVidia texture rectangle extension.");
        } else {
            s_TextureMode = GL_TEXTURE_2D;
            AVG_TRACE(Logger::CONFIG, 
                    "Using power of 2 textures.");
        }
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s_MaxTexSize);
        AVG_TRACE(Logger::CONFIG,
                "Max. texture size is " << s_MaxTexSize);
        if (s_MaxTexSize == 0) {
            s_MaxTexSize = 512;
            AVG_TRACE(Logger::WARNING,
                "This looks like a broken OpenGL driver. Using max. texture size of 512.");
        }
    }
    return s_TextureMode;
}

void OGLSurface::setupTiles()
{
    if (m_Size.x > s_MaxTexSize || m_Size.y > s_MaxTexSize) {
        m_TileSize = IntPoint(s_MaxTexSize/2, s_MaxTexSize/2);
    } else {
        if (getTextureMode() == GL_TEXTURE_2D) {
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

    for (unsigned int y=0; y<m_Tiles.size(); y++) {
        for (unsigned int x=0; x<m_Tiles[y].size(); x++) {
            TextureTile& CurTile = m_Tiles[y][x];
            DPoint TLPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y][x]);
            DPoint TRPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y][x+1]);
            DPoint BLPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y+1][x]);
            DPoint BRPoint = 
                    calcFinalVertex(pDestRect, m_TileVertices[y+1][x+1]);
            bltTile(CurTile, TLPoint, TRPoint, BLPoint, BRPoint); 
        }
    }

    AVG_TRACE(Logger::BLTS, "(" << pDestRect->tl.x << ", " 
            << pDestRect->tl.y << ")" << ", width:" << pDestRect->Width() 
            << ", height: " << pDestRect->Height() << ", " 
            << getGlModeString(getSrcMode()) << "-->" 
            << getGlModeString(getDestMode()) << endl);
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

void OGLSurface::bltTile(const TextureTile& Tile,
        const DPoint& TLPoint, const DPoint& TRPoint,
        const DPoint& BLPoint, const DPoint& BRPoint)
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

int OGLSurface::getDestMode()
{
    switch (m_pf) {
        case I8:
            return GL_ALPHA;
        case R8G8B8:
        case B8G8R8:
            return GL_RGB;
        case R8G8B8A8:
        case B8G8R8A8:
            return GL_RGBA;
        case R8G8B8X8:
        case B8G8R8X8:
            return GL_RGB;    
        case YCbCr422:
            return GL_YCBCR_MESA;    
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported pixel format " << 
                    Bitmap::getPixelFormatString(m_pf) <<
                    " in OGLSurface::getDestMode()");
    }
    return 0;
}    

int OGLSurface::getSrcMode()
{
    switch (m_pf) {
        case I8:
            return GL_ALPHA;
        case R8G8B8:
            return GL_RGB;
        case B8G8R8:
            return GL_BGR;
        case B8G8R8X8:
        case B8G8R8A8:
            return GL_BGRA;
        case R8G8B8X8:
        case R8G8B8A8:
            return GL_RGBA;
        case YCbCr422:
            return GL_YCBCR_MESA;    
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported pixel format " << 
                    Bitmap::getPixelFormatString(m_pf) <<
                    " in OGLSurface::getSrcMode()");
    }
    return 0;
}

int OGLSurface::getPixelType()
{
    if (m_pf == YCbCr422) {
        return GL_UNSIGNED_SHORT_8_8_REV_MESA;
    } else {
        return GL_UNSIGNED_BYTE;
    }
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

OGLSurface::MemoryMode OGLSurface::getMemoryModeSupported()
{
    static bool s_bChecked = false;
    static MemoryMode s_MemoryMode;
    if (!s_bChecked) {
        if (queryOGLExtension("GL_ARB_pixel_buffer_object") || 
            queryOGLExtension("GL_EXT_pixel_buffer_object"))
        {
            s_MemoryMode = PBO;
            AVG_TRACE(Logger::CONFIG, "Using pixel buffer objects.");
        } else if (false) { // queryGLXExtension("GLX_MESA_allocate_memory")) {
            // Turned off - it seems to be slower than anything else...
            s_MemoryMode = MESA;
            s_AllocMemMESAProc = (PFNGLXALLOCATEMEMORYMESAPROC)
                    glXGetProcAddressARB((const GLubyte*)"glXAllocateMemoryMESA");
            s_FreeMemMESAProc = (PFNGLXFREEMEMORYMESAPROC)
                    glXGetProcAddressARB((const GLubyte*)"glXFreeMemoryMESA");
            AVG_TRACE(Logger::CONFIG, "Using MESA extension to allocate AGP memory.");
        } else {
            s_MemoryMode = OGL;
            AVG_TRACE(Logger::CONFIG, "Not using GL memory extensions.");
        }
        s_bChecked = true;
    }
    return s_MemoryMode;
}

}
