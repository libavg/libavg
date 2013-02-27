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

#ifndef _Arg_H_
#define _Arg_H_

#include "../api.h"
#include "../base/GLMHelper.h"

#include "ArgBase.h"

#include <string>
#include <vector>

namespace avg {

class ExportedObject;

template<class T>
class AVG_TEMPLATE_API Arg: public ArgBase
{
public:
    Arg(std::string sName, const T& Value, bool bRequired = false, 
            ptrdiff_t MemberOffset = -1);
    virtual ~Arg();

    void setValue(const T& Value);
    const T& getValue() const;
    virtual void setMember(ExportedObject * pObj) const;
    virtual ArgBase* createCopy() const;

private:
    T m_Value;
};

template<class T>
Arg<T>::Arg(std::string sName, const T& Value, bool bRequired, 
        ptrdiff_t MemberOffset)
    : ArgBase(sName, bRequired, MemberOffset),
      m_Value(Value)
{
}

template<class T>
Arg<T>::~Arg()
{
}

template<class T>
const T& Arg<T>::getValue() const
{
    return m_Value;
}
    
template<class T>
void Arg<T>::setValue(const T& Value)
{
    m_Value = Value;
    m_bDefault = false;
}

template<class T>
void Arg<T>::setMember(ExportedObject * pObj) const
{
    if (getMemberOffset() != -1) {
        T* pMember = (T*)((char*)pObj+getMemberOffset());
        *pMember = m_Value;
    }
}

template<class T>
ArgBase* Arg<T>::createCopy() const
{
    return new Arg<T>(*this);
}

#ifdef AVG_PLUGIN
#ifndef _WIN32
// Under Linux, templates used by plugins need to be instantiated explicitly if
// RTTI is needed. Templates instantiated implicitly get instantiated again in the
// plugin with a different typeid. 
extern template class Arg<int>;
extern template class Arg<bool>;
extern template class Arg<float>;
extern template class Arg<std::string>;
extern template class Arg<glm::vec2>;
extern template class Arg<glm::vec3>;
extern template class Arg<glm::ivec3>;
extern template class Arg<std::vector<float> >;
extern template class Arg<std::vector<int> >;
extern template class Arg<std::vector<glm::vec2> >;
extern template class Arg<std::vector<glm::ivec2> >;
#endif
#endif

}

#endif

