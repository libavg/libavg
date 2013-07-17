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

#ifndef _StateAnim_H_
#define _StateAnim_H_

#include "../api.h"

#include "Anim.h"

#include <vector>
#include <map>

namespace avg {

struct AVG_API AnimState {
    AnimState(const std::string& sName, AnimPtr pAnim, const std::string& sNextName = "");
    AnimState();

    std::string m_sName;
    AnimPtr m_pAnim;
    std::string m_sNextName;
};

class AVG_API StateAnim: public Anim {
public:
    StateAnim(const std::vector<AnimState>& states);
    virtual ~StateAnim();

    virtual void abort();

    virtual void setState(const std::string& sName, bool bKeepAttr=false);
    const std::string& getState() const;
    void setDebug(bool bDebug);
    
    virtual bool step();

private:
    void switchToNewState(const std::string& sName, bool bKeepAttr);

    std::map<std::string, AnimState> m_States;
    bool m_bDebug;
    std::string m_sCurStateName;
};

}

#endif 



