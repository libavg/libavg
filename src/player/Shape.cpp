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

#include "OGLTiledSurface.h"
#include "SDLDisplayEngine.h"

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Shape::Shape(const string& sFilename, int texWrapSMode, int texWrapTMode)
    : Image(sFilename, false),
      m_TexWrapSMode(texWrapSMode),
      m_TexWrapTMode(texWrapTMode)
{
}

Shape::~Shape()
{
}

void Shape::setBitmap(const Bitmap* pBmp)
{
    State prevState = getState();
    Image::setBitmap(pBmp);
    if (getState() == GPU) {
        downloadTexture();
        if (prevState != GPU) {
            m_pVertexArray = VertexArrayPtr(new VertexArray(0, 0, 100, 100));
        }
    }
}

void Shape::moveToGPU(SDLDisplayEngine* pEngine)
{
    Image::moveToGPU(pEngine);
    if (getState() == GPU) {
        downloadTexture();
    }
    m_pVertexArray = VertexArrayPtr(new VertexArray(0, 0, 100, 100));
}

void Shape::moveToCPU()
{
    m_pVertexArray = VertexArrayPtr();
    Image::moveToCPU();
}

VertexArrayPtr Shape::getVertexArray()
{
    return m_pVertexArray;
}

void Shape::draw()
{
    bool bIsTextured = (getState() == GPU);
    if (bIsTextured) {
        m_pTexture->activate();
    }
    getEngine()->enableTexture(bIsTextured);
    getEngine()->enableGLColorArray(!bIsTextured);
    m_pVertexArray->draw();
    if (bIsTextured) {
        m_pTexture->deactivate();
    }
}

void Shape::downloadTexture()
{
    PixelFormat pf = getPixelFormat();
    IntPoint size = getSize();
    SDLDisplayEngine* pEngine = getEngine();
    m_pTexture = OGLTexturePtr(new OGLTexture(size, pf, m_TexWrapSMode, m_TexWrapTMode,
            pEngine));
    OGLSurface * pSurface = getSurface();
    if (pSurface->getMemMode() == PBO) {
        pSurface->bindPBO();
    }
    m_pTexture->downloadTexture(0, pSurface->getBmp(), pSurface->getMemMode());
    if (pSurface->getMemMode() == PBO) {
        pSurface->unbindPBO();
    }
}

}
