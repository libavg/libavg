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

#include "VectorNode.h"

#include "NodeDefinition.h"
#include "SDLDisplayEngine.h"
#include "OGLSurface.h"
#include "Image.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"
#include "../base/WideLine.h"

#include "../graphics/VertexArray.h"
#include "../graphics/Filterfliprgb.h"

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition VectorNode::createDefinition()
{
    return NodeDefinition("vector")
        .extendDefinition(Node::createDefinition())
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(VectorNode, m_sColorName)))
        .addArg(Arg<double>("strokewidth", 1, false, offsetof(VectorNode, m_StrokeWidth)))
        .addArg(Arg<string>("texhref", "", false, offsetof(VectorNode, m_TexHRef)))
        ;
}

VectorNode::VectorNode(const ArgList& Args)
    : m_pImage(new Image("", true))
{
    m_TexHRef = Args.getArgVal<string>("texhref"); 
    setTexHRef(m_TexHRef);
}

VectorNode::~VectorNode()
{
}

void VectorNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    setDrawNeeded(true);
    m_Color = colorStringToColor(m_sColorName);
    m_pImage->moveToGPU(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    Node::setRenderingEngines(pDisplayEngine, pAudioEngine);
    if (isTextured()) {
        downloadTexture();
    }

    m_pVertexArray = VertexArrayPtr(new VertexArray(getNumVertexes(), getNumIndexes(),
            100, 100));
    if (getNumFillVertexes() != 0) {
            m_pFillVertexArray = VertexArrayPtr(new VertexArray(
                    getNumFillVertexes(), getNumFillIndexes(), 100, 100));
    }
    m_OldOpacity = -1;
}

void VectorNode::connect()
{
    Node::connect();
    checkReload();
}

void VectorNode::disconnect()
{
    m_pVertexArray = VertexArrayPtr();
    m_pFillVertexArray = VertexArrayPtr();
    m_pImage->moveToCPU();
    deleteTexture();

    Node::disconnect();
}

const std::string& VectorNode::getTexHRef() const
{
    return m_TexHRef;
}

void VectorNode::setTexHRef(const string& href)
{
    m_TexHRef = href;
    checkReload();
}

static ProfilingZone PrerenderProfilingZone("VectorNode::prerender");
static ProfilingZone VAProfilingZone("VectorNode::update VA");
static ProfilingZone VASizeProfilingZone("VectorNode::resize VA");

void VectorNode::preRender()
{
    ScopeTimer Timer(PrerenderProfilingZone);
    double curOpacity = getEffectiveOpacity();

    if (m_bVASizeChanged) {
        ScopeTimer Timer(VASizeProfilingZone);
        m_pVertexArray->changeSize(getNumVertexes(), getNumIndexes());
        if (getNumFillVertexes() != 0) {
            m_pFillVertexArray->changeSize(getNumFillVertexes(), getNumFillIndexes());
        }
        m_bVASizeChanged = false;
    }
    {
        ScopeTimer Timer(VAProfilingZone);
        if (m_bDrawNeeded || curOpacity != m_OldOpacity) {
            m_pVertexArray->reset();
            if (getNumFillVertexes() != 0) {
                m_pFillVertexArray->reset();
            }
            calcVertexes(m_pVertexArray, m_pFillVertexArray, curOpacity);
            m_pVertexArray->update();
            if (getNumFillVertexes() != 0) {
                m_pFillVertexArray->update();
            }
            m_bDrawNeeded = false;
            m_OldOpacity = curOpacity;
        }
    }
    
}

void VectorNode::maybeRender(const DRect& Rect)
{
    assert(getState() == NS_CANRENDER);
    if (getEffectiveOpacity() > 0.01) {
        if (getID() != "") {
            AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                    " with ID " << getID());
        } else {
            AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
        }
        render(Rect);
    }
}

static ProfilingZone RenderProfilingZone("VectorNode::render");

void VectorNode::render(const DRect& rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    SDLDisplayEngine * pEngine = getDisplayEngine();
    if (getNumFillVertexes() != 0) {
        pEngine->enableTexture(false);
        pEngine->enableGLColorArray(true);
        m_pFillVertexArray->draw();
    }
    if (isTextured()) {
        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_TexID);
    }
    pEngine->enableTexture(isTextured());
    pEngine->enableGLColorArray(!isTextured());
    m_pVertexArray->draw();
}

int VectorNode::getNumFillVertexes()
{
    return 0;
}

int VectorNode::getNumFillIndexes()
{
    return 0;
}

void VectorNode::checkReload()
{
    bool bOldImageExists = isTextured();
    string sLastFilename = m_pImage->getFilename();
    Node::checkReload(m_TexHRef, m_pImage);
    if (m_pImage->getState() == Image::GPU && 
        (sLastFilename != m_pImage->getFilename() || !bOldImageExists))
    {
        downloadTexture();
    }
}

void VectorNode::setColor(const string& sColor)
{
    if (m_sColorName != sColor) {
        m_sColorName = sColor;
        m_Color = colorStringToColor(m_sColorName);
        m_bDrawNeeded = true;
    }
}

const string& VectorNode::getColor() const
{
    return m_sColorName;
}

void VectorNode::setStrokeWidth(double width)
{
    if (width != m_StrokeWidth) {
        m_bDrawNeeded = true;
        m_StrokeWidth = width;
    }
}

double VectorNode::getStrokeWidth() const
{
    return m_StrokeWidth;
}

Pixel32 VectorNode::getColorVal() const
{
    return m_Color;
}

void VectorNode::updateLineData(VertexArrayPtr& pVertexArray, double opacity,
        const DPoint& p1, const DPoint& p2, double TC1, double TC2)
{
    Pixel32 color = getColorVal();
    color.setA((unsigned char)(opacity*255));

    WideLine wl(p1, p2, getStrokeWidth());
    int curVertex = pVertexArray->getCurVert();
    pVertexArray->appendPos(wl.pl0, DPoint(TC1, 1), color);
    pVertexArray->appendPos(wl.pr0, DPoint(TC1, 0), color);
    pVertexArray->appendPos(wl.pl1, DPoint(TC2, 1), color);
    pVertexArray->appendPos(wl.pr1, DPoint(TC2, 0), color);
    pVertexArray->appendQuadIndexes(curVertex+1, curVertex, curVertex+3, curVertex+2); 
}
     
void VectorNode::setDrawNeeded(bool bSizeChanged)
{
    m_bDrawNeeded = true;
    if (bSizeChanged) {
        m_bVASizeChanged = true;
    }
}
        
bool VectorNode::isDrawNeeded()
{
    return m_bDrawNeeded;
}

void VectorNode::downloadTexture()
{
    PixelFormat pf = m_pImage->getPixelFormat();
    IntPoint size = m_pImage->getSize();

    SDLDisplayEngine* pEngine = getDisplayEngine();
    m_TexID = pEngine->createTexture(size, pf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "VectorNode::downloadTexture: glTexParameteri()");
    
    OGLSurface* pSurface = m_pImage->getSurface();
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

void VectorNode::deleteTexture()
{
    glDeleteTextures(1, &m_TexID);
}

bool VectorNode::isTextured() const
{
    return (m_pImage->getState() != Image::NOT_AVAILABLE);
}

}
