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
    return NodeDefinition("image", Node::buildNode<ImageNode>)
        .extendDefinition(RasterNode::createDefinition())
        .addArg(Arg<UTF8String>("href", "", false, offsetof(ImageNode, m_href)));
}

ImageNode::ImageNode(const ArgList& Args)
{
    m_pImage = ImagePtr(new Image(getSurface(), ""));
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
    if (m_pImage) {
        m_pImage->moveToGPU(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    } else {
        getSurface()->create(m_pScene->getSize(), B8G8R8X8);
        getSurface()->setTexID(m_pScene->getTexID());
    }
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
        if (m_pImage) {
            m_pImage = ImagePtr(new Image(getSurface(), ""));
        }
        m_href = "";
    } else {
        if (m_pImage) {
            m_pImage->moveToCPU();
        }
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
    if (getState() == Node::NS_CANRENDER) {
        bind();
    }
    m_href = "";
    setViewport(-32767, -32767, -32767, -32767);
}

static ProfilingZone RenderProfilingZone("ImageNode::render");

void ImageNode::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_pScene || m_pImage->getState() == Image::GPU) {
        blt32(getSize(), getEffectiveOpacity(), getBlendMode());
    }
}

IntPoint ImageNode::getMediaSize()
{
    if (m_pImage) {
        return m_pImage->getSize();
    } else {
        return m_pScene->getSize();
    }
}

void ImageNode::checkReload()
{
    if (isSceneURL(m_href)) {
        m_pScene = Player::get()->getSceneFromURL(m_href);
        if (m_pScene->isRunning()) {
            getSurface()->setTexID(m_pScene->getTexID());
        }
        m_pImage = ImagePtr();
    } else {
        Node::checkReload(m_href, m_pImage);
        setViewport(-32767, -32767, -32767, -32767);
    }
    RasterNode::checkReload();
}

BitmapPtr ImageNode::getBitmap()
{
    if (m_pImage) {
        return m_pImage->getBitmap();
    } else {
        return getSurface()->readbackBmp();
    }
}

bool ImageNode::isSceneURL(const std::string& sURL)
{
    return sURL.find("scene:") == 0;
}

}
