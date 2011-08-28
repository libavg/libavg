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

#include "OGLHelper.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/ObjectCounter.h"

#include <stdio.h>

using namespace std;
using namespace boost;

namespace avg {

thread_specific_ptr<vector<unsigned int> > FBO::s_pFBOIDs;

FBO::FBO(const IntPoint& size, PixelFormat pf, unsigned numTextures, 
        unsigned multisampleSamples, bool bUsePackedDepthStencil, bool bMipmap)
    : m_Size(size),
      m_PF(pf),
      m_MultisampleSamples(multisampleSamples),
      m_bUsePackedDepthStencil(bUsePackedDepthStencil),
      m_bMipmap(bMipmap)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    AVG_ASSERT(numTextures == 1 || multisampleSamples == 1);
    if (multisampleSamples > 1 && !(isMultisampleFBOSupported())) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Multisample offscreen rendering is not supported by this OpenGL driver/card combination.");
    }
    initCache();

    for (unsigned i=0; i<numTextures; ++i) {
        GLTexturePtr pTex = GLTexturePtr(new GLTexture(size, pf, bMipmap));
        // Workaround for NVidia driver bug - GL_TEX_MIN_FILTER without
        // glGenerateMipmaps causes FBO creation to fail(?!).
        pTex->generateMipmaps();
        m_pTextures.push_back(pTex);
    }
    init();
}

FBO::~FBO()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    
    unsigned oldFBOID;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&oldFBOID);
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    
    for (unsigned i=0; i<m_pTextures.size(); ++i) {
        glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+i, 
                GL_TEXTURE_2D, 0, 0);
    }
    
    returnToCache(m_FBO);
    if (m_MultisampleSamples > 1) {
        glproc::DeleteRenderbuffers(1, &m_ColorBuffer);
        returnToCache(m_OutputFBO);
    }
    if (m_bUsePackedDepthStencil && isPackedDepthStencilSupported()) {
        glproc::DeleteRenderbuffers(1, &m_StencilBuffer);
        glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
                GL_RENDERBUFFER_EXT, 0);
        glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                GL_RENDERBUFFER_EXT, 0);
        if (m_MultisampleSamples > 1) {
            glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_OutputFBO);
            glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                    GL_TEXTURE_2D, 0, 0);
        }
    }
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, oldFBOID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "~FBO");
}

void FBO::activate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::activate: BindFramebuffer()");
    checkError("activate");
}

PixelFormat FBO::getPF() const
{
    return m_PF;
}

unsigned FBO::getNumTextures() const
{
    return m_pTextures.size();
}

void FBO::copyToDestTexture() const
{
    if (m_MultisampleSamples != 1) {
        // Copy Multisample FBO to destination fbo
        glproc::BindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_FBO);
        glproc::BindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, m_OutputFBO);
        glproc::BlitFramebuffer(0, 0, m_Size.x, m_Size.y, 0, 0, m_Size.x, m_Size.y,
                GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    }
    if (m_bMipmap) {
        for (unsigned i=0; i< m_pTextures.size(); ++i) {
            m_pTextures[i]->generateMipmaps();
        }
    }
}

