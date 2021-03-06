//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

#include "InvertFXNode.h"

#include "../base/ObjectCounter.h"
#include "../base/Logger.h"

#include "../graphics/GPUInvertFilter.h"

#include <sstream>

using namespace std;

namespace avg {

InvertFXNode::InvertFXNode()
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

InvertFXNode::~InvertFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void InvertFXNode::disconnect()
{
    m_pFilter = GPUInvertFilterPtr();
    FXNode::disconnect();
}

GPUFilterPtr InvertFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUInvertFilterPtr(new GPUInvertFilter(size, true, false));
    setDirty();
    return m_pFilter;
}

std::string InvertFXNode::toString()
{
    stringstream s;
    s << "InvertFXNode" << std::endl;
    return s.str();
}

}

