//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "GLContext.h"
#include "Filterfliprgb.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/ObjectCounter.h"

#include <stdio.h>

using namespace std;
using namespace boost;

namespace avg {

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#endif

#ifdef AVG_ENABLE_EGL
#define GL_DEPTH_STENCIL_EXT GL_DEPTH24_STENCIL8_OES
#endif

FBO::FBO(const IntPoint& size, PixelFormat pf, unsigned numTextures, 
        unsigned multisampleSamples, bool bUsePackedDepthStencil, bool bUseStencil,
        bool bMipmap)
    : m_Size(size),
      m_PF(pf),
      m_MultisampleSamples(multisampleSamples),
      m_bUsePackedDepthStencil(bUsePackedDepthStencil),
      m_bUseStencil(bUseStencil),
      m_bMipmap(bMipmap)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    AVG_ASSERT(numTextures == 1 || multisampleSamples == 1);
    if (multisampleSamples > 1 && !(isMultisampleFBOSupported())) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Multisample offscreen rendering is not supported by this OpenGL driver/card combination.");
    }
    if (multisampleSamples < 1) {
        throwMultisampleError();
    }
    for (unsigned i=0; i<numTextures; ++i) {
        GLTexturePtr pTex = GLTexturePtr(new GLTexture(size, pf, bMipmap));
        pTex->generateMipmaps();
        GLContext::checkError("FBO::FBO: generateMipmaps");
        m_pTextures.push_back(pTex);
    }
    init();
}

FBO::~FBO()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    
    unsigned oldFBOID;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&oldFBOID);
    glproc::BindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    
    for (unsigned i=0; i<m_pTextures.size(); ++i) {
        glproc::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, 
                GL_TEXTURE_2D, 0, 0);
    }
   
    GLContext* pContext = GLContext::getCurrent();
    if (pContext) {
        pContext->returnFBOToCache(m_FBO);
        if (m_MultisampleSamples > 1) {
            glproc::DeleteRenderbuffers(1, &m_ColorBuffer);
            pContext->returnFBOToCache(m_OutputFBO);
        }
        if (m_bUsePackedDepthStencil && isPackedDepthStencilSupported()) {
            glproc::DeleteRenderbuffers(1, &m_StencilBuffer);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                    GL_RENDERBUFFER, 0);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER, 0);
            if (m_MultisampleSamples > 1) {
                glproc::BindFramebuffer(GL_FRAMEBUFFER, m_OutputFBO);
                glproc::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, 0, 0);
            }
        } else if (m_bUseStencil) {
            glproc::DeleteRenderbuffers(1, &m_StencilBuffer);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER, 0);
        }
        glproc::BindFramebuffer(GL_FRAMEBUFFER, oldFBOID);
        GLContext::checkError("~FBO");
    }
}

void FBO::activate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    GLContext::checkError("FBO::activate: BindFramebuffer()");
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
#ifndef AVG_ENABLE_EGL
    if (m_MultisampleSamples != 1) {
        // Copy Multisample FBO to destination fbo
        glproc::BindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_FBO);
        glproc::BindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, m_OutputFBO);
        glproc::BlitFramebuffer(0, 0, m_Size.x, m_Size.y, 0, 0, m_Size.x, m_Size.y,
                GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
    }
#endif
    if (m_bMipmap) {
        for (unsigned i=0; i< m_pTextures.size(); ++i) {
            m_pTextures[i]->generateMipmaps();
        }
    }
}

BitmapPtr FBO::getImage(int i) const
{
    GLContext* pContext = GLContext::getCurrent();
    if (pContext->getMemoryMode() == MM_PBO) {
        moveToPBO(i);
        return getImageFromPBO();
    } else {
        BitmapPtr pBmp(new Bitmap(m_Size, m_PF)); 
        glproc::BindFramebuffer(GL_FRAMEBUFFER, m_OutputFBO); 
        glReadPixels(0, 0, m_Size.x, m_Size.y, GLTexture::getGLFormat(m_PF),  
                GLTexture::getGLType(m_PF), pBmp->getPixels()); 
        GLContext::checkError("FBO::getImage ReadPixels()"); 
        return pBmp;
    }
}

