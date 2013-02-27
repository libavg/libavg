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

#include "OffscreenCanvasNode.h"
#include "Player.h"

#include "TypeDefinition.h"

#include "../base/FileHelper.h"

using namespace std;

namespace avg {

void OffscreenCanvasNode::registerType()
{
    TypeDefinition def = TypeDefinition("canvas", "canvasbase",
            ExportedObject::buildObject<OffscreenCanvasNode>)
        .addArg(Arg<bool>("handleevents", false, false, 
                offsetof(OffscreenCanvasNode, m_bHandleEvents)))
        .addArg(Arg<int>("multisamplesamples", 1, false, 
                offsetof(OffscreenCanvasNode, m_MultiSampleSamples)))
        .addArg(Arg<bool>("mipmap", false, false, 
                offsetof(OffscreenCanvasNode, m_bMipmap)))
        .addArg(Arg<bool>("autorender", true, false,
                offsetof(OffscreenCanvasNode, m_bAutoRender)));
    TypeRegistry::get()->registerType(def);
}

OffscreenCanvasNode::OffscreenCanvasNode(const ArgList& args)
    : CanvasNode(args)
{
    args.setMembers(this);
}

OffscreenCanvasNode::~OffscreenCanvasNode()
{
}

bool OffscreenCanvasNode::getHandleEvents() const
{
    return m_bHandleEvents;
}

int OffscreenCanvasNode::getMultiSampleSamples() const
{
    return m_MultiSampleSamples;
}

bool OffscreenCanvasNode::getMipmap() const
{
    return m_bMipmap;
}

bool OffscreenCanvasNode::getAutoRender() const
{
    return m_bAutoRender;
}

void OffscreenCanvasNode::setAutoRender(bool bAutoRender)
{
    m_bAutoRender = bAutoRender;
}

}
