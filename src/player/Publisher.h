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

#ifndef _Publisher_H_
#define _Publisher_H_

#include "../api.h"

#include "BoostPython.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <string>
#include <vector>
#include <map>

namespace avg {

class SubscriberInfo {
public:
    SubscriberInfo(int id, const boost::python::object& callable);
    virtual ~SubscriberInfo();

    void invoke(const std::vector<boost::python::object>& args) const;
    int getID() const;

private:
    int m_ID;
    boost::python::object m_Callable;

};

typedef boost::shared_ptr<SubscriberInfo> SubscriberInfoPtr;

class Publisher;
typedef boost::shared_ptr<Publisher> PublisherPtr;

class AVG_API Publisher: public boost::enable_shared_from_this<Publisher> 
{
public:
    Publisher();
    virtual ~Publisher();

    int subscribe(int messageID, const boost::python::object& callable);
    void unsubscribe(int messageID, int subscriberID);

    // These should really be protected, but python derived classes need to call them too.
    void publish(int messageID);
    
    template<class ARG_TYPE>
    void notifySubscribers(int messageID, const ARG_TYPE& arg);
    void notifySubscribers(int messageID, const std::vector<boost::python::object>& args);
    void notifySubscribersPy(int messageID, const boost::python::list& args);

private:
    typedef std::vector<SubscriberInfoPtr> SubscriberInfoVector;
    typedef std::map<int, SubscriberInfoVector> SignalMap;
    
    SubscriberInfoVector& safeFindSubscribers(int messageID);

    SignalMap m_SignalMap;
    static int s_LastSubscriberID;
};

template<class ARG_TYPE>
void Publisher::notifySubscribers(int messageID, const ARG_TYPE& arg)
{
    std::vector<boost::python::object> args;
    boost::python::object pyArg(arg);
    args.push_back(pyArg);
    notifySubscribers(messageID, args);
}


}

#endif