void FBO::moveToPBO(int i) const
{
    AVG_ASSERT(GLContext::getCurrent()->getMemoryMode() == MM_PBO);
#ifndef AVG_ENABLE_EGL
    // Get data directly from the FBO using glReadBuffer. At least on NVidia/Linux, this 
    // is faster than reading stuff from the texture.
    copyToDestTexture();
    glproc::BindFramebuffer(GL_FRAMEBUFFER, m_OutputFBO); 
 
    m_pOutputPBO->activate(); 
    GLContext::checkError("FBO::moveToPBO BindBuffer()"); 
    glReadBuffer(GL_COLOR_ATTACHMENT0+i); 
    GLContext::checkError("FBO::moveToPBO ReadBuffer()"); 
 
    glReadPixels(0, 0, m_Size.x, m_Size.y, GLTexture::getGLFormat(m_PF),  
            GLTexture::getGLType(m_PF), 0); 
    GLContext::checkError("FBO::moveToPBO ReadPixels()");     
#endif
}
 
BitmapPtr FBO::getImageFromPBO() const
{
#ifdef AVG_ENABLE_EGL
    AVG_ASSERT(false);
    return BitmapPtr();
#else
    AVG_ASSERT(GLContext::getCurrent()->getMemoryMode() == MM_PBO);
    m_pOutputPBO->activate(); 
    GLContext::checkError("FBO::getImageFromPBO BindBuffer()"); 

    BitmapPtr pBmp(new Bitmap(m_Size, m_PF)); 
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY); 
    GLContext::checkError("FBO::getImageFromPBO MapBuffer()"); 
    Bitmap PBOBitmap(m_Size, m_PF, (unsigned char *)pPBOPixels,  
            m_Size.x*getBytesPerPixel(m_PF), false); 
    pBmp->copyPixels(PBOBitmap); 
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT); 
    GLContext::checkError("FBO::getImageFromPBO UnmapBuffer()"); 
    return pBmp; 
#endif
}

GLTexturePtr FBO::getTex(int i) const
{
    return m_pTextures[i];
}

const IntPoint& FBO::getSize() const
{
    return m_Size;
}

unsigned FBO::getID() const
{
    return m_FBO;
}

