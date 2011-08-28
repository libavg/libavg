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

#include "Shape.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "../graphics/Filterfliprgb.h"

#include "OGLSurface.h"
#include "SDLDisplayEngine.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Shape::Shape(const MaterialInfo& material)
{
    m_pSurface = new OGLSurface(material);
    m_pImage = ImagePtr(new Image(m_pSurface));
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
        m_pSurface->downloadTexture();
        if (prevState != Image::GPU) {
            // TODO: This shouldn't happen.
            m_pVertexArray = VertexArrayPtr(new VertexArray(100, 100));
        }
    }
}

void Shape::moveToGPU(SDLDisplayEngine* pEngine)
{
    m_pSurface->attach(pEngine);
    m_pImage->moveToGPU(pEngine);
    if (m_pImage->getSource() != Image::NONE) {
        m_pSurface->downloadTexture();
    }
    m_pVertexArray = VertexArrayPtr(new VertexArray(100, 100));
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

void Shape::draw()
{
    bool bIsTextured = isTextured();
    SDLDisplayEngine* pEngine = m_pImage->getEngine();
    if (bIsTextured) {
        m_pSurface->activate();
    } else {
        if (pEngine->isUsingShaders()) {
            glproc::UseProgramObject(0);
        }
        for (int i = 1; i < 5; ++i) {
            glproc::ActiveTexture(GL_TEXTURE0 + i);
            glDisable(GL_TEXTURE_2D);
        }
        glproc::ActiveTexture(GL_TEXTURE0);
    }
    pEngine->enableTexture(bIsTextured);
    pEngine->enableGLColorArray(!bIsTextured);
    m_pVertexArray->draw();
}

void Shape::discard()
{
    m_pVertexArray = VertexArrayPtr();
    m_pImage->discard();
}

}
