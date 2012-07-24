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

boost::python::object SubscriberInfo::s_MethodrefModule;

SubscriberInfo::SubscriberInfo(int id, const boost::python::object& callable)
    : m_ID(id)
{
    if (s_MethodrefModule.ptr() == boost::python::object().ptr()) {
        s_MethodrefModule = boost::python::import("libavg.methodref");
    }
    // Use the methodref module to manage the lifetime of the callables. This makes 
    // sure that all callbacks are deleted when the publisher disappears.
    m_Callable = boost::python::object(s_MethodrefModule.attr("methodref")(callable));
}

SubscriberInfo::~SubscriberInfo()
{
}

void SubscriberInfo::invoke(const std::vector<boost::python::object>& args) const
{
    boost::python::object callWeakRef = s_MethodrefModule.attr("callWeakRef");
    switch (args.size()) {
        case 0:
            callWeakRef(m_Callable);
            break;
        case 1:
            callWeakRef(m_Callable, args[0]);
            break;
        case 2:
            callWeakRef(m_Callable, args[0], args[1]);
            break;
        case 3:
            callWeakRef(m_Callable, args[0], args[1], args[2]);
            break;
        case 4:
            callWeakRef(m_Callable, args[0], args[1], args[2], args[3]);
            break;
        case 5:
            callWeakRef(m_Callable, args[0], args[1], args[2], args[3], args[4]);
            break;
        case 6:
            callWeakRef(m_Callable, args[0], args[1], args[2], args[3], args[4], args[5]);
            break;
        default:
            AVG_ASSERT_MSG(false, "Messages with > 6 parameters not implemented yet. Please file a bug if you need this support.");
    }
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

int Publisher::subscribe(int messageID, const boost::python::object& callable)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    int subscriberID = s_LastSubscriberID;
    s_LastSubscriberID++;
    subscribers.push_back(SubscriberInfoPtr(new SubscriberInfo(subscriberID, callable)));
    return subscriberID;
}

void Publisher::unsubscribe(int messageID, int subscriberID)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
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
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                " doesn't have a subscriber with ID "+toString(subscriberID));
    }
}

void Publisher::publish(int messageID)
{
    if (m_SignalMap.find(messageID) != m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                "already registered.");
    }
    m_SignalMap[messageID] = vector<SubscriberInfoPtr>();
}

void Publisher::notifySubscribersPy(int messageID, const boost::python::list& args)
{
    /*
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        (*it)->invoke(args);
    }
    */
}

void Publisher::notifySubscribers(int messageID,
        const std::vector<boost::python::object>& args)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        (*it)->invoke(args);
    }
}

Publisher::SubscriberInfoVector& Publisher::safeFindSubscribers(int messageID)
{
    if (m_SignalMap.find(messageID) == m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "No signal with ID "+toString(messageID));
    }
    vector<SubscriberInfoPtr>& subscribers = m_SignalMap[messageID];
    return subscribers;
}

}