void FBO::init()
{
    GLContext* pContext = GLContext::getCurrent();
    if (m_bUsePackedDepthStencil && !isPackedDepthStencilSupported()) {
        throw Exception(AVG_ERR_UNSUPPORTED, "OpenGL implementation does not support offscreen cropping (GL_EXT_packed_depth_stencil).");
    }
    if (m_MultisampleSamples > 1 && !isMultisampleFBOSupported()) {
        throw Exception(AVG_ERR_UNSUPPORTED, "OpenGL implementation does not support multisample offscreen rendering (GL_EXT_framebuffer_multisample).");
    }
#ifndef AVG_ENABLE_EGL
    if (GLContext::getCurrent()->getMemoryMode() == MM_PBO) {
        m_pOutputPBO = PBOPtr(new PBO(m_Size, m_PF, GL_STREAM_READ));
    }
#endif

    m_FBO = pContext->genFBO();
    GLContext::checkError("FBO::init: GenFramebuffers()");

    glproc::BindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    GLContext::checkError("FBO::init: BindFramebuffer()");

    IntPoint glSize = m_pTextures[0]->getGLSize();
    if (m_MultisampleSamples == 1) {
        for (unsigned i=0; i<m_pTextures.size(); ++i) {
            glproc::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, 
                    GL_TEXTURE_2D, m_pTextures[i]->getID(), 0);
            GLContext::checkError("FBO: glFramebufferTexture2D()");
        }
        if (m_bUsePackedDepthStencil) {
            glproc::GenRenderbuffers(1, &m_StencilBuffer);
            glproc::BindRenderbuffer(GL_RENDERBUFFER, m_StencilBuffer);
            glproc::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL_EXT, 
                    glSize.x, glSize.y);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                    GL_RENDERBUFFER, m_StencilBuffer);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER, m_StencilBuffer);
            GLContext::checkError("FBO::init: FramebufferRenderbuffer(DEPTH_STENCIL)");
        } else if (m_bUseStencil) {
            glproc::GenRenderbuffers(1, &m_StencilBuffer);
            glproc::BindRenderbuffer(GL_RENDERBUFFER, m_StencilBuffer);
            glproc::RenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 
                    glSize.x, glSize.y);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER, m_StencilBuffer);
            GLContext::checkError("FBO::init: FramebufferRenderbuffer(STENCIL)");
        }
        m_OutputFBO = m_FBO;
    } else {
#ifdef AVG_ENABLE_EGL
        AVG_ASSERT(false);
#else        
        glproc::GenRenderbuffers(1, &m_ColorBuffer);
        glproc::BindRenderbuffer(GL_RENDERBUFFER, m_ColorBuffer);
        GLContext::enableErrorLog(false);
        glproc::RenderbufferStorageMultisample(GL_RENDERBUFFER, m_MultisampleSamples,
                GL_RGBA8, glSize.x, glSize.y);
        GLContext::enableErrorLog(true);
        GLenum err = glGetError();
        if (err == GL_INVALID_VALUE) {
            glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
            glproc::DeleteFramebuffers(1, &m_FBO);
            glproc::DeleteRenderbuffers(1, &m_ColorBuffer);
            m_pOutputPBO = PBOPtr();
            throwMultisampleError();
        }
        GLContext::checkError("FBO::init: RenderbufferStorageMultisample");
        glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_RENDERBUFFER, m_ColorBuffer);
        GLContext::checkError("FBO::init: FramebufferRenderbuffer");
        if (m_bUsePackedDepthStencil) {
            glproc::GenRenderbuffers(1, &m_StencilBuffer);
            glproc::BindRenderbuffer(GL_RENDERBUFFER, m_StencilBuffer);
            glproc::RenderbufferStorageMultisample(GL_RENDERBUFFER, 
                    m_MultisampleSamples, GL_DEPTH_STENCIL_EXT, glSize.x, glSize.y);
            GLenum err = glGetError();
            if (err == GL_INVALID_OPERATION) {
                glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
                glproc::DeleteFramebuffers(1, &m_FBO);
                glproc::DeleteRenderbuffers(1, &m_ColorBuffer);
                m_pOutputPBO = PBOPtr();
                throwMultisampleError();
            }
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                    GL_RENDERBUFFER, m_StencilBuffer);
            glproc::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER, m_StencilBuffer);
            GLContext::checkError("FBO::init: FramebufferRenderbuffer(STENCIL)");
        } else {
            AVG_ASSERT_MSG(!m_bUseStencil, 
                "Multisample FBO with stencil & not depth buffers not implemented yet.");
        }
        checkError("init multisample");
        m_OutputFBO = pContext->genFBO();
        glproc::BindFramebuffer(GL_FRAMEBUFFER, m_OutputFBO);
        glproc::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_2D, m_pTextures[0]->getID(), 0);

        GLContext::checkError("FBO::init: Multisample init");
#endif
    }

    checkError("init");
    glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::throwMultisampleError()
{
    throw(Exception(AVG_ERR_UNSUPPORTED, 
                string("Unsupported value for number of multisample samples (")
                + toString(m_MultisampleSamples) + ")."));
}

bool FBO::isFBOSupported()
{
    return GLContext::getCurrent()->isGLES() || 
            queryOGLExtension("GL_EXT_framebuffer_object");
}

bool FBO::isMultisampleFBOSupported()
{
#ifdef AVG_ENABLE_EGL
    return false;
#else
    int maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        return false;
    }
    return queryOGLExtension("GL_EXT_framebuffer_multisample") && 
            queryOGLExtension("GL_EXT_framebuffer_blit") && maxSamples > 1;
#endif
}

bool FBO::isPackedDepthStencilSupported()
{
    return queryOGLExtension("GL_EXT_packed_depth_stencil") || 
            queryOGLExtension("GL_OES_packed_depth_stencil");
}

void FBO::checkError(const string& sContext)
{
    GLenum status = glproc::CheckFramebufferStatus(GL_FRAMEBUFFER);
    string sErr;
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            return;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            sErr = "GL_FRAMEBUFFER_UNSUPPORTED";
            throw Exception(AVG_ERR_UNSUPPORTED, string("Framebuffer error: ")+sErr);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            sErr = "GL_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            sErr = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
            break;
#ifndef AVG_ENABLE_EGL
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
#endif
        default:
            sErr = "Unknown error";
            break;
    }
    cerr << "Framebuffer error (" << sContext << "): " << sErr << endl;
    AVG_ASSERT(false);
}

}


