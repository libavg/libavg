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

#include "WrapHelper.h"
#include "raw_constructor.hpp"

#include "../player/FXNode.h"
#include "../player/NullFXNode.h"
#include "../player/BlurFXNode.h"
#include "../player/ShadowFXNode.h"

#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;

void export_fx()
{

    class_<FXNode, boost::shared_ptr<FXNode>, boost::noncopyable>("FXNode", no_init)
        ;

    class_<NullFXNode, bases<FXNode>, boost::shared_ptr<NullFXNode>, boost::noncopyable>(
            "NullFXNode")
        ;

    class_<BlurFXNode, bases<FXNode>, boost::shared_ptr<BlurFXNode>, 
            boost::noncopyable>("BlurFXNode")
        .def("setParam", &BlurFXNode::setParam)
        ;
    
    class_<ShadowFXNode, bases<FXNode>, boost::shared_ptr<ShadowFXNode>, 
            boost::noncopyable>("ShadowFXNode")
        .def("setParams", &ShadowFXNode::setParams)
        ;
}
