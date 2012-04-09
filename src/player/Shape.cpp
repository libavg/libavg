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

#include "Shape.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "../graphics/Filterfliprgb.h"
#include "../graphics/GLContext.h"
#include "../graphics/OGLShader.h"

#include "OGLSurface.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Shape::Shape(const MaterialInfo& material)
{
    m_pSurface = new OGLSurface();
    m_pImage = ImagePtr(new Image(m_pSurface, material));
}

Shape::~Shape()
{
    delete m_pSurface;
}

void Shape::setBitmap(BitmapPtr pBmp)
{
    Image::State prevState = m_pImage->getState();
    if (pBmp) {
        m_pImage->setBitmap(pBmp);
    } else {
        m_pImage->setEmpty();
    }
    if (m_pImage->getState() == Image::GPU) {
        if (prevState != Image::GPU) {
            // TODO: This shouldn't happen.
            m_pVertexArray = VertexArrayPtr(new VertexArray());
        }
    }
}

void Shape::moveToGPU()
{
    m_pImage->moveToGPU();
    m_pVertexArray = VertexArrayPtr(new VertexArray());
}

void Shape::moveToCPU()
{
    m_pVertexArray = VertexArrayPtr();
    m_pImage->moveToCPU();
}

ImagePtr Shape::getImage()
{
    return m_pImage;
}

bool Shape::isTextured() const
{
    return m_pImage->getSource() != Image::NONE;
}

VertexArrayPtr Shape::getVertexArray()
{
    return m_pVertexArray;
}

void Shape::draw(const glm::mat4& transform)
{
    bool bIsTextured = isTextured();
    GLContext* pContext = GLContext::getCurrent();
    if (bIsTextured) {
        m_pSurface->activate();
    } else {
        StandardShaderPtr pShader = pContext->getStandardShader();
        pShader->setUntextured();
        pShader->activate();
    }
    pContext->enableGLColorArray(!bIsTextured);
    glLoadMatrixf(glm::value_ptr(transform));
    m_pVertexArray->draw();
}

void Shape::discard()
{
    m_pVertexArray = VertexArrayPtr();
    m_pImage->discard();
}

}
