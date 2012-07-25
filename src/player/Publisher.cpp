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

#include "SubscriberInfo.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"

using namespace std;

namespace avg {

int Publisher::s_LastSubscriberID = 0;

Publisher::Publisher()
    : m_bIsInNotify(false)
{
}

Publisher::~Publisher()
{
}

int Publisher::subscribe(int messageID, const py::object& callable)
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
            if (m_bIsInNotify) {
                m_PendingUnsubscribes.push_back(
                        std::pair<int, int>(messageID, subscriberID));
            } else {
                subscribers.erase(it);
            }
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                " doesn't have a subscriber with ID "+toString(subscriberID));
    }
}

void Publisher::unsubscribeCallable(int messageID, const py::object& callable)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    bool bFound = false;
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->isCallable(callable)) {
            if (m_bIsInNotify) {
                m_PendingUnsubscribes.push_back(
                        std::pair<int, int>(messageID, (*it)->getID()));
            } else {
                subscribers.erase(it);
            }
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                " doesn't have a subscriber with the given callable.");
    }

}

int Publisher::getNumSubscribers(int messageID)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    return subscribers.size(); 
}

void Publisher::publish(int messageID)
{
    if (m_SignalMap.find(messageID) != m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                "already registered.");
    }
    m_SignalMap[messageID] = vector<SubscriberInfoPtr>();
}

void Publisher::removeSubscribers()
{
    SignalMap::iterator it;
    for (it = m_SignalMap.begin(); it != m_SignalMap.end(); ++it) {
        it->second = SubscriberInfoVector();
    }
}

void Publisher::notifySubscribersPy(int messageID, const py::list& args)
{
    m_bIsInNotify = true;
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end();) {
        if ((*it)->hasExpired()) {
            it = subscribers.erase(it);
        } else {
            (*it)->invoke(args);
            it++;
        }
    }
    m_bIsInNotify = false;

    // The subscribers can issue unsubscribes during the notification. We delay processing
    // them so for loop above doesn't get messed up.
    std::vector<UnsubscribeDescription>::iterator itUnsub;
    for (itUnsub = m_PendingUnsubscribes.begin(); itUnsub != m_PendingUnsubscribes.end();
            itUnsub++)
    {
        unsubscribe(itUnsub->first, itUnsub->second);
    }
    m_PendingUnsubscribes.clear();
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
