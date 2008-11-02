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

#define AVG_PLUGIN
#include "../../api.h"

#include "../../player/AreaNode.h"
#include "../../player/NodeDefinition.h"

#include "../../base/Logger.h"
#include "../../graphics/OGLHelper.h"
#include "../../wrapper/WrapHelper.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace boost::python;

namespace avg {
    
class ColorNode : public AreaNode 
{
public:
    static NodeDefinition createNodeDefinition();
    
    ColorNode(const ArgList& Args, bool bFromXML);

    void setFillColor(const std::string& sColor);
    const std::string& getFillColor() const;

    virtual void maybeRender(const DRect& Rect);
    virtual void render (const DRect& Rect);

protected:
    void parseColor(const std::string& sColorSreing);
    
    std::string m_sFillColorName;
    float m_r, m_g, m_b;
};

ColorNode::ColorNode(const ArgList& Args, bool bFromXML) :
    m_sFillColorName("FFFFFF")
{   
    AVG_TRACE(Logger::PLUGIN, "ColorNode c'tor gets Argument fillcolor= "  << Args.getArgVal<string>("fillcolor")); 
    
    Args.setMembers(this);
    AVG_TRACE(Logger::PLUGIN, "ColorNode constructed with " << m_sFillColorName);   

    parseColor(m_sFillColorName);
}

void ColorNode::setFillColor(const string& sFillColor)
{
    AVG_TRACE(Logger::PLUGIN, "setFillColor called with " << sFillColor);   
    m_sFillColorName = sFillColor;
    parseColor(m_sFillColorName);
}

const std::string& ColorNode::getFillColor() const
{
    return m_sFillColorName;
}

void ColorNode::parseColor(const std::string& sColorSreing)
{
    istringstream(sColorSreing.substr(0,2)) >> hex >> m_r;
    istringstream(sColorSreing.substr(2,2)) >> hex >> m_g;
    istringstream(sColorSreing.substr(4,2)) >> hex >> m_b;
}


void ColorNode::maybeRender(const DRect& rect)
{
    render(rect);
}

void ColorNode::render (const DRect& rect)
{
    //AVG_TRACE(Logger::PLUGIN, "ColorNode::render");   
    
    glClearColor(m_r, m_g, m_b, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT);
}

NodeDefinition ColorNode::createNodeDefinition()
{
    class_<ColorNode, bases<AreaNode>, boost::noncopyable>("ColorNode", no_init)
        .add_property("fillcolor", make_function(&ColorNode::getFillColor,
        return_value_policy<copy_const_reference>()), &ColorNode::setFillColor);
       
    return NodeDefinition("colornode", (NodeBuilder)ColorNode::buildNode<ColorNode>)
        .extendDefinition(AreaNode::createDefinition())
        .addArg(Arg<string>("fillcolor", "0F0F0F", false, 
                offsetof(ColorNode, m_sFillColorName)));
}
 
}

#ifdef _WIN32
#pragma warning(disable: 4190)
#endif
AVG_PLUGIN_API avg::NodeDefinition getNodeDefinition()
{
    return avg::ColorNode::createNodeDefinition();
}

AVG_PLUGIN_API const char** getAllowedParentNodeNames()
{
    static const char *names[] = {"avg", 0};
    return names;
}

