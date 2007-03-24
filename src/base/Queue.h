//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _Queue_H_
#define _Queue_H_

#include "Exception.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <deque>

namespace avg {

template<class QElement>
class Queue 
{
    public:
        Queue(int MaxSize=-1);
        virtual ~Queue();

        bool empty() const;
        QElement pop(bool bBlock = true);
        void push(const QElement& Elem);
        int size() const;
        int getMaxSize() const;

    private:
        std::deque<QElement> m_Elements;
        mutable boost::mutex m_Mutex;
        mutable boost::condition m_Cond;
        int m_MaxSize;
};

typedef boost::mutex::scoped_lock scoped_lock;

template<class QElement>
Queue<QElement>::Queue(int MaxSize)
    : m_MaxSize(MaxSize)
{
}

template<class QElement>
Queue<QElement>::~Queue()
{
}

template<class QElement>
bool Queue<QElement>::empty() const
{
    scoped_lock Lock(m_Mutex);
    return m_Elements.empty();
}

template<class QElement>
QElement Queue<QElement>::pop(bool bBlock)
{
    scoped_lock Lock(m_Mutex);
    if(m_Elements.empty()) {
        if (bBlock) {
            while(m_Elements.empty()) {
                m_Cond.wait(Lock);
            }
        } else {
            throw Exception(AVG_ERR_QUEUE_EMPTY);
        }
    }
    QElement Elem = m_Elements.front(); 
    m_Elements.pop_front();
    m_Cond.notify_one();
    return Elem;
}

template<class QElement>
void Queue<QElement>::push(const QElement& Elem)
{
    scoped_lock Lock(m_Mutex);
    if(m_Elements.size() == (unsigned)m_MaxSize) {
        while(m_Elements.size() == (unsigned)m_MaxSize) {
            m_Cond.wait(Lock);
        }
    }
    m_Elements.push_back(Elem);
    m_Cond.notify_one();
}

template<class QElement>
int Queue<QElement>::size() const
{
    scoped_lock Lock(m_Mutex);
    return m_Elements.size();
}

template<class QElement>
int Queue<QElement>::getMaxSize() const
{
    scoped_lock Lock(m_Mutex);
    return m_MaxSize;
}

}
#endif
