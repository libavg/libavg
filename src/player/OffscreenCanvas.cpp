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

#include "OffscreenCanvas.h"

#include "SDLDisplayEngine.h"
#include "CanvasNode.h"
#include "OGLTexture.h"
#include "Player.h"

#include "../base/Exception.h"
#include "../base/ProfilingZone.h"

#include <iostream>

using namespace boost;
using namespace std;

namespace avg {
    
OffscreenCanvas::OffscreenCanvas(Player * pPlayer)
    : Canvas(pPlayer)
{
}

OffscreenCanvas::~OffscreenCanvas()
{
    if (isRunning()) {
        glDeleteTextures(1, &m_TexID);
    }
}

void OffscreenCanvas::setRoot(NodePtr pRootNode)
{
    Canvas::setRoot(pRootNode);
    if (!getRootNode()) {
        throw (Exception(AVG_ERR_XML_PARSE,
                    "Root node of a canvas tree needs to be a <canvas> node."));
    }
}

void OffscreenCanvas::initPlayback(SDLDisplayEngine* pDisplayEngine, 
        AudioEngine* pAudioEngine)
{
    Canvas::initPlayback(pDisplayEngine, pAudioEngine, getMultiSampleSamples());
    m_bUseMipmaps = getMipmap();
    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenCanvas::initPlayback: glGenTextures()");
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenCanvas::initPlayback: glBindTexture()");
    createFBO();
    glEnable(GL_STENCIL_TEST);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void OffscreenCanvas::stopPlayback()
{
    m_pFBO = FBOPtr();
    glDeleteTextures(1, &m_TexID);
    Canvas::stopPlayback();
}

static ProfilingZone OffscreenRenderProfilingZone("Render OffscreenCanvas");

void OffscreenCanvas::render()
{
    if (!isRunning()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "OffscreenCanvas::screenshot(): Player.play() needs to be called before rendering offscreen canvases."));
    }
    m_pFBO->activate();
    Canvas::render(IntPoint(getRootNode()->getSize()), true, OffscreenRenderProfilingZone);
    m_pFBO->copyToDestTexture();
    m_pFBO->deactivate();
    if (m_bUseMipmaps) {
        glBindTexture(GL_TEXTURE_2D, m_TexID);
        glproc::GenerateMipmap(GL_TEXTURE_2D);
    }
}

BitmapPtr OffscreenCanvas::screenshot() const
{
    if (!isRunning()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "OffscreenCanvas::screenshot(): Canvas is not being rendered. No screenshot available."));
    }
    OGLTexturePtr pTex(new OGLTexture(getSize(), B8G8R8X8, 
            MaterialInfo(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true), 
            getDisplayEngine(), PBO));
    pTex->setTexID(m_TexID);
    return pTex->readbackBmp();
}

bool OffscreenCanvas::getHandleEvents() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(getRootNode())->getHandleEvents();
}

int OffscreenCanvas::getMultiSampleSamples() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(
            getRootNode())->getMultiSampleSamples();
}

bool OffscreenCanvas::getMipmap() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(getRootNode())->getMipmap();
}

std::string OffscreenCanvas::getID() const
{
    return getRootNode()->getID();
}

bool OffscreenCanvas::isRunning() const
{
    return (m_pFBO != FBOPtr());
}

unsigned OffscreenCanvas::getTexID() const
{
    AVG_ASSERT(isRunning());
    return m_pFBO->getTexture();
}

void OffscreenCanvas::addDependentCanvas(CanvasPtr pCanvas)
{
    m_pDependentCanvases.push_back(pCanvas);
    Player::get()->newCanvasDependency(
            dynamic_pointer_cast<OffscreenCanvas>(shared_from_this()));
}

void OffscreenCanvas::removeDependentCanvas(CanvasPtr pCanvas)
{
    for (unsigned i=0; i<m_pDependentCanvases.size(); ++i) {
        if (pCanvas == m_pDependentCanvases[i]) {
            m_pDependentCanvases.erase(m_pDependentCanvases.begin()+i);
//            dump();
            return;
        }
    }
    AVG_ASSERT(false);
}

bool OffscreenCanvas::hasDependentCanvas(CanvasPtr pCanvas) const
{
    for (unsigned i=0; i<m_pDependentCanvases.size(); ++i) {
        if (pCanvas == m_pDependentCanvases[i]) {
            return true;
        }
    }
    return false;
}

unsigned OffscreenCanvas::getNumDependentCanvases() const
{
    return m_pDependentCanvases.size();
}

bool OffscreenCanvas::isMultisampleSupported()
{
    return FBO::isMultisampleFBOSupported();
}

void OffscreenCanvas::dump() const
{
    cerr << "Canvas: " << getRootNode()->getID() << endl;
    for (unsigned i=0; i<m_pDependentCanvases.size(); ++i) {
        cerr << " " << m_pDependentCanvases[i]->getRootNode()->getID() << endl;
    }
}

void OffscreenCanvas::createFBO()
{
    IntPoint size = getSize();
    if (m_bUseMipmaps) {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);    
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, size.x);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenCanvas::createFBO: glTexImage2D()");

    m_pFBO = FBOPtr(new FBO(size, R8G8B8X8, m_TexID, getMultiSampleSamples(), true,
            m_bUseMipmaps));
}

}
