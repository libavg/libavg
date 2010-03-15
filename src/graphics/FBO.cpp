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

#include "FBO.h"

#include "../base/Exception.h"
#include "../graphics/PBOImage.h"
#include "../graphics/OGLHelper.h"

#include <stdio.h>

using namespace std;

namespace avg {

FBO::FBO(const IntPoint& size, PixelFormat pf, unsigned texID, 
        unsigned multisampleSamples)
    : m_Size(size),
      m_PF(pf),
      m_MultisampleSamples(multisampleSamples)
{
    m_TexIDs.push_back(texID);
    init();
    if (multisampleSamples > 1 && !(isMultisampleFBOSupported())) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Multisample offscreen rendering is not supported by this OpenGL driver/card combination.");
    }
}

FBO::FBO(const IntPoint& size, PixelFormat pf, vector<unsigned> texIDs)
    : m_Size(size),
      m_PF(pf),
      m_MultisampleSamples(1),
      m_TexIDs(texIDs)
{
    m_TexIDs = texIDs;
    init();
}

FBO::~FBO()
{
    glproc::DeleteFramebuffers(1, &m_FBO);
    if (m_MultisampleSamples > 1) {
        glproc::DeleteRenderbuffers(1, &m_ColorBuffer);
        glproc::DeleteFramebuffers(1, &m_OutputFBO);
    }
}

void FBO::copyToDestTexture() const
{
    if (m_MultisampleSamples != 1) {
        // Copy Multisample FBO to destination fbo
        glproc::BindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_FBO);
        glproc::BindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, m_OutputFBO);
        glproc::BlitFramebuffer(0, 0, m_Size.x, m_Size.y, 0, 0, m_Size.x, m_Size.y,
                GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_OutputFBO);
    }
}

BitmapPtr FBO::getImage(int i) const
{
    copyToDestTexture();
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    PixelFormat pf = m_pOutputPBO->getExtPF();
    IntPoint size = m_pOutputPBO->getSize();
    BitmapPtr pBmp(new Bitmap(size, pf));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_pOutputPBO->getOutputPBO());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage BindBuffer()");
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT+i);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage ReadBuffer()");

    glReadPixels (0, 0, size.x, size.y, m_pOutputPBO->getFormat(pf), 
            m_pOutputPBO->getType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage ReadPixels()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage MapBuffer()");
    Bitmap PBOBitmap(size, pf, (unsigned char *)pPBOPixels, 
            size.x*Bitmap::getBytesPerPixel(pf), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage: UnmapBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    return pBmp;
}

unsigned FBO::getTexture() const
{
    return m_TexIDs[0];
}

void FBO::init()
{
    m_pOutputPBO = PBOImagePtr(new PBOImage(m_Size, m_PF, m_PF, false, true));

    glproc::GenFramebuffers(1, &m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: GenFramebuffers()");

    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: BindFramebuffer()");

    if (m_MultisampleSamples == 1) {
        for (unsigned i=0; i<m_TexIDs.size(); ++i) {
            glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
                    GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_2D, 
                    m_TexIDs[i], 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO: glFramebufferTexture2D()");
        }
    } else {
        glproc::GenRenderbuffers(1, &m_ColorBuffer);
        glproc::BindRenderbuffer(GL_RENDERBUFFER_EXT, m_ColorBuffer);
        glproc::RenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, m_MultisampleSamples,
                GL_RGBA8, m_Size.x, m_Size.y);
        glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                GL_RENDERBUFFER_EXT, m_ColorBuffer);
        checkError();
        glproc::GenFramebuffers(1, &m_OutputFBO);
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_OutputFBO);
        glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                GL_TEXTURE_2D, m_TexIDs[0], 0);

        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: Multisample init");
    }

    checkError();
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

void FBO::activate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::activate: BindFramebuffer()");
    checkError();
}

void FBO::deactivate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::deactivate: BindFramebuffer()");
}

bool FBO::isFBOSupported()
{
    return queryOGLExtension("GL_EXT_framebuffer_object");
}

bool FBO::isMultisampleFBOSupported()
{
    return queryOGLExtension("GL_EXT_framebuffer_multisample") && 
            queryOGLExtension("GL_EXT_framebuffer_blit");
}
    
void FBO::checkError() const
{
    GLenum status = glproc::CheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    string sErr;
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            return;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            sErr = "GL_FRAMEBUFFER_UNSUPPORTED_EXT";
            throw Exception(AVG_ERR_UNSUPPORTED, string("Framebuffer error: ")+sErr);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            sErr = "GL_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
            break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
            // Missing in some versions of glext.h
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT";
            break;
#endif
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT";
            break;
        case GL_FRAMEBUFFER_BINDING_EXT:
            sErr = "GL_FRAMEBUFFER_BINDING_EXT";
            break;
        default:
            sErr = "Unknown error";
            break;
    }
    cerr << "Framebuffer error: " << sErr << endl;
    AVG_ASSERT(false);
}

} // namespace avg


