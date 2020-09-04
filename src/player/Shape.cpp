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

#include "Shape.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/Triangle.h"
#include "../base/Rect.h"

#include "../graphics/Filterfliprgb.h"
#include "../graphics/GLContext.h"
#include "../graphics/OGLShader.h"
#include "../graphics/StandardShader.h"
#include "../graphics/Bitmap.h"

#include "OGLSurface.h"
#include "GPUImage.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Shape::Shape(const WrapMode& wrapMode, bool bUseMipmaps)
{
    m_pSurface = new OGLSurface(wrapMode);
    m_pGPUImage = GPUImagePtr(new GPUImage(m_pSurface, bUseMipmaps));
    m_pVertexData = VertexDataPtr(new VertexData());
}

Shape::~Shape()
{
    delete m_pSurface;
}

void Shape::setBitmap(BitmapPtr pBmp)
{
    if (pBmp) {
        m_pGPUImage->setBitmap(pBmp);
    } else {
        m_pGPUImage->setEmpty();
    }
}

void Shape::moveToGPU()
{
    m_pGPUImage->moveToGPU();
}

void Shape::moveToCPU()
{
    m_pGPUImage->moveToCPU();
}

GPUImagePtr Shape::getGPUImage()
{
    return m_pGPUImage;
}

void Shape::setVertexData(VertexDataPtr pVertexData)
{
    m_pVertexData = pVertexData;
    m_Bounds = m_pVertexData->calcBoundingRect();
}

void Shape::setVertexArray(const VertexArrayPtr& pVA)
{
    pVA->startSubVA(m_SubVA);
    m_SubVA.appendVertexData(m_pVertexData);
/*
    cerr << endl;
    cerr << "Global VA: " << endl;
    pVA->dump();
    cerr << "Local vertex data: " << endl;
    m_pVertexData->dump();
*/
}

void Shape::draw(GLContext* pContext, const glm::mat4& transform, float opacity)
{
    bool bIsTextured = (m_pGPUImage->getSource() != GPUImage::NONE);
    StandardShader* pShader = pContext->getStandardShader();
    pShader->setTransform(transform);
    pShader->setAlpha(opacity);
    if (bIsTextured) {
        m_pSurface->activate(pContext);
        pShader->activate();
    } else {
        pShader->setUntextured();
        pShader->activate();
    }
    m_SubVA.draw();
}

bool Shape::isPtInside(const glm::vec2& pos)
{
    if (!m_Bounds.contains(pos)) {
        return false;
    }
    const Vertex* pVertexes = m_pVertexData->getVertexPointer();
    const GL_INDEX_TYPE* pIndexes = m_pVertexData->getIndexPointer();
    for (int i=0; i<m_pVertexData->getNumIndexes(); i+=3) {
        const GLfloat* pPos0 = pVertexes[pIndexes[i]].m_Pos;
        const GLfloat* pPos1 = pVertexes[pIndexes[i+1]].m_Pos;
        const GLfloat* pPos2 = pVertexes[pIndexes[i+2]].m_Pos;
        Triangle tri(glm::vec2(pPos0[0], pPos0[1]), glm::vec2(pPos1[0], pPos1[1]),
                glm::vec2(pPos2[0], pPos2[1]));
        if (tri.isInside(pos)) {
            return true;
        }
    }
    return false;
}

void Shape::discard()
{
    m_pVertexData->reset();
    m_pGPUImage->setEmpty();
}

}
