//
// $Id$
//

#include "AVGOGLBmp.h"
#include "OGLHelper.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <paintlib/plstdpch.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <sstream>

int AVGOGLBmp::m_TextureMode = 0;

AVGOGLBmp::AVGOGLBmp()
    : m_TexID(0),
      m_bBound(false)
{
    // Do an NVIDIA texture support query if it hasn't happened already.
    getTextureMode();
}


AVGOGLBmp::~AVGOGLBmp()
{
}

int nextpow2(int n) {
    double d = log(n)/log(2);
    return int(pow(2, ceil(d)));
}

void AVGOGLBmp::bind() 
{
    if (m_bBound) {
        unbind();
    }
    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLBmp::bind: glGenTextures()");

    GLenum TexMode;
    TexMode = m_TextureMode;
    glBindTexture(TexMode, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "AVGOGLBmp::bind: glBindTexture()");

    glTexParameteri(TexMode, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(TexMode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TexMode, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(TexMode, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLBmp::bind: glTexParameteri()");
    int DestMode;
    if (HasAlpha()) {
        DestMode = GL_RGBA;
    } else {
        DestMode = GL_RGB;    
    }
    if (getTextureMode() == GL_TEXTURE_RECTANGLE_NV) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                DestMode, GetWidth(), GetHeight(), 0,
                GL_BGRA, GL_UNSIGNED_BYTE, GetLineArray()[0]);
    } else {
        // Only pow2 textures supported.
        PLAnyBmp Pow2Bmp;
        m_TexSize = GetWidth();
        if (m_TexSize < GetHeight()) {
            m_TexSize = GetHeight();
        }
        m_TexSize = nextpow2(m_TexSize);
        Pow2Bmp.Create(m_TexSize, m_TexSize, 32, false, false);
        for (int y=0; y<GetHeight(); y++) {
            memcpy (Pow2Bmp.GetLineArray()[y], GetLineArray()[y],
                    GetWidth()*4);
        }
        glTexImage2D(GL_TEXTURE_2D, 0,
                DestMode, m_TexSize, m_TexSize, 0,
                GL_BGRA, GL_UNSIGNED_BYTE, Pow2Bmp.GetPixels());
    }
    
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLBmp::bind: glTexImage2D()");
    m_bBound = true;
}

void AVGOGLBmp::unbind() 
{
    if (m_bBound) {
        glDeleteTextures(1, &m_TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGOGLBmp::bind: glDeleteTextures()");
    }
    m_bBound = false;
}

int AVGOGLBmp::getTexID() 
{
    return m_TexID;
}

int AVGOGLBmp::getTexSize()
{
    return m_TexSize;
}

void AVGOGLBmp::freeMembers()
{
    if (m_TexID != -1) {
        unbind();
    }
}

int AVGOGLBmp::getTextureMode()
{
     if (m_TextureMode == 0) {
        // TODO: Change to GL_TEXTURE_RECTANGLE_EXT so we don't depend on 
        // proprietary NVidia stuff
        if (!queryOGLExtension("GL_NV_texture_rectangle")) {
            m_TextureMode = GL_TEXTURE_RECTANGLE_NV;
            AVG_TRACE(AVGPlayer::DEBUG_BLTS, 
                    "Using NVidia texture rectangle extension.");
        } else {
            m_TextureMode = GL_TEXTURE_2D;
            AVG_TRACE(AVGPlayer::DEBUG_BLTS, 
                    "Using power of 2 textures.");
        }
    }
    return m_TextureMode;
}
