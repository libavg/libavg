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

#include "GLContextMultiplexer.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/Backtrace.h"

#include "GLTexture.h"
#include "MCTexture.h"
#include "VertexArray.h"


namespace avg {

using namespace std;

GLContextMultiplexer* GLContextMultiplexer::s_pGLContextMultiplexer = 0;

GLContextMultiplexer* GLContextMultiplexer::get()
{
    AVG_ASSERT(s_pGLContextMultiplexer);
    return s_pGLContextMultiplexer;
}

bool GLContextMultiplexer::exists()
{
    return s_pGLContextMultiplexer != 0;
}

GLContextMultiplexer::GLContextMultiplexer()
{
//    AVG_ASSERT(!s_pGLContextMultiplexer);
    s_pGLContextMultiplexer = this;
}

GLContextMultiplexer::~GLContextMultiplexer()
{
    m_pPendingTexCreates.clear();
    m_pPendingTexUploads.clear();
    m_PendingTexDeletes.clear();

    m_pPendingVACreates.clear();
    m_PendingBufferDeletes.clear();

    s_pGLContextMultiplexer = 0;
}

MCTexturePtr GLContextMultiplexer::createTexture(const IntPoint& size, PixelFormat pf, 
        bool bMipmap, int potBorderColor, unsigned wrapSMode, unsigned wrapTMode, 
        bool bForcePOT)
{
    MCTexturePtr pTex(new MCTexture(size, pf, bMipmap, potBorderColor, 
            wrapSMode, wrapTMode, bForcePOT));
    m_pPendingTexCreates.push_back(pTex);
    return pTex;
}

GLTexturePtr GLContextMultiplexer::createGLTexture(const IntPoint& size, PixelFormat pf, 
        bool bMipmap, int potBorderColor, unsigned wrapSMode, unsigned wrapTMode, 
        bool bForcePOT)
{
    GLTexturePtr pTex(new GLTexture(size, pf, bMipmap, potBorderColor, 
            wrapSMode, wrapTMode, bForcePOT));
    return pTex;
}

void GLContextMultiplexer::scheduleTexUpload(MCTexturePtr pTex, BitmapPtr pBmp)
{
    m_pPendingTexUploads[pTex] = pBmp;
}

MCTexturePtr GLContextMultiplexer::createTextureFromBmp(BitmapPtr pBmp, bool bMipmap,
        int potBorderColor, unsigned wrapSMode, unsigned wrapTMode, bool bForcePOT)
{
    MCTexturePtr pTex = createTexture(pBmp->getSize(), pBmp->getPixelFormat(), bMipmap,
            potBorderColor, wrapSMode, wrapTMode, bForcePOT);
    scheduleTexUpload(pTex, pBmp);
    return pTex;
}

GLTexturePtr GLContextMultiplexer::createGLTextureFromBmp(BitmapPtr pBmp, bool bMipmap,
        int potBorderColor, unsigned wrapSMode, unsigned wrapTMode, bool bForcePOT)
{
    GLTexturePtr pTex = createGLTexture(pBmp->getSize(), pBmp->getPixelFormat(), bMipmap,
            potBorderColor, wrapSMode, wrapTMode, bForcePOT);
    pTex->moveBmpToTexture(pBmp);
    return pTex;
}

void GLContextMultiplexer::deleteTexture(unsigned texID)
{
    m_PendingTexDeletes.push_back(texID);
}

VertexArrayPtr GLContextMultiplexer::createVertexArray(int reserveVerts,
        int reserveIndexes)
{
    VertexArrayPtr pVA(new VertexArray(reserveVerts, reserveIndexes));
    m_pPendingVACreates.push_back(pVA);
    return pVA;
}

void GLContextMultiplexer::deleteBuffers(BufferIDMap& bufferIDs)
{
    m_PendingBufferDeletes.push_back(bufferIDs);
}

void GLContextMultiplexer::uploadData()
{
    GLContext* pContext = GLContext::getCurrent();
    for (unsigned i=0; i<m_PendingBufferDeletes.size(); ++i) {
        glproc::DeleteBuffers(1, &m_PendingBufferDeletes[i][pContext]);
        GLContext::checkError("GLContextMultiplexer: delete buffers");
    }

    for (unsigned i=0; i<m_pPendingVACreates.size(); ++i) {
        m_pPendingVACreates[i]->init();
    }

    for (unsigned i=0; i<m_PendingTexDeletes.size(); ++i) {
        glDeleteTextures(1, &m_PendingTexDeletes[i]);
        GLContext::checkError("GLContextMultiplexer: delete textures");
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
}

void GLContextMultiplexer::reset()
{
    m_pPendingTexCreates.clear();
    m_pPendingTexUploads.clear();
    m_PendingTexDeletes.clear();

    m_pPendingVACreates.clear();
    m_PendingBufferDeletes.clear();
}

}
