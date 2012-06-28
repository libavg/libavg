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

#include "Style.h"

#include "../base/ObjectCounter.h"

using namespace std;

namespace avg {

Style::Style(const boost::python::dict& params)
{
    // The constructor param is just the python args dictionary.
    // Extract the basestyle argument & put the rest of the arguments in m_Properties.
    StylePtr pBaseStyle;
    if (params.has_key("basestyle")) {
        pBaseStyle = boost::python::extract<StylePtr>(params["basestyle"]);
    }
    if (pBaseStyle) {
        boost::python::dict d = pBaseStyle->getDict();
        m_Properties = d.copy();
    }
    m_Properties.update(params);
    if (params.has_key("basestyle")) {
        m_Properties.attr("__delitem__")("basestyle");
    }
    ObjectCounter::get()->incRef(&typeid(*this));
}

Style::~Style()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

const boost::python::dict& Style::getDict() const
{
    return m_Properties;
}

}

