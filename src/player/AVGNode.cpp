//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "AVGNode.h"

#include "../base/XMLHelper.h"

#include <iostream>

using namespace std;

namespace avg {

AVGNode::AVGNode (const xmlNodePtr xmlNode, Player * pPlayer)
    : DivNode(xmlNode, pPlayer)
{
    m_bEnableCrop = getDefaultedBoolAttr (xmlNode, "enablecrop", true);
    m_sKeyUpHandler = getDefaultedStringAttr (xmlNode, "onkeyup", "");
    m_sKeyDownHandler = getDefaultedStringAttr (xmlNode, "onkeydown", "");
}

AVGNode::~AVGNode()
{
}

string AVGNode::getTypeStr ()
{
    return "AVGNode";
}

void AVGNode::handleEvent (KeyEvent* pEvent)
{
    string Code;
    int EventType = pEvent->getType();
    switch (EventType) {
        case Event::KEYDOWN:
            Code = m_sKeyDownHandler;
            break;
        case Event::KEYUP:
            Code = m_sKeyUpHandler;
            break;
         default:
            break;
    }
    if (!Code.empty()) {
        callPython(Code, *pEvent);
    } 
}

bool AVGNode::getCropSetting() {
    return m_bEnableCrop;
}

}
