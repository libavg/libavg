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

#include "FXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"

namespace avg {

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
        m_pFBO = FBOPtr(new FBO(m_Size, B8G8R8A8, 1, 1, false, false));
    }
}

void FXNode::disconnect()
{
    m_pEngine = 0;
    m_pFBO = FBOPtr();
}

void FXNode::setSize(const IntPoint& newSize)
{
    if (newSize != m_Size) {
        m_Size = newSize;
        if (m_pEngine) {
            m_pFBO = FBOPtr(new FBO(m_Size, B8G8R8A8, 1, 1, false, false));
        }
    }
}

GLTexturePtr FXNode::getTex()
{
    return m_pFBO->getTex();
}

SDLDisplayEngine* FXNode::getEngine() const
{
    return m_pEngine;
}

FBOPtr FXNode::getFBO()
{
    return m_pFBO;
}

}
