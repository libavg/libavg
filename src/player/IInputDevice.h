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

#ifndef _IInputDevice_H_
#define _IInputDevice_H_

#include "../api.h"
#include "DivNode.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

#define EXTRACT_INPUTDEVICE_CLASSNAME( classType ) (#classType)

namespace avg {

class Event;
typedef boost::shared_ptr<Event> EventPtr;
typedef boost::shared_ptr<DivNode> DivNodePtr;

class AVG_API IInputDevice {
    public:
        IInputDevice(const std::string& name,
                const DivNodePtr& pEventReceiverNode=DivNodePtr())
            : m_sName(name),
              m_pEventReceiverNode(pEventReceiverNode)
        {
        }

        virtual ~IInputDevice()
        {
        };

        virtual void start()
        {
        };

        virtual std::vector<EventPtr> pollEvents() = 0;

        const DivNodePtr& getEventReceiverNode() const
        {
            return m_pEventReceiverNode;
        }

        void setEventReceiverNode(const DivNodePtr pEventReceiverNode)
        {
            m_pEventReceiverNode = pEventReceiverNode;
        }

        const std::string& getName() const
        {
            return m_sName;
        }

    private:
        std::string m_sName;
        DivNodePtr m_pEventReceiverNode;
};

typedef boost::shared_ptr<IInputDevice> IInputDevicePtr;

}

#endif
