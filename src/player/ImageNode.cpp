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

#include "ImageNode.h"

#include "SDLDisplayEngine.h"
#include "NodeDefinition.h"
#include "OGLSurface.h"
#include "Player.h"
#include "OffscreenScene.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfliprgb.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition ImageNode::createDefinition()
{
    return NodeDefinition("image", VisibleNode::buildNode<ImageNode>)
        .extendDefinition(RasterNode::createDefinition())
        .addArg(Arg<UTF8String>("href", "", false, offsetof(ImageNode, m_href)));
}

ImageNode::ImageNode(const ArgList& Args)
{
    m_pImage = ImagePtr(new Image(getSurface()));
    Args.setMembers(this);
    setHRef(m_href);
    ObjectCounter::get()->incRef(&typeid(*this));
}

ImageNode::~ImageNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ImageNode::setRenderingEngines(DisplayEngine * pDisplayEngine,
        AudioEngine * pAudioEngine)
{
    getSurface()->attach(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    m_pImage->moveToGPU(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

void ImageNode::connect(Scene * pScene)
{
    RasterNode::connect(pScene);
    checkReload();
}

void ImageNode::disconnect(bool bKill)
{
    if (bKill) {
        m_pImage = ImagePtr(new Image(getSurface()));
        m_href = "";
    } else {
        m_pImage->moveToCPU();
    }
    RasterNode::disconnect(bKill);
}

const UTF8String& ImageNode::getHRef() const
{
    return m_href;
}

void ImageNode::setHRef(const UTF8String& href)
{
    m_href = href;
    checkReload();
}

void ImageNode::setBitmap(const Bitmap * pBmp)
{
    m_pImage->setBitmap(pBmp);
    if (getState() == VisibleNode::NS_CANRENDER) {
        bind();
    }
    m_href = "";
    setViewport(-32767, -32767, -32767, -32767);
}

static ProfilingZone RenderProfilingZone("ImageNode::render");

void ImageNode::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_pImage->getState() == Image::GPU) {
        blt32(getSize(), getEffectiveOpacity(), getBlendMode(), 
                bool(m_pImage->getScene()));
    }
}

IntPoint ImageNode::getMediaSize()
{
    return m_pImage->getSize();
}

void ImageNode::checkReload()
{
    if (isSceneURL(m_href)) {
        OffscreenScenePtr pScene = Player::get()->getSceneFromURL(m_href);
        m_pImage->setScene(pScene);
    } else {
        VisibleNode::checkReload(m_href, m_pImage);
    }
    setViewport(-32767, -32767, -32767, -32767);
    RasterNode::checkReload();
}

VisibleNodePtr ImageNode::getElementByPos(const DPoint & pos)
{
    OffscreenScenePtr pScene = m_pImage->getScene();
    if (pScene && pScene->getHandleEvents()) {
        DPoint nodeSize(getSize());
        DPoint sceneSize(pScene->getSize());
        DPoint localPos(pos.x*(sceneSize.x/nodeSize.x), 
                pos.y*(sceneSize.y/nodeSize.y));
        return pScene->getRootNode()->getElementByPos(localPos);
    } else {
        return RasterNode::getElementByPos(pos);
    }
}

BitmapPtr ImageNode::getBitmap()
{
    return m_pImage->getBitmap();
}

bool ImageNode::isSceneURL(const std::string& sURL)
{
    return sURL.find("scene:") == 0;
}

}
