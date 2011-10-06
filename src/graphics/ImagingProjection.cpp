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

#include "../base/Exception.h"

namespace avg {

ImagingProjection::ImagingProjection(IntPoint size)
    : m_pVA(new VertexArray)
{
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

void ImagingProjection::activate()
{
    IntPoint destSize = m_DestRect.size();
    glViewport(0, 0, destSize.x, destSize.y);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, destSize.x, 0, destSize.y);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    IntPoint offset = m_DestRect.tl;
    glTranslated(-offset.x, -offset.y, 0);
    glScaled(m_SrcSize.x, m_SrcSize.y, 1);
    
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "ImagingProjection::activate()");
}

void ImagingProjection::draw()
{
    m_pVA->draw();
}

void ImagingProjection::init(IntPoint srcSize, IntRect destRect)
{
    m_SrcSize = srcSize;
    m_DestRect = destRect;
    DRect dest = destRect;
    DPoint p1 = DPoint(dest.tl.x/srcSize.x, dest.tl.y/srcSize.y);
    DPoint p3 = DPoint(dest.br.x/srcSize.x, dest.br.y/srcSize.y);
    DPoint p2 = DPoint(p1.x, p3.y);
    DPoint p4 = DPoint(p3.x, p1.y);
    m_pVA->reset();
    m_pVA->appendPos(p1, p1);
    m_pVA->appendPos(p2, p2);
    m_pVA->appendPos(p3, p3);
    m_pVA->appendPos(p4, p4);
    m_pVA->appendQuadIndexes(1,0,2,3);
}

}

