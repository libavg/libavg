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

#include "GLContextManager.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/Backtrace.h"
#include "../base/ScopeTimer.h"

#include "GLTexture.h"
#include "MCTexture.h"
#include "VertexArray.h"
#include "MCFBO.h"
#include "ShaderRegistry.h"

#ifdef __APPLE__
    #include "CGLContext.h"
#elif defined linux
    #ifdef AVG_ENABLE_EGL
        #include "EGLContext.h"
    #else
        #include "SDLGLXContext.h"
    #endif
#elif defined _WIN32
    #include "WGLContext.h"
#endif


namespace avg {

using namespace std;

GLContextManager* GLContextManager::s_pGLContextManager = 0;

GLContextManager* GLContextManager::get()
{
    AVG_ASSERT(s_pGLContextManager);
    return s_pGLContextManager;
}

bool GLContextManager::exists()
{
    return s_pGLContextManager != 0;
}

GLContextManager::GLContextManager()
{
//    AVG_ASSERT(!s_pGLContextManager);
    s_pGLContextManager = this;
}

GLContextManager::~GLContextManager()
{
    m_pPendingTexCreates.clear();
    m_pPendingTexUploads.clear();
    m_PendingTexDeletes.clear();

    m_pPendingFBOCreates.clear();

    m_pPendingVACreates.clear();
    m_PendingBufferDeletes.clear();

    s_pGLContextManager = 0;
}

GLContext* GLContextManager::createContext(const GLConfig& glConfig, 
            const IntPoint& windowSize, const SDL_SysWMinfo* pSDLWMInfo)
{
    if (glConfig.m_bGLES) {
        AVG_ASSERT(isGLESSupported());
    }
    GLContext* pContext;
#ifdef __APPLE__
    pContext = new CGLContext(glConfig, windowSize, pSDLWMInfo);
#elif defined linux
    #ifdef AVG_ENABLE_EGL
        GLConfig tempConfig = glConfig;
        tempConfig.m_bGLES = true;
        pContext = new EGLContext(tempConfig, windowSize, pSDLWMInfo);
    #else
        pContext = new SDLGLXContext(glConfig, windowSize, pSDLWMInfo);
    #endif
#elif defined _WIN32
    pContext = new WGLContext(glConfig, windowSize, pSDLWMInfo);
#else
    AVG_ASSERT(false);
    pContext = 0;
#endif
    return pContext;
}

void GLContextManager::registerContext(GLContext* pContext)
{
    m_pContexts.push_back(pContext);
}

void GLContextManager::unregisterContext(GLContext* pContext)
{
    vector<GLContext*>::iterator it;
    for (it=m_pContexts.begin(); it!=m_pContexts.end(); ++it) {
        if (*it == pContext) {
            m_pContexts.erase(it);
            return;
        }
    }
    AVG_ASSERT(false);
}

MCTexturePtr GLContextManager::createTexture(const IntPoint& size, PixelFormat pf, 
        bool bMipmap, unsigned wrapSMode, unsigned wrapTMode, 
        bool bForcePOT, int potBorderColor)
{
    MCTexturePtr pTex(new MCTexture(size, pf, bMipmap, wrapSMode, wrapTMode, bForcePOT,
            potBorderColor));
    m_pPendingTexCreates.push_back(pTex);
    return pTex;
}

MCFBOPtr GLContextManager::createFBO(const IntPoint& size, PixelFormat pf, 
        unsigned numTextures, unsigned multisampleSamples, bool bUsePackedDepthStencil,
        bool bUseStencil, bool bMipmap, unsigned wrapSMode, unsigned wrapTMode)
{
    MCFBOPtr pFBO(new MCFBO(size, pf, numTextures, multisampleSamples, 
            bUsePackedDepthStencil, bUseStencil, bMipmap, wrapSMode, wrapTMode));
    m_pPendingFBOCreates.push_back(pFBO);
    return pFBO;
}

void GLContextManager::createShader(const string& sID)
{
    GLContext* pContext = GLContext::getCurrent();
    for (unsigned i=0; i<m_pContexts.size(); ++i) {
        m_pContexts[i]->activate();
        m_pContexts[i]->getShaderRegistry()->createShader(sID);
    }
    pContext->activate();
}

void GLContextManager::scheduleTexUpload(MCTexturePtr pTex, BitmapPtr pBmp)
{
    m_pPendingTexUploads[pTex] = pBmp;
}

MCTexturePtr GLContextManager::createTextureFromBmp(BitmapPtr pBmp, bool bMipmap,
        unsigned wrapSMode, unsigned wrapTMode, bool bForcePOT, int potBorderColor)
{
    MCTexturePtr pTex = createTexture(pBmp->getSize(), pBmp->getPixelFormat(), bMipmap,
            wrapSMode, wrapTMode, bForcePOT, potBorderColor);
    scheduleTexUpload(pTex, pBmp);
    return pTex;
}

void GLContextManager::deleteTexture(unsigned texID)
{
    m_PendingTexDeletes.push_back(texID);
}

VertexArrayPtr GLContextManager::createVertexArray(int reserveVerts,
        int reserveIndexes)
{
    VertexArrayPtr pVA(new VertexArray(reserveVerts, reserveIndexes));
    m_pPendingVACreates.push_back(pVA);
    return pVA;
}

void GLContextManager::deleteBuffers(BufferIDMap& bufferIDs)
{
    m_PendingBufferDeletes.push_back(bufferIDs);
}

void GLContextManager::uploadData()
{
    GLContext* pContext = GLContext::getCurrent();
    for (unsigned i=0; i<m_pContexts.size(); ++i) {
        m_pContexts[i]->activate();
        uploadDataForContext();
    }
    pContext->activate();
    reset();
}

static ProfilingZoneID UploadDataProfilingZone("uploadData");

void GLContextManager::uploadDataForContext()
{
    ScopeTimer timer(UploadDataProfilingZone);
    GLContext* pContext = GLContext::getCurrent();
    for (unsigned i=0; i<m_PendingBufferDeletes.size(); ++i) {
        glproc::DeleteBuffers(1, &m_PendingBufferDeletes[i][pContext]);
        GLContext::checkError("GLContextManager: delete buffers");
    }

    for (unsigned i=0; i<m_pPendingVACreates.size(); ++i) {
        m_pPendingVACreates[i]->initForGLContext();
    }

    for (unsigned i=0; i<m_PendingTexDeletes.size(); ++i) {
        glDeleteTextures(1, &m_PendingTexDeletes[i]);
        GLContext::checkError("GLContextManager: delete textures");
    }

    for (unsigned i=0; i<m_pPendingTexCreates.size(); ++i) {
        m_pPendingTexCreates[i]->initForGLContext();
    }

    TexUploadMap::iterator it;
    for (it=m_pPendingTexUploads.begin(); it!=m_pPendingTexUploads.end(); ++it) {
        MCTexturePtr pTex = it->first;
        BitmapPtr pBmp = it->second;
        pTex->moveBmpToTexture(pBmp);
    }

    for (unsigned i=0; i<m_pPendingFBOCreates.size(); ++i) {
        m_pPendingFBOCreates[i]->initForGLContext();
    }

    for (unsigned i=0; i<m_pPendingShaderParamCreates.size(); ++i) {
        m_pPendingShaderParamCreates[i]->initForGLContext();
    }
    
}

void GLContextManager::reset()
{
    m_pPendingTexCreates.clear();
    m_pPendingTexUploads.clear();
    m_PendingTexDeletes.clear();

    m_pPendingFBOCreates.clear();
    m_pPendingShaderParamCreates.clear();

    m_pPendingVACreates.clear();
    m_PendingBufferDeletes.clear();
}

bool GLContextManager::isGLESSupported()
{
#if defined linux
    #ifdef AVG_ENABLE_EGL
    return true;
    #else
    return SDLGLXContext::haveARBCreateContext();
    #endif
#else
    return false;
#endif
}

}
