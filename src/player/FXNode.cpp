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
#include "Player.h"

#include "../base/ObjectCounter.h"
#include "../graphics/GLContext.h"

namespace avg {

using namespace std;

FXNode::FXNode(bool bSupportsGLES) 
    : m_Size(0, 0),
      m_bSupportsGLES(bSupportsGLES),
      m_bDirty(true)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

FXNode::~FXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void FXNode::connect()
{
    checkGLES();
    if (m_Size != IntPoint(0,0)) {
        m_pFilter = createFilter(m_Size);
    }
}

void FXNode::disconnect()
{
    m_pFilter = GPUFilterPtr();
}

void FXNode::setSize(const IntPoint& newSize)
{
    if (newSize != m_Size) {
        m_Size = newSize;
        if (m_pFilter) {
            m_pFilter = createFilter(m_Size);
        }
    }
}

void FXNode::apply(GLTexturePtr pSrcTex)
{
    // blt overwrites everything, so no glClear necessary before.
    GLContext::getMain()->setBlendMode(GLContext::BLEND_COPY);
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

FRect FXNode::getRelDestRect() const
{
    return m_pFilter->getRelDestRect();
}

bool FXNode::isDirty() const
{
    return m_bDirty;
}

void FXNode::resetDirty()
{
    m_bDirty = false;
}

FBOPtr FXNode::getFBO()
{
    return m_pFilter->getFBO();
}

void FXNode::setDirty()
{
    m_bDirty = true;
}

void FXNode::checkGLES() const
{
    if (!m_bSupportsGLES && GLContext::getCurrent()->isGLES()) {
        throw Exception(AVG_ERR_UNSUPPORTED, "This effect is unsupported under OpenGL ES.");
    }
}

}
