//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
#include "Filterfliprgb.h"

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
    FBOMap::iterator it;
    for (it=m_pFBOs.begin(); it!=m_pFBOs.end(); ++it) {
        GLContext* pContext = it->first;
        FBOPtr pFBO = it->second;
        pContext->activate();
        m_pFBOs[pContext] = FBOPtr();
    }
}

void MCFBO::initForGLContext()
{
    GLContext* pContext = GLContext::getCurrent();

    vector<GLTexturePtr> pTextures;
    for (unsigned i=0; i<m_pTextures.size(); ++i) {
        MCTexturePtr pTex = m_pTextures[i];
        pTex->initForGLContext();
        pTex->generateMipmaps();
        GLContext::checkError("MCFBO::initForGLContext: generateMipmaps");
        pTextures.push_back(pTex->getCurTex());
    }

    AVG_ASSERT(m_pFBOs.count(pContext) == 0);
    m_pFBOs[pContext] = FBOPtr(new FBO(*this, pTextures));
}

void MCFBO::activate() const
{
    getCurFBO()->activate();
}

void MCFBO::copyToDestTexture() const
{
    getCurFBO()->copyToDestTexture();
}

BitmapPtr MCFBO::getImage(int i) const
{
    return getCurFBO()->getImage(i);
}

void MCFBO::moveToPBO(int i) const
{
    getCurFBO()->moveToPBO(i);
}
 
BitmapPtr MCFBO::getImageFromPBO() const
{
    return getCurFBO()->getImageFromPBO();
}

MCTexturePtr MCFBO::getTex(int i) const
{
    return m_pTextures[i];
}

FBOPtr MCFBO::getCurFBO() const
{
    FBOMap::const_iterator it = m_pFBOs.find(GLContext::getCurrent());
    AVG_ASSERT(it != m_pFBOs.end());
    return it->second;
}

}


