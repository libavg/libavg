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

#ifndef _Style_H_
#define _Style_H_

#include "BoostPython.h"

#include "../api.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace avg {

class Style;
typedef boost::shared_ptr<Style> StylePtr;

class AVG_API Style 
{
    public:
        Style(const boost::python::dict& params);
        virtual ~Style();

        // python dict interface.
        boost::python::object __getitem__(boost::python::object& key) const;
        bool __contains__(boost::python::object& key) const;
        boost::python::list keys() const;
        boost::python::list values() const;
        boost::python::list items() const;
        int __len__() const;
        boost::python::object __iter__() const;
        boost::python::object iteritems() const;
        boost::python::object iterkeys() const;
        boost::python::object itervalues() const;
        std::string __repr__() const;

        // C++ interface
        boost::python::dict mergeParams(const boost::python::dict& attrs);

    private:
        const boost::python::dict& getDict()const;

        boost::python::dict m_Properties;
};

}

#endif

