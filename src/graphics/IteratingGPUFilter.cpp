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

#include "IteratingGPUFilter.h"

#include "OGLHelper.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {

IteratingGPUFilter::IteratingGPUFilter(const IntPoint& size, int numIterations)
    : m_NumIterations(numIterations)
{
    m_pSrcPBO = PBOImagePtr(new PBOImage(size, R32G32B32A32F, R32G32B32A32F, 
            true, false));
    m_pDestPBO = PBOImagePtr(new PBOImage(size, R32G32B32A32F, R32G32B32A32F, 
            false, false));
    vector<unsigned> texIDs;
    texIDs.push_back(m_pSrcPBO->getTexID());
    texIDs.push_back(m_pDestPBO->getTexID());
    m_pFBO = FBOPtr(new FBO(size, R32G32B32A32F, texIDs));
}

IteratingGPUFilter::~IteratingGPUFilter()
{
}

BitmapPtr IteratingGPUFilter::apply(BitmapPtr pImage)
{
    m_pSrcPBO->setImage(pImage);
    applyOnGPU();
    return m_pFBO->getImage(1);
}

void IteratingGPUFilter::applyOnGPU()
{
    for(int i=0;i<2;i++) {
        glproc::ActiveTexture(GL_TEXTURE0+i);        
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    m_pFBO->activate();
    for(int k=0; k<m_NumIterations; k++) {
        glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
        applyOnce(m_pSrcPBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        applyOnce(m_pDestPBO);
    }
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    applyOnce(m_pSrcPBO);
    
    m_pFBO->deactivate();
}

} // namespace avg

