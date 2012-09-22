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

#ifndef _OffscreenCanvasNode_H_
#define _OffscreenCanvasNode_H_

#include "../api.h"
#include "CanvasNode.h"

#include <string>

namespace avg {

class AVG_API OffscreenCanvasNode : public CanvasNode
{
    public:
        static void registerType();
        
        OffscreenCanvasNode(const ArgList& args);
        virtual ~OffscreenCanvasNode();

        bool getHandleEvents() const;
        int getMultiSampleSamples() const;
        bool getMipmap() const;
        bool getAutoRender() const;
        void setAutoRender(bool bAutoRender);

    private:
        bool m_bHandleEvents;
        int m_MultiSampleSamples;
        bool m_bMipmap;
        bool m_bAutoRender;
};

typedef boost::shared_ptr<OffscreenCanvasNode> OffscreenCanvasNodePtr;

}

#endif
