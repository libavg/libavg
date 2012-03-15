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

#ifndef _AttrAnim_H_
#define _AttrAnim_H_

#include "Anim.h"

#include "../api.h"
// Python docs say python.h should be included before any standard headers (!)
#include "../player/WrapPython.h" 

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <string>
#include <map>

namespace avg {

struct ObjAttrID {
    ObjAttrID(const boost::python::object& obj, const std::string& sAttrName)
        : m_ObjHash(boost::python::extract<long>(obj.attr("__hash__")())),
          m_sAttrName(sAttrName)
    {
    }
    long m_ObjHash;
    std::string m_sAttrName;
    bool operator < (const ObjAttrID& other) const;
};

class AttrAnim;

typedef boost::shared_ptr<class Anim> AttrAnimPtr;
typedef boost::weak_ptr<class Anim> AttrAnimWeakPtr;

class AVG_API AttrAnim: public Anim
{
public:
    static int getNumRunningAnims();

    AttrAnim(const boost::python::object& node, const std::string& sAttrName,
            const boost::python::object& startCallback, 
            const boost::python::object& stopCallback);
    virtual ~AttrAnim();
    
    virtual void start(bool bKeepAttr=false);

protected:
    boost::python::object getValue() const;
    void setValue(const boost::python::object& val);

    void addToMap();
    void removeFromMap();
    void stopActiveAttrAnim();

private:
    AttrAnim();
    AttrAnim(const AttrAnim&);

    boost::python::object m_Node;
    std::string m_sAttrName;

    typedef std::map<ObjAttrID, AttrAnimPtr> AttrAnimationMap;
    static AttrAnimationMap s_ActiveAnimations;
};

}

#endif 



