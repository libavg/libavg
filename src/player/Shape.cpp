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

#include "Shape.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

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

VertexDataPtr Shape::getVertexData()
{
    return m_pVertexData;
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

void Shape::draw(const glm::mat4& transform, float opacity)
{
    bool bIsTextured = (m_pGPUImage->getSource() != GPUImage::NONE);
    GLContext* pContext = GLContext::getCurrent();
    StandardShaderPtr pShader = pContext->getStandardShader();
    pShader->setTransform(transform);
    pShader->setAlpha(opacity);
    if (bIsTextured) {
        m_pSurface->activate();
        pShader->activate();
    } else {
        pShader->setUntextured();
        pShader->activate();
    }
    m_SubVA.draw();
}

void Shape::discard()
{
    m_pVertexData->reset();
    m_pGPUImage->setEmpty();
}

}
