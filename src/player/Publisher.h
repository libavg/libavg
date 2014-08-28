//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _Publisher_H_
#define _Publisher_H_

#include "../api.h"

#include "ExportedObject.h"
#include "BoostPython.h"
#include "PublisherDefinition.h"
#include "MessageID.h"

#include <boost/shared_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <list>
#include <map>

namespace avg {

class SubscriberInfo;
typedef boost::shared_ptr<SubscriberInfo> SubscriberInfoPtr;
typedef boost::weak_ptr<SubscriberInfo> SubscriberInfoWeakPtr;

class Publisher;
typedef boost::shared_ptr<Publisher> PublisherPtr;

class AVG_API Publisher: public ExportedObject
{
public:
    Publisher();
    Publisher(const std::string& sTypeName);
    virtual ~Publisher();

    int subscribe(MessageID messageID, PyObject* pCallable);
    void unsubscribe(MessageID messageID, int subscriberID);
    void unsubscribe1(int subscriberID);
    void unsubscribeCallable(MessageID messageID, PyObject* pCallable);
    int getNumSubscribers(MessageID messageID);
    bool isSubscribed(MessageID messageID, int subscriberID);
    bool isSubscribedCallable(MessageID messageID, PyObject* pCallable);

    // The following methods should really be protected, but python derived classes need
    // to call them too.
    void publish(MessageID messageID);
   
    void notifySubscribers(MessageID messageID);
    void notifySubscribers(const std::string& sMsgName);
    template<class ARG_TYPE>
    void notifySubscribers(const std::string& sMsgName, const ARG_TYPE& arg);
    void notifySubscribersPy(MessageID messageID, const py::list& args);

    static MessageID genMessageID();

protected:
    void removeSubscribers();

private:
    typedef std::list<SubscriberInfoWeakPtr> WeakSubscriberInfoList;
    typedef std::list<SubscriberInfoPtr> SubscriberInfoList;
    typedef std::map<MessageID, SubscriberInfoList> SignalMap;
    
    void unsubscribeIterator(MessageID messageID, SubscriberInfoList::iterator it);
    SubscriberInfoList& safeFindSubscribers(MessageID messageID);
    void throwSubscriberNotFound(MessageID messageID, int subscriberID);
    void dumpSubscribers(MessageID messageID);

    PublisherDefinitionPtr m_pPublisherDef;
    SignalMap m_SignalMap;
    static int s_LastSubscriberID;

    typedef std::pair<MessageID, int> UnsubscribeDescription;
};

template<class ARG_TYPE>
void Publisher::notifySubscribers(const std::string& sMsgName, const ARG_TYPE& arg)
{
    MessageID messageID = m_pPublisherDef->getMessageID(sMsgName);
    SubscriberInfoList& subscribers = safeFindSubscribers(messageID);
    if (!subscribers.empty()) {
        py::list args;
        py::object pyArg(arg);
        args.append(pyArg);
        notifySubscribersPy(messageID, args);
    }
}


}

#endif

