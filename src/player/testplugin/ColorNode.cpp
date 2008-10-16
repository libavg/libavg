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
//  Original author of this file is Jan Boelsche (regular.gonzales@googlemail.com).
//

#include <string>
#include <iostream>

#include "../Node.h"
#include "../NodeDefinition.h"

using namespace std;

namespace avg {

class ColorNode : public Node {
public:
	static NodePtr create(const ArgList& Args, bool bFromXML);
	static NodeDefinition createNodeDefinition();
	virtual ~ColorNode() {}	
};

NodePtr ColorNode::create(const ArgList& Args, bool bFromXML) {
	return NodePtr(new ColorNode());
}

NodeDefinition ColorNode::createNodeDefinition() {
	return NodeDefinition("colornode", (NodeBuilder)ColorNode::create)
		.extendDefinition(Node::createDefinition());
}
 
}

extern "C" avg::NodeDefinition getNodeDefinition() {
	return avg::ColorNode::createNodeDefinition();
}