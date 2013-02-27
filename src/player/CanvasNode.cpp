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

#include "CanvasNode.h"
#include "Player.h"

#include "TypeDefinition.h"

#include "../base/FileHelper.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {

void CanvasNode::registerType()
{
    TypeDefinition def = TypeDefinition("canvasbase", "div", 
            ExportedObject::buildObject<CanvasNode>);
    TypeRegistry::get()->registerType(def);
}

CanvasNode::CanvasNode(const ArgList& args)
    : DivNode(args)
{
    args.setMembers(this);
    if (getSize() == glm::vec2(0, 0)) {
        throw (Exception(AVG_ERR_OUT_OF_RANGE,
                "<avg> and <canvas> node width and height attributes are mandatory."));
    }
}

CanvasNode::~CanvasNode()
{
}

string CanvasNode::getEffectiveMediaDir()
{
    string sMediaDir = getMediaDir();
    if (!isAbsPath(sMediaDir)) {
        sMediaDir = Player::get()->getCurDirName()+sMediaDir;
    }
    if (sMediaDir[sMediaDir.length()-1] != '/') {
        sMediaDir += '/';
    }
    return sMediaDir;
}

}
