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
#include "PublisherDefinitionRegistry.h"
#include "Player.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"

using namespace std;

namespace avg {

int Publisher::s_LastSubscriberID = 0;

Publisher::Publisher()
    : m_bIsInNotify(false)
{
    m_pPublisherDef = PublisherDefinition::create("");
}


Publisher::Publisher(const string& sTypeName)
    : m_bIsInNotify(false)
{
    m_pPublisherDef = PublisherDefinitionRegistry::get()->getDefinition(sTypeName);
    vector<MessageID> messageIDs = m_pPublisherDef->getMessageIDs();
    for (unsigned i=0; i<messageIDs.size(); ++i) {
        m_SignalMap[messageIDs[i]] = vector<SubscriberInfoPtr>();
    }
}

Publisher::~Publisher()
{
}

int Publisher::subscribe(MessageID messageID, const py::object& callable)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    int subscriberID = s_LastSubscriberID;
    s_LastSubscriberID++;
    subscribers.push_back(SubscriberInfoPtr(new SubscriberInfo(subscriberID, callable)));
    return subscriberID;
}

void Publisher::unsubscribe(MessageID messageID, int subscriberID)
{
    bool bFound = false;
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->getID() == subscriberID) {
            if (m_bIsInNotify) {
                tryUnsubscribeInNotify(messageID, subscriberID);
            } else {
                subscribers.erase(it);
            }
            bFound = true;
            break;
        }
    }
    checkSubscriberNotFound(bFound, messageID, subscriberID);
}

void Publisher::unsubscribeCallable(MessageID messageID, const py::object& callable)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    bool bFound = false;
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->isCallable(callable)) {
            if (m_bIsInNotify) {
                tryUnsubscribeInNotify(messageID, (*it)->getID());
            } else {
                subscribers.erase(it);
            }
            bFound = true;
            break;
        }
    }
    checkSubscriberNotFound(bFound, messageID, -1);
}

int Publisher::getNumSubscribers(MessageID messageID)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    return subscribers.size(); 
}
    
bool Publisher::isSubscribed(MessageID messageID, int subscriberID)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->getID() == subscriberID) {
            return true;
        }
    }
    return false;
}

bool Publisher::isSubscribedCallable(MessageID messageID, const py::object& callable)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->isCallable(callable)) {
            return true;
        }
    }
    return false;
}

void Publisher::publish(MessageID messageID)
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

void Publisher::notifySubscribers(MessageID messageID)
{
    SubscriberInfoVector& subscribers = safeFindSubscribers(messageID);
    if (!subscribers.empty()) {
        py::list args;
        notifySubscribersPy(messageID, args);
    }
}
    
void Publisher::notifySubscribers(const string& sMsgName)
{
    MessageID messageID = m_pPublisherDef->getMessageID(sMsgName);
    notifySubscribers(messageID);
}

void Publisher::notifySubscribersPy(MessageID messageID, const py::list& args)
{
    AVG_ASSERT(!(Player::get()->isTraversingTree()));
    m_bIsInNotify = true;
    SubscriberInfoVector& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end();) {
        if ((*it)->hasExpired()) {
            it = subscribers.erase(it);
            // Remove from the 'pending unsubscribes' list as well.
            std::vector<UnsubscribeDescription>::iterator itUnsub;
            for (itUnsub = m_PendingUnsubscribes.begin();
                    itUnsub != m_PendingUnsubscribes.end(); itUnsub++)
            {
                if (itUnsub->first == messageID && itUnsub->second == (*it)->getID()) {
                    m_PendingUnsubscribes.erase(itUnsub);
                    break;
                }
            }
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

MessageID Publisher::genMessageID()
{
    return PublisherDefinitionRegistry::get()->genMessageID();
}

Publisher::SubscriberInfoVector& Publisher::safeFindSubscribers(MessageID messageID)
{
    if (m_SignalMap.find(messageID) == m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "No signal with ID "+toString(messageID));
    }
    vector<SubscriberInfoPtr>& subscribers = m_SignalMap[messageID];
    return subscribers;
}

void Publisher::tryUnsubscribeInNotify(MessageID messageID, int subscriberID)
{
    std::vector<UnsubscribeDescription>::iterator it2;
    for (it2 = m_PendingUnsubscribes.begin(); it2 != m_PendingUnsubscribes.end(); it2++) {
        checkSubscriberNotFound(!(it2->first == messageID && it2->second == subscriberID),
                messageID, subscriberID);
    }
    m_PendingUnsubscribes.push_back(
            std::pair<MessageID, int>(messageID, subscriberID));
}

void Publisher::checkSubscriberNotFound(bool bFound, MessageID messageID,
        int subscriberID)
{
    if (!bFound) {
        if (subscriberID == -1) {
            throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                    " doesn't have a subscriber with the given callable.");
        } else {
            throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                    " doesn't have a subscriber with ID "+toString(subscriberID));
        }
    }
}

void Publisher::dumpSubscribers(MessageID messageID)
{
    vector<SubscriberInfoPtr>& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoVector::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        cerr << (*it)->getID() << " ";
    }
    cerr << endl;
}


}
