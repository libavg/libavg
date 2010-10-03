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

#ifndef _ChromaKeyFXNode_H_
#define _ChromaKeyFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/GPUChromaKeyFilter.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API ChromaKeyFXNode: public FXNode {
public:
    ChromaKeyFXNode();
    virtual ~ChromaKeyFXNode();

    virtual void disconnect();

    void setColor(const Pixel32& color);
    const Pixel32& getColor() const;
    void setHTolerance(double tolerance);
    double getHTolerance() const;
    void setSTolerance(double tolerance);
    double getSTolerance() const;
    void setLTolerance(double tolerance);
    double getLTolerance() const;
    void setSoftness(double softness);
    double getSoftness() const;

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size);
    void setFilterParams();

    GPUChromaKeyFilterPtr m_pFilter;

    Pixel32 m_Color;
    double m_HTolerance;
    double m_STolerance;
    double m_LTolerance;
    double m_Softness;
};

typedef boost::shared_ptr<ChromaKeyFXNode> ChromaKeyFXNodePtr;

}

#endif

