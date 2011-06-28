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

#ifndef _ParallelAnim_H_
#define _ParallelAnim_H_

#include "../api.h"

#include "Anim.h"

#include <vector>

namespace avg {

class ParallelAnim;
typedef boost::shared_ptr<class ParallelAnim> ParallelAnimPtr;

class AVG_API ParallelAnim: public Anim {
public:
    virtual ~ParallelAnim();
    ParallelAnim(const std::vector<AnimPtr>& anims,
            const boost::python::object& startCallback=boost::python::object(), 
            const boost::python::object& stopCallback=boost::python::object(),
            long long maxAge=-1);

    virtual void start(bool bKeepAttr=false);
    virtual void abort();
    
    virtual bool step();

private:
    std::vector<AnimPtr> m_Anims;
    std::vector<AnimPtr> m_RunningAnims;
    long long m_MaxAge;

    long long m_StartTime;
    ParallelAnimPtr m_This; // Makes sure there is always a reference to the animation
                            // while it's running.
};

}

#endif 