BitmapPtr FBO::getImage(int i) const
{
    copyToDestTexture();
    if (m_MultisampleSamples != 1) {
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_OutputFBO);
    } else {
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    }
    PixelFormat pf = m_pOutputPBO->getPF();
    IntPoint size = m_pOutputPBO->getSize();
    BitmapPtr pBmp(new Bitmap(size, pf));

    m_pOutputPBO->activate();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage BindBuffer()");
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT+i);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage ReadBuffer()");

    glReadPixels (0, 0, size.x, size.y, GLTexture::getGLFormat(pf), 
            GLTexture::getGLType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage ReadPixels()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage MapBuffer()");
    Bitmap PBOBitmap(size, pf, (unsigned char *)pPBOPixels, 
            size.x*Bitmap::getBytesPerPixel(pf), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage: UnmapBuffer()");
    m_pOutputPBO->deactivate();
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    return pBmp;
}

GLTexturePtr FBO::getTex(int i) const
{
    return m_pTextures[i];
}

const IntPoint& FBO::getSize() const
{
    return m_Size;
}

void FBO::initCache()
{
    if (s_pFBOIDs.get() == 0) {
        s_pFBOIDs.reset(new vector<unsigned int>);
    }
}

void FBO::deleteCache()
{
    if (s_pFBOIDs.get() != 0) {
        for (unsigned i=0; i<s_pFBOIDs->size(); ++i) {
            glproc::DeleteFramebuffers(1, &((*s_pFBOIDs)[i]));
        }
        s_pFBOIDs->clear();
        s_pFBOIDs.reset();
    }
}

void FBO::init()
{
    if (m_bUsePackedDepthStencil && !isPackedDepthStencilSupported()) {
        throw Exception(AVG_ERR_UNSUPPORTED, "OpenGL implementation does not support offscreen cropping (GL_EXT_packed_depth_stencil).");
    }
    if (m_MultisampleSamples > 1 && !isMultisampleFBOSupported()) {
        throw Exception(AVG_ERR_UNSUPPORTED, "OpenGL implementation does not support multisample offscreen rendering (GL_EXT_framebuffer_multisample).");
    }
    m_pOutputPBO = PBOPtr(new PBO(m_Size, m_PF, GL_STREAM_READ));

    m_FBO = genFramebuffer();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: GenFramebuffers()");

    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: BindFramebuffer()");

    if (m_MultisampleSamples == 1) {
        glDisable(GL_MULTISAMPLE);
        for (unsigned i=0; i<m_pTextures.size(); ++i) {
            glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
                    GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_2D, 
                    m_pTextures[i]->getID(), 0);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO: glFramebufferTexture2D()");
        }
        if (m_bUsePackedDepthStencil) {
            glproc::GenRenderbuffers(1, &m_StencilBuffer);
            glproc::BindRenderbuffer(GL_RENDERBUFFER_EXT, m_StencilBuffer);
            glproc::RenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT, 
                    m_Size.x, m_Size.y);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
                    GL_RENDERBUFFER_EXT, m_StencilBuffer);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                    GL_RENDERBUFFER_EXT, m_StencilBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "FBO::init: FramebufferRenderbuffer(STENCIL)");
        }
    } else {
        glEnable(GL_MULTISAMPLE);
        glproc::GenRenderbuffers(1, &m_ColorBuffer);
        glproc::BindRenderbuffer(GL_RENDERBUFFER_EXT, m_ColorBuffer);
        glproc::RenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, m_MultisampleSamples,
                GL_RGBA8, m_Size.x, m_Size.y);
        GLenum err = glGetError();
        if (err == GL_INVALID_VALUE) {
            glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
            glproc::DeleteFramebuffers(1, &m_FBO);
            glproc::DeleteRenderbuffers(1, &m_ColorBuffer);
            m_pOutputPBO = PBOPtr();
            throw(Exception(AVG_ERR_UNSUPPORTED, 
                    string("Unsupported value for number of multisample samples (")
                    + toString(m_MultisampleSamples) + ")."));
        }
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: RenderbufferStorageMultisample");
        glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                GL_RENDERBUFFER_EXT, m_ColorBuffer);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: FramebufferRenderbuffer");
        if (m_bUsePackedDepthStencil) {
            glproc::GenRenderbuffers(1, &m_StencilBuffer);
            glproc::BindRenderbuffer(GL_RENDERBUFFER_EXT, m_StencilBuffer);
            glproc::RenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, 
                    m_MultisampleSamples, GL_DEPTH_STENCIL_EXT, m_Size.x, m_Size.y);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
                    GL_RENDERBUFFER_EXT, m_StencilBuffer);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                    GL_RENDERBUFFER_EXT, m_StencilBuffer);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                    "FBO::init: FramebufferRenderbuffer(STENCIL)");
        }
        checkError("init multisample");
        m_OutputFBO = genFramebuffer();
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_OutputFBO);
        glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                GL_TEXTURE_2D, m_pTextures[0]->getID(), 0);

        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::init: Multisample init");
    }

    checkError("init");
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

unsigned FBO::genFramebuffer() const
{
    unsigned fboID;
    if (s_pFBOIDs->empty()) {
        glproc::GenFramebuffers(1, &fboID);
    } else {
        fboID = s_pFBOIDs->back();
        s_pFBOIDs->pop_back();
    }
    return fboID;
}

void FBO::returnToCache(unsigned fboID) 
{
    if (s_pFBOIDs.get() != 0) {
        s_pFBOIDs->push_back(fboID);
    }
}
bool FBO::isFBOSupported()
{
    return queryOGLExtension("GL_EXT_framebuffer_object");
}

bool FBO::isMultisampleFBOSupported()
{
    int maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
    // For some reason, this fails on Linux/i945 and similar setups. Multisample
    // FBO is broken anyway on these machines, so we just return false...
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        return false;
    }
    return queryOGLExtension("GL_EXT_framebuffer_multisample") && 
            queryOGLExtension("GL_EXT_framebuffer_blit") && maxSamples > 1;
}
    
bool FBO::isPackedDepthStencilSupported()
{
    return queryOGLExtension("GL_EXT_packed_depth_stencil");
}

void FBO::checkError(const string& sContext) const
{
    GLenum status = glproc::CheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    string sErr;
    switch (status) {
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
    cerr << "Framebuffer error (" << sContext << "): " << sErr << endl;
    AVG_ASSERT(false);
}

}


