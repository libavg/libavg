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
      m_TexID(-1),
      m_TexWrapSMode(texWrapSMode),
      m_TexWrapTMode(texWrapTMode)
{
}

Shape::~Shape()
{
    deleteTexture();
}

void Shape::setBitmap(const Bitmap* pBmp)
{
    deleteTexture();
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
        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_TexID);
    }
    getEngine()->enableTexture(bIsTextured);
    getEngine()->enableGLColorArray(!bIsTextured);
    m_pVertexArray->draw();
}

void Shape::downloadTexture()
{
    PixelFormat pf = getPixelFormat();
    IntPoint size = getSize();
    SDLDisplayEngine* pEngine = getEngine();
   
    m_TexID = pEngine->createTexture(size, pf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_TexWrapSMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_TexWrapTMode);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "Shape::downloadTexture: glTexParameteri()");
    
    OGLSurface* pSurface = getSurface();
    unsigned char * pPixels;
    if (pSurface->getMemMode() == OGL) {
        pPixels = pSurface->getBmp()->getPixels();
    } else {
        pPixels = 0;
        pSurface->bindPBO();
    }
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "VectorNode::downloadTexture: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y,
            pEngine->getOGLSrcMode(pf), pEngine->getOGLPixelType(pf), pPixels);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "VectorNode::downloadTexture: glTexSubImage2D()");
    if (pSurface->getMemMode() == PBO) {
        pSurface->unbindPBO();
    }
}

void Shape::deleteTexture()
{
    if (m_TexID != -1) {
        glDeleteTextures(1, &m_TexID);
    }
}


}
