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

#include "GLTexture.h"


namespace avg {

using namespace std;

GLContextMultiplexer* GLContextMultiplexer::s_pGLContextMultiplexer = 0;

GLContextMultiplexer* GLContextMultiplexer::get()
{
    AVG_ASSERT(s_pGLContextMultiplexer);
    return s_pGLContextMultiplexer;
}

GLContextMultiplexer::GLContextMultiplexer()
{
//    AVG_ASSERT(!s_pGLContextMultiplexer);
    s_pGLContextMultiplexer = this;
}

GLContextMultiplexer::~GLContextMultiplexer()
{
//    s_pGLContextMultiplexer = 0;
}

GLTexturePtr GLContextMultiplexer::createTexture(const IntPoint& size, PixelFormat pf, 
        bool bMipmap, int potBorderColor, unsigned wrapSMode, unsigned wrapTMode, 
        bool bForcePOT)
{
    GLTexturePtr pTex = GLTexturePtr(new GLTexture(size, pf, bMipmap, potBorderColor, 
            wrapSMode, wrapTMode, bForcePOT, true));
    m_pPendingTexCreates.push_back(pTex);
    return pTex;
}

void GLContextMultiplexer::scheduleTexUpload(GLTexturePtr pTex, BitmapPtr pBmp)
{
    m_pPendingTexUploads[pTex] = pBmp;
}

void GLContextMultiplexer::uploadTextures()
{
    for (unsigned i=0; i<m_pPendingTexCreates.size(); ++i) {
        m_pPendingTexCreates[i]->init();
    }

    TexUploadMap::iterator it;
    for (it=m_pPendingTexUploads.begin(); it!=m_pPendingTexUploads.end(); ++it) {
        GLTexturePtr pTex = it->first;
        BitmapPtr pBmp = it->second;
        pTex->moveBmpToTexture(pBmp);
    }
}

void GLContextMultiplexer::reset()
{
    m_pPendingTexCreates.clear();
    m_pPendingTexUploads.clear();
}

}
