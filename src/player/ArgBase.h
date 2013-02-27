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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#ifndef _ArgBase_H_
#define _ArgBase_H_

#include "../api.h"

#include <string>

#include <boost/shared_ptr.hpp>

namespace avg {

class ExportedObject;

class AVG_API ArgBase
{
public:
    ArgBase(std::string sName, bool bRequired, ptrdiff_t memberOffset);
    virtual ~ArgBase();
    
    std::string getName() const;
    bool isDefault() const;
    bool isRequired() const;
    
    virtual void setMember(ExportedObject * pObj) const = 0;
   
    virtual ArgBase* createCopy() const = 0;

protected:
    ptrdiff_t getMemberOffset() const;
    bool m_bDefault;

private:
    std::string m_sName;
    bool m_bRequired;
    ptrdiff_t m_MemberOffset;
};

typedef boost::shared_ptr<ArgBase> ArgBasePtr;

}

#endif
