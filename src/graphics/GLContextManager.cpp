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

#include "GLTexture.h"
#include "MCTexture.h"
#include "VertexArray.h"
#include "MCFBO.h"


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

MCTexturePtr GLContextManager::createTexture(const IntPoint& size, PixelFormat pf, 
        bool bMipmap, unsigned wrapSMode, unsigned wrapTMode, 
        bool bForcePOT, int potBorderColor)
{
    MCTexturePtr pTex(new MCTexture(size, pf, bMipmap, wrapSMode, wrapTMode, bForcePOT,
            potBorderColor));
    m_pPendingTexCreates.push_back(pTex);
    return pTex;
}

GLTexturePtr GLContextManager::createGLTexture(const IntPoint& size, PixelFormat pf, 
        bool bMipmap, unsigned wrapSMode, unsigned wrapTMode, 
        bool bForcePOT, int potBorderColor)
{
    GLTexturePtr pTex(new GLTexture(size, pf, bMipmap, wrapSMode, wrapTMode, bForcePOT,
            potBorderColor));
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

GLTexturePtr GLContextManager::createGLTextureFromBmp(BitmapPtr pBmp, bool bMipmap,
        unsigned wrapSMode, unsigned wrapTMode, bool bForcePOT, int potBorderColor)
{
    GLTexturePtr pTex = createGLTexture(pBmp->getSize(), pBmp->getPixelFormat(), bMipmap,
            wrapSMode, wrapTMode, bForcePOT, potBorderColor);
    pTex->moveBmpToTexture(pBmp);
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
    for (unsigned i=0; i<m_PendingBufferDeletes.size(); ++i) {
        glproc::DeleteBuffers(1, &m_PendingBufferDeletes[i][pContext]);
        GLContext::checkError("GLContextManager: delete buffers");
    }

    for (unsigned i=0; i<m_pPendingVACreates.size(); ++i) {
        m_pPendingVACreates[i]->init();
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
}

void GLContextManager::reset()
{
    m_pPendingTexCreates.clear();
    m_pPendingTexUploads.clear();
    m_PendingTexDeletes.clear();

    m_pPendingFBOCreates.clear();

    m_pPendingVACreates.clear();
    m_PendingBufferDeletes.clear();
}

}
