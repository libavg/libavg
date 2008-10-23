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

#include "../Node.h"
#include "../NodeDefinition.h"

#include "../../base/Logger.h"
#include "../../graphics/Pixel32.h"
#include "../../wrapper/WrapHelper.h"

#include <OpenGL/OpenGL.h>

#include <string>
#include <iostream>

using namespace std;
using namespace boost::python;

namespace avg {
	
class ColorNode : public Node 
{
public:
	static NodePtr create(const ArgList& Args, bool bFromXML);
	static NodeDefinition createNodeDefinition();
	
	ColorNode(const ArgList& Args, bool bFromXML);
	virtual ~ColorNode() {}

	void setFillColor(const std::string& sColor);
    const std::string& getFillColor() const;
	void render (const DRect& Rect);

private:
	std::string m_sFillColorName;
    Pixel32 m_FillColor;
};

ColorNode::ColorNode(const ArgList& Args, bool bFromXML) :
	m_sFillColorName("FFFFFF")
{   
    AVG_TRACE(Logger::PLUGIN, "ColorNode c'tor gets Argument fillcolor= "  << Args.getArgVal<string>("fillcolor"));	
    
	Args.setMembers(this);
	AVG_TRACE(Logger::PLUGIN, "ColorNode constructed with " << m_sFillColorName);	

    m_FillColor = colorStringToColor(m_sFillColorName);
}

void ColorNode::setFillColor(const string& sFillColor)
{
	AVG_TRACE(Logger::PLUGIN, "setFillColor called with " << sFillColor);	
    if (sFillColor != m_sFillColorName) {
     	m_sFillColorName = sFillColor;
        m_FillColor = colorStringToColor(m_sFillColorName);
        //setDrawNeeded(true);
    }
}

const std::string& ColorNode::getFillColor() const
{
    return m_sFillColorName;
}

void ColorNode::render (const DRect& Rect)
{
	AVG_TRACE(Logger::PLUGIN, "ColorNode::render");	
	
	glClearColor(1.0, 1.0, 0.0, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT);
}


NodePtr ColorNode::create(const ArgList& Args, bool bFromXML)
{
	return NodePtr(new ColorNode(Args, bFromXML));
}


NodeDefinition ColorNode::createNodeDefinition()
{
	class_<ColorNode, bases<Node>, boost::noncopyable>("ColorNode", no_init)
        .add_property("fillcolor", make_function(&ColorNode::getFillColor,
		return_value_policy<copy_const_reference>()), &ColorNode::setFillColor);
       
	return NodeDefinition("colornode", (NodeBuilder)ColorNode::create)
		.extendDefinition(Node::createDefinition())
		.addArg(Arg<string>("fillcolor", "0F0F0F", false, 
                offsetof(ColorNode, m_sFillColorName)));
}
 
}

extern "C" avg::NodeDefinition getNodeDefinition() {
	return avg::ColorNode::createNodeDefinition();
}

