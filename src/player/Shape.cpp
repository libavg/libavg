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

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Shape::Shape(const string& sFilename, int texWrapSMode, int texWrapTMode)
{
    m_pSurface = new OGLSurface(texWrapSMode, texWrapTMode);
    m_pImage = ImagePtr(new Image(m_pSurface, sFilename, false));
}

Shape::~Shape()
{
    delete m_pSurface;
}

void Shape::setBitmap(const Bitmap* pBmp)
{
    Image::State prevState = m_pImage->getState();
    if (pBmp) {
        m_pImage->setBitmap(pBmp);
    } else {
        m_pImage->setFilename("");
    }
    if (m_pImage->getState() == Image::GPU) {
        downloadTexture();
        if (prevState != Image::GPU) {
            m_pVertexArray = VertexArrayPtr(new VertexArray(0, 0, 100, 100));
        }
    }
}

void Shape::moveToGPU(SDLDisplayEngine* pEngine)
{
    m_pImage->moveToGPU(pEngine);
    if (m_pImage->getState() == Image::GPU) {
        downloadTexture();
    }
    m_pVertexArray = VertexArrayPtr(new VertexArray(0, 0, 100, 100));
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

VertexArrayPtr Shape::getVertexArray()
{
    return m_pVertexArray;
}

void Shape::draw()
{
    bool bIsTextured = (m_pImage->getState() == Image::GPU);
    if (bIsTextured) {
        m_pImage->getTexture()->activate();
    }
    m_pImage->getEngine()->enableTexture(bIsTextured);
    m_pImage->getEngine()->enableGLColorArray(!bIsTextured);
    m_pVertexArray->draw();
    if (bIsTextured) {
        m_pImage->getTexture()->deactivate();
    }
}

void Shape::downloadTexture()
{
    OGLSurface * pSurface = m_pImage->getSurface();
    if (pSurface->getMemMode() == PBO) {
        pSurface->bindPBO();
    }
    pSurface->downloadTexture();
    if (pSurface->getMemMode() == PBO) {
        pSurface->unbindPBO();
    }
}

}
