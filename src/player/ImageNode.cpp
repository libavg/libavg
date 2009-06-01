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
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/Exception.h"

#include "../graphics/Filterfliprgb.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition ImageNode::createDefinition()
{
    return NodeDefinition("image", Node::buildNode<ImageNode>)
        .extendDefinition(RasterNode::createDefinition())
        .addArg(Arg<string>("href", "", false, offsetof(ImageNode, m_href)));
}

ImageNode::ImageNode(const ArgList& Args, bool bFromXML)
{
    m_pImage = ImagePtr(new Image(getSurface(), "", true));
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
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    m_pImage->moveToGPU(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
}

void ImageNode::connect()
{
    RasterNode::connect();
    checkReload();
}

void ImageNode::disconnect(bool bKill)
{
    if (bKill) {
        m_pImage = ImagePtr(new Image(getSurface(), "", true));
    } else {
        m_pImage->moveToCPU();
    }
    RasterNode::disconnect(bKill);
}

const std::string& ImageNode::getHRef() const
{
    return m_href;
}

void ImageNode::setHRef(const string& href)
{
    m_href = href;
    checkReload();
}

void ImageNode::setBitmap(const Bitmap * pBmp)
{
    m_pImage->setBitmap(pBmp);
    m_href = "";
    IntPoint Size = getMediaSize();
    setViewport(-32767, -32767, -32767, -32767);
}

static ProfilingZone RenderProfilingZone("ImageNode::render");

void ImageNode::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_pImage->getState() == Image::GPU) {
        m_pImage->getTiledSurface()->blt32(getSize(), getEffectiveOpacity(), 
                getBlendMode());
    }
}

IntPoint ImageNode::getMediaSize()
{
    return m_pImage->getSize();
}

void ImageNode::checkReload()
{
    Node::checkReload(m_href, m_pImage);
    IntPoint Size = getMediaSize();
    setViewport(-32767, -32767, -32767, -32767);
    RasterNode::checkReload();
}

Bitmap * ImageNode::getBitmap()
{
    return m_pImage->getBitmap();
}

}
