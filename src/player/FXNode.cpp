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

#include "FXNode.h"
#include "SDLDisplayEngine.h"
#include "Player.h"

#include "../base/ObjectCounter.h"

namespace avg {

using namespace std;

FXNode::FXNode() 
    : m_pEngine(0), 
      m_Size(0, 0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

FXNode::~FXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void FXNode::connect(SDLDisplayEngine* pEngine)
{
    m_pEngine = pEngine;
    if (m_Size != IntPoint(0,0)) {
        m_pFilter = createFilter(m_Size);
    }
}

void FXNode::disconnect()
{
    m_pEngine = 0;
    m_pFilter = GPUFilterPtr();
}

void FXNode::setSize(const IntPoint& newSize)
{
    if (newSize != m_Size) {
        m_Size = newSize;
        if (m_pEngine) {
            m_pFilter = createFilter(m_Size);
        }
    }
}

void FXNode::apply(GLTexturePtr pSrcTex)
{
    // blt overwrites everything, so no glClear necessary before.
    getEngine()->setBlendMode(DisplayEngine::BLEND_COPY);
    m_pFilter->apply(pSrcTex);
}

GLTexturePtr FXNode::getTex()
{
    return m_pFilter->getDestTex();
}

BitmapPtr FXNode::getImage()
{
    return m_pFilter->getImage();
}

DRect FXNode::getRelDestRect() const
{
    return m_pFilter->getRelDestRect();
}

SDLDisplayEngine* FXNode::getEngine() const
{
    return m_pEngine;
}

FBOPtr FXNode::getFBO()
{
    return m_pFilter->getFBO();
}

}
