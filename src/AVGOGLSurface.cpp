//
// $Id$
//

#include "AVGOGLSurface.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGException.h"
#include "OGLHelper.h"

#include <paintlib/plstdpch.h>
#include <paintlib/plrect.h>
#include <paintlib/planybmp.h>
#include <paintlib/plsubbmp.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <sstream>

using namespace std;

int AVGOGLSurface::s_TextureMode = 0;
int AVGOGLSurface::s_MaxTexSize = 0;

AVGOGLSurface::AVGOGLSurface()
    : m_TexID(0),
      m_bBound(false),
      m_pBmp(0),
      m_pSubBmp(0)
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
}

PLBmpBase* AVGOGLSurface::getBmp()
{
    return m_pBmp;
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
}

void AVGOGLSurface::discardBmp()
{
    if (m_pBmp) {
        delete m_pBmp;
        m_pBmp = 0;
    }
}

int nextpow2(int n) {
    double d = ::log(n)/::log(2);
    return int(pow(2, ceil(d)));
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
        rebind();
    } else {
        bindOneTexture(m_TexID);
        m_bBound = true;
    }
}

void AVGOGLSurface::unbind() 
{
    if (m_bBound) {
        glDeleteTextures(1, &m_TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGOGLSurface::unbind: glDeleteTextures()");
    }
    m_bBound = false;
}

void AVGOGLSurface::rebind()
{
    glBindTexture(s_TextureMode, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glBindTexture()");
    glTexSubImage2D(s_TextureMode, 0, 0, 0, m_pBmp->GetWidth(), 
            m_pBmp->GetHeight(), getSrcMode(), GL_UNSIGNED_BYTE, 
            m_pBmp->GetLineArray()[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glTexSubImage2D()");
}

int AVGOGLSurface::getTexID() 
{
    return m_TexID;
}

int AVGOGLSurface::getTexHeight()
{
    return m_TexHeight;
}

int AVGOGLSurface::getTexWidth()
{
    return m_TexWidth;
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

void AVGOGLSurface::bindOneTexture(unsigned int& TexID)
{

    int Width = m_pBmp->GetWidth();
    int Height = m_pBmp->GetHeight();
    int bpp = m_pBmp->GetBitsPerPixel();
    
    int DestMode = getDestMode();
    int SrcMode = getSrcMode();

    glGenTextures(1, &TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::bindOneTexture: glGenTextures()");

    glBindTexture(s_TextureMode, TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "AVGOGLSurface::bindOneTexture: glBindTexture()");

    glTexParameteri(s_TextureMode, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(s_TextureMode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::bind: glTexParameteri()");
    
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Texture upload. Size=" << 
            Width << "x" << Height << ", SrcMode=" <<
            getGlModeString(SrcMode) << ", DestMode=" << 
            getGlModeString(DestMode) << ".");
    if (Width > s_MaxTexSize || Height > s_MaxTexSize)
    {
        stringstream s;
        s << "Texture size is " << Width << "x" 
            << Height << ", OpenGL maximum is " 
            << s_MaxTexSize << "." << endl;
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, s.str());
    }
    if (getTextureMode() == GL_TEXTURE_RECTANGLE_NV) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                DestMode, Width, Height, 0,
                getSrcMode(), GL_UNSIGNED_BYTE, m_pBmp->GetLineArray()[0]);
    } else {
        // Only pow2 textures supported.
        PLAnyBmp Pow2Bmp;
        m_TexWidth = nextpow2(Width);
        m_TexHeight = nextpow2(Height);
        Pow2Bmp.Create(m_TexWidth, m_TexHeight, bpp, 
                m_pBmp->HasAlpha(), false);
        for (int y=0; y<Height; y++) {
            memcpy (Pow2Bmp.GetLineArray()[y], m_pBmp->GetLineArray()[y],
                    Width*bpp/8);
        }
        glTexImage2D(GL_TEXTURE_2D, 0,
                DestMode, m_TexWidth, m_TexHeight, 0,
                getSrcMode(), GL_UNSIGNED_BYTE, Pow2Bmp.GetPixels());
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::bind: glTexImage2D()");
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

