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
//  Original author of this file is Jan Boelsche (regular.gonzales@googlemail.com).
//

#define AVG_PLUGIN
#include "../../api.h"

#include "../../player/Player.h"
#include "../../player/AreaNode.h"
#include "../../player/TypeDefinition.h"

#include "../../base/Logger.h"
#include "../../graphics/OGLHelper.h"
#include "../../wrapper/WrapHelper.h"
#include "../../wrapper/raw_constructor.hpp"

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
    static void registerType();
    
    ColorNode(const ArgList& Args);

    void setFillColor(const std::string& sColor);
    const std::string& getFillColor() const;

    float getFloat() const;
    void setFloat(float f);

    virtual void maybeRender(const glm::mat4& parentTransform);
    virtual void render();

protected:
    void parseColor(const std::string& sColorSreing);
    
    std::string m_sFillColorName;
    float m_r, m_g, m_b;
    float m_FloatParam;
};

ColorNode::ColorNode(const ArgList& Args) :
    m_sFillColorName("FFFFFF")
{   
    AVG_TRACE(Logger::category::PLUGIN, Logger::severity::INFO,
            "ColorNode c'tor gets Argument fillcolor= "  << Args.getArgVal<string>("fillcolor")); 
    
    Args.setMembers(this);
    AVG_TRACE(Logger::category::PLUGIN, Logger::severity::INFO,
            "ColorNode constructed with " << m_sFillColorName);   

    parseColor(m_sFillColorName);
}

void ColorNode::setFillColor(const string& sFillColor)
{
    AVG_TRACE(Logger::category::PLUGIN,  Logger::severity::INFO,
            "setFillColor called with " << sFillColor);   
    m_sFillColorName = sFillColor;
    parseColor(m_sFillColorName);
}

const std::string& ColorNode::getFillColor() const
{
    return m_sFillColorName;
}

float ColorNode::getFloat() const
{
    return m_FloatParam;
}

void ColorNode::setFloat(float f)
{
    m_FloatParam = f;
}


void ColorNode::parseColor(const std::string& sColorSreing)
{
    istringstream(sColorSreing.substr(0,2)) >> hex >> m_r;
    istringstream(sColorSreing.substr(2,2)) >> hex >> m_g;
    istringstream(sColorSreing.substr(4,2)) >> hex >> m_b;
}


void ColorNode::maybeRender(const glm::mat4& parentTransform)
{
    render();
}

void ColorNode::render()
{
    //AVG_TRACE(Logger::category::PLUGIN, "ColorNode::render");   
    
    glClearColor(m_r, m_g, m_b, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT);
}

char colorNodeName[] = "colornode";

void ColorNode::registerType()
{
    avg::TypeDefinition def = avg::TypeDefinition("colornode", "areanode", 
            ExportedObject::buildObject<ColorNode>)
        .addArg(Arg<float>("floatparam", 0.0f, false,
                offsetof(ColorNode, m_FloatParam)))
        .addArg(Arg<string>("fillcolor", "0F0F0F", false, 
                offsetof(ColorNode, m_sFillColorName)));
    const char* allowedParentNodeNames[] = {"avg", 0};
    avg::TypeRegistry::get()->registerType(def, allowedParentNodeNames);
}
 
}

using namespace avg;

BOOST_PYTHON_MODULE(colorplugin)
{
    class_<ColorNode, bases<AreaNode>, boost::noncopyable>("ColorNode", no_init)
        .def("__init__", raw_constructor(createNode<colorNodeName>))
        .add_property("floatparam", &ColorNode::getFloat, &ColorNode::setFloat)
        .add_property("fillcolor", make_function(&ColorNode::getFillColor,
        return_value_policy<copy_const_reference>()), &ColorNode::setFillColor);
}

AVG_PLUGIN_API void registerPlugin()
{
    initcolorplugin();
    object mainModule(handle<>(borrowed(PyImport_AddModule("__builtin__"))));
    object colorModule(handle<>(PyImport_ImportModule("colorplugin")));
    mainModule.attr("colorplugin") = colorModule;

    avg::ColorNode::registerType();

}

