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

#include "Publisher.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"

using namespace std;

namespace avg {

SubscriberInfo::SubscriberInfo(int id, const boost::python::object& callable)
    : m_ID(id),
      m_Callable(callable)
{
    // TODO: callable needs to be weakref
}

SubscriberInfo::~SubscriberInfo()
{
}

void SubscriberInfo::invoke(const boost::python::dict& args) const
{
    m_Callable(**args);
}

int SubscriberInfo::getID() const
{
    return m_ID;
}

int Publisher::s_LastSubscriberID = 0;

Publisher::Publisher()
{
}

Publisher::~Publisher()
{
}

int Publisher::subscribe(int signalID, const boost::python::object& callable)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(signalID);
    int subscriberID = s_LastSubscriberID;
    s_LastSubscriberID++;
    subscribers.push_back(SubscriberInfoPtr(new SubscriberInfo(subscriberID, callable)));
    return subscriberID;
}

void Publisher::unsubscribe(int signalID, int subscriberID)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(signalID);
    bool bFound = false;
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->getID() == subscriberID) {
            subscribers.erase(it);
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(signalID)+
                " doesn't have a subscriber with ID "+toString(subscriberID));
    }
}

void Publisher::publish(int signalID)
{
    if (m_SignalMap.find(signalID) != m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(signalID)+
                "already registered.");
    }
    m_SignalMap[signalID] = vector<SubscriberInfoPtr>();
}

void Publisher::notifySubscribers(int signalID, const boost::python::dict& args)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(signalID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        (*it)->invoke(args);
    }
}

Publisher::SubscriberInfoVector& Publisher::safeFindSubscribers(int signalID)
{
    if (m_SignalMap.find(signalID) == m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "No signal with ID "+toString(signalID));
    }
    vector<SubscriberInfoPtr>& subscribers = m_SignalMap[signalID];
    return subscribers;
}

}
