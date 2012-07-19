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

#include "ImagingProjection.h"

#include "GLContext.h"
#include "OGLShader.h"

#include "../base/Exception.h"

namespace avg {

ImagingProjection::ImagingProjection(IntPoint size)
    : m_pVA(new VertexArray){
    init(size, IntRect(IntPoint(0,0), size));
}

ImagingProjection::ImagingProjection(IntPoint srcSize, IntRect destRect)
    : m_pVA(new VertexArray)
{
    init(srcSize, destRect);
}

ImagingProjection::~ImagingProjection()
{
}

void ImagingProjection::draw(const OGLShaderPtr& pShader)
{
    IntPoint destSize = m_DestRect.size();
    glViewport(0, 0, destSize.x, destSize.y);
    pShader->setTransform(m_ProjMat); 
    m_pVA->draw();
}

void ImagingProjection::init(IntPoint srcSize, IntRect destRect)
{
    m_SrcSize = srcSize;
    m_DestRect = destRect;
    FRect dest = destRect;
    glm::vec2 p1(dest.tl.x/srcSize.x, dest.tl.y/srcSize.y);
    glm::vec2 p3(dest.br.x/srcSize.x, dest.br.y/srcSize.y);
    glm::vec2 p2(p1.x, p3.y);
    glm::vec2 p4(p3.x, p1.y);
    m_pVA->reset();
    m_pVA->appendPos(p1, p1);
    m_pVA->appendPos(p2, p2);
    m_pVA->appendPos(p3, p3);
    m_pVA->appendPos(p4, p4);
    m_pVA->appendQuadIndexes(1,0,2,3);
    
    IntPoint destSize = m_DestRect.size();
    glm::mat4 projMat(glm::ortho(0.f, float(destSize.x), 0.f, float(destSize.y)));
    
    glm::vec3 offset(-m_DestRect.tl.x, -m_DestRect.tl.y, 0);
    glm::mat4 transform = glm::translate(projMat, offset);
    glm::vec3 size(m_SrcSize.x, m_SrcSize.y, 1);
    m_ProjMat = glm::scale(transform, size);
}

}

