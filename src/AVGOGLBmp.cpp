//
// $Id$
//

#include "AVGOGLBmp.h"
#include "OGLHelper.h"
#include "AVGException.h"

#include <paintlib/plstdpch.h>

#include <GL/gl.h>
#include <GL/glu.h>


AVGOGLBmp::AVGOGLBmp()
    : m_TexID(0),
      m_bBound(false)
{
}


AVGOGLBmp::~AVGOGLBmp()
{
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
    TexMode = GL_TEXTURE_RECTANGLE_NV;
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
    if (HasAlpha()) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                GL_RGBA, GetWidth(), GetHeight(), 0,
                GL_BGRA, GL_UNSIGNED_BYTE, GetLineArray()[0]);
    } else {
        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                GL_RGB, GetWidth(), GetHeight(), 0,
                GL_BGRA, GL_UNSIGNED_BYTE, GetLineArray()[0]);
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

void AVGOGLBmp::freeMembers()
{
    if (m_TexID != -1) {
        unbind();
    }
}

