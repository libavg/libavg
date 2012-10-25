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
        m_SignalMap[messageIDs[i]] = list<SubscriberInfoPtr>();
    }
}

Publisher::~Publisher()
{
}

int Publisher::subscribe(MessageID messageID, const py::object& callable)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    int subscriberID = s_LastSubscriberID;
    s_LastSubscriberID++;
//    cerr << this << " subscribe " << messageID << ", " << subscriberID << endl;
    subscribers.push_front(SubscriberInfoPtr(new SubscriberInfo(subscriberID, callable)));
    return subscriberID;
}

void Publisher::unsubscribe(MessageID messageID, int subscriberID)
{
//    cerr << this << " unsubscribe " << messageID << ", " << subscriberID << endl;
//    cerr << "  ";
//    dumpSubscribers(messageID);
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoList::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->getID() == subscriberID) {
            if (m_bIsInNotify) {
//                cerr << "  delayed" << endl;
                tryUnsubscribeInNotify(messageID, subscriberID);
            } else {
//                cerr << "  removed" << endl;
                subscribers.erase(it);
            }
            return;
        }
    }
//    cerr << "  End of unsubscribe: ";
//    dumpSubscribers(messageID);
    throwSubscriberNotFound(messageID, subscriberID);
}

void Publisher::unsubscribe1(int subscriberID)
{
    SignalMap::iterator it;
    for (it = m_SignalMap.begin(); it != m_SignalMap.end(); ++it) {
        SubscriberInfoList& subscribers = it->second;
        SubscriberInfoList::iterator it2;
        for (it2 = subscribers.begin(); it2 != subscribers.end(); it2++) {
            if ((*it2)->getID() == subscriberID) {
                if (m_bIsInNotify) {
//                    cerr << "  delayed" << endl;
                    MessageID messageID = it->first;
                    tryUnsubscribeInNotify(messageID, subscriberID);
                } else {
//                    cerr << "  removed" << endl;
                    subscribers.erase(it2);
                }
                return;
            }
        }
    }
    throw Exception(AVG_ERR_INVALID_ARGS, 
            "Subscriber with ID "+toString(subscriberID)+" not found.");
}

void Publisher::unsubscribeCallable(MessageID messageID, const py::object& callable)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoList::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->isCallable(callable)) {
            if (m_bIsInNotify) {
                tryUnsubscribeInNotify(messageID, (*it)->getID());
            } else {
                subscribers.erase(it);
            }
            return;
        }
    }
    throwSubscriberNotFound(messageID, -1);
}

int Publisher::getNumSubscribers(MessageID messageID)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    return subscribers.size(); 
}
    
bool Publisher::isSubscribed(MessageID messageID, int subscriberID)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoList::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        if ((*it)->getID() == subscriberID) {
            return true;
        }
    }
    return false;
}

bool Publisher::isSubscribedCallable(MessageID messageID, const py::object& callable)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoList::iterator it;
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
    m_SignalMap[messageID] = SubscriberInfoList();
}

void Publisher::removeSubscribers()
{
    SignalMap::iterator it;
    for (it = m_SignalMap.begin(); it != m_SignalMap.end(); ++it) {
        it->second = SubscriberInfoList();
    }
}

void Publisher::notifySubscribers(MessageID messageID)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
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
//    cerr << this << " notifySubscribers " << messageID << endl;
//    cerr << "  ";
//    dumpSubscribers(messageID);
    AVG_ASSERT(!(Player::get()->isTraversingTree()));
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoList::iterator it;
    for (it = subscribers.begin(); it != subscribers.end();) {
//        cerr << "  next" << endl;
        if ((*it)->hasExpired()) {
//            cerr << "  expired: " << (*it)->getID() << endl;
            subscribers.erase(it++);
        } else {
//            cerr << "  invoke: " << (*it)->getID() << endl;
            m_bIsInNotify = true;
            (*it)->invoke(args);
            m_bIsInNotify = false;

            // Process pending unsubscribe requests
            bool bCurrentIsUnsubscribed = false;
            std::vector<UnsubscribeDescription>::iterator itUnsub;
            for (itUnsub = m_PendingUnsubscribes.begin(); 
                    itUnsub != m_PendingUnsubscribes.end();
                    itUnsub++)
            {
//                cerr << "    Unsubscribing: " << itUnsub->second << endl;
                if (it != subscribers.end() && itUnsub->first == messageID && 
                         itUnsub->second == (*it)->getID())
                {
//                    cerr << "      current" << endl;
                    it++;
                    bCurrentIsUnsubscribed = true;
                }
//                cerr << "      ";
//                dumpSubscribers(messageID);
                unsubscribe(itUnsub->first, itUnsub->second);
//                cerr << "      done" << endl;
            }
            m_PendingUnsubscribes.clear();
            if (!bCurrentIsUnsubscribed) {
                it++;
            }
//            cerr << "    end invoke" << endl;
        }
    }
//    cerr << "  end notify" << endl;
}

MessageID Publisher::genMessageID()
{
    return PublisherDefinitionRegistry::get()->genMessageID();
}

Publisher::SubscriberInfoList& Publisher::safeFindSubscribers(MessageID messageID)
{
    if (m_SignalMap.find(messageID) == m_SignalMap.end()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "No signal with ID "+toString(messageID));
    }
    SubscriberInfoList& subscribers = m_SignalMap[messageID];
    return subscribers;
}

void Publisher::tryUnsubscribeInNotify(MessageID messageID, int subscriberID)
{
    std::vector<UnsubscribeDescription>::iterator it2;
    for (it2 = m_PendingUnsubscribes.begin(); it2 != m_PendingUnsubscribes.end(); it2++) {
        if (it2->first == messageID && it2->second == subscriberID) {
            throwSubscriberNotFound(messageID, subscriberID);
        }
    }
    m_PendingUnsubscribes.push_back(std::pair<MessageID, int>(messageID, subscriberID));
}

void Publisher::throwSubscriberNotFound(MessageID messageID, int subscriberID)
{
    if (subscriberID == -1) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                " doesn't have a subscriber with the given callable.");
    } else {
        throw Exception(AVG_ERR_INVALID_ARGS, "Signal with ID "+toString(messageID)+
                " doesn't have a subscriber with ID "+toString(subscriberID));
    }
}

void Publisher::dumpSubscribers(MessageID messageID)
{
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    SubscriberInfoList::iterator it;
    for (it = subscribers.begin(); it != subscribers.end(); it++) {
        cerr << (*it)->getID() << " ";
    }
    cerr << endl;
}


}
