//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#include "MCFBO.h"

#include "OGLHelper.h"
#include "GLContext.h"
#include "GLContextManager.h"
#include "Filterfliprgb.h"
#include "FBO.h"
#include "MCTexture.h"
#include "GLTexture.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/ObjectCounter.h"

#include <stdio.h>

using namespace std;
using namespace boost;

namespace avg {

MCFBO::MCFBO(const IntPoint& size, PixelFormat pf, unsigned numTextures, 
        unsigned multisampleSamples, bool bUsePackedDepthStencil, bool bUseStencil,
        bool bMipmap)
    : FBOInfo(size, pf, numTextures, multisampleSamples, bUsePackedDepthStencil,
            bUseStencil, bMipmap)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    for (unsigned i=0; i<numTextures; ++i) {
        MCTexturePtr pTex(new MCTexture(size, pf, bMipmap));
        m_pTextures.push_back(pTex);
    }
}

MCFBO::~MCFBO()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    if (GLContextManager::isActive()) {
        FBOMap::iterator it;
        for (it=m_pFBOs.begin(); it!=m_pFBOs.end(); ++it) {
            GLContext* pContext = it->first;
            FBOPtr pFBO = it->second;
            pContext->activate();
            m_pFBOs[pContext] = FBOPtr();
        }
    }
}

void MCFBO::initForGLContext()
{
    GLContext* pContext = GLContext::getCurrent();
    vector<GLTexturePtr> pTextures;
    for (unsigned i=0; i<m_pTextures.size(); ++i) {
        MCTexturePtr pMCTex = m_pTextures[i];
        pMCTex->initForGLContext(pContext);
        GLTexturePtr pTex = pMCTex->getTex(pContext);
        pTex->generateMipmaps();
        GLContext::checkError("MCFBO::initForGLContext: generateMipmaps");
        pTextures.push_back(pTex);
    }

    AVG_ASSERT(m_pFBOs.count(pContext) == 0);
    m_pFBOs[pContext] = FBOPtr(new FBO(*this, pTextures));
}

void MCFBO::activate(GLContext* pContext) const
{
    getCurFBO(pContext)->activate();
}

void MCFBO::copyToDestTexture(GLContext* pContext) const
{
    getCurFBO(pContext)->copyToDestTexture();
}

BitmapPtr MCFBO::getImage(GLContext* pContext, int i) const
{
    return getCurFBO(pContext)->getImage(i);
}

void MCFBO::moveToPBO(GLContext* pContext, int i) const
{
    getCurFBO(pContext)->moveToPBO(i);
}
 
BitmapPtr MCFBO::getImageFromPBO(GLContext* pContext) const
{
    return getCurFBO(pContext)->getImageFromPBO();
}

MCTexturePtr MCFBO::getTex(int i) const
{
    return m_pTextures[i];
}

FBOPtr MCFBO::getCurFBO(GLContext* pContext) const
{
    FBOMap::const_iterator it = m_pFBOs.find(pContext);
    AVG_ASSERT(it != m_pFBOs.end());
    return it->second;
}

}


