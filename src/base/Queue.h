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

#ifndef _Queue_H_
#define _Queue_H_

#include "../api.h"
#include "Exception.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>

#include <deque>
#include <iostream>

namespace avg {

typedef boost::mutex::scoped_lock scoped_lock;

template<class QElement>
class AVG_TEMPLATE_API Queue 
{
public:
    typedef boost::shared_ptr<QElement> QElementPtr;

    Queue(int maxSize=-1);
    virtual ~Queue();

    bool empty() const;
    QElementPtr pop(bool bBlock = true);
    void clear();
    void push(const QElementPtr& pElem);
    QElementPtr peek(bool bBlock = true) const;
    int size() const;
    int getMaxSize() const;

private:
    QElementPtr getFrontElement(bool bBlock, scoped_lock& Lock) const;

    std::deque<QElementPtr> m_pElements;
    mutable boost::mutex m_Mutex;
    mutable boost::condition m_Cond;
    int m_MaxSize;
};

template<class QElement>
Queue<QElement>::Queue(int maxSize)
    : m_MaxSize(maxSize)
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
    return m_pElements.empty();
}

template<class QElement>
typename Queue<QElement>::QElementPtr Queue<QElement>::pop(bool bBlock)
{
    scoped_lock lock(m_Mutex);
    QElementPtr pElem = getFrontElement(bBlock, lock); 
    if (pElem) {
        m_pElements.pop_front();
        m_Cond.notify_one();
    }
    return pElem;
}

template<class QElement>
void Queue<QElement>::clear()
{
    QElementPtr pElem;
    do {
        pElem = pop(false);
    } while (pElem);
}

template<class QElement>
typename Queue<QElement>::QElementPtr Queue<QElement>::peek(bool bBlock) const
{
    scoped_lock lock(m_Mutex);
    QElementPtr pElem = getFrontElement(bBlock, lock); 
    if (pElem) {
        m_Cond.notify_one();
    }
    return pElem;
}

template<class QElement>
void Queue<QElement>::push(const QElementPtr& pElem)
{
    assert(pElem);
    scoped_lock lock(m_Mutex);
    if (m_pElements.size() == (unsigned)m_MaxSize) {
        while (m_pElements.size() == (unsigned)m_MaxSize) {
            m_Cond.wait(lock);
        }
    }
    m_pElements.push_back(pElem);
    m_Cond.notify_one();
}

template<class QElement>
int Queue<QElement>::size() const
{
    scoped_lock lock(m_Mutex);
    return int(m_pElements.size());
}

template<class QElement>
int Queue<QElement>::getMaxSize() const
{
    scoped_lock lock(m_Mutex);
    return m_MaxSize;
}

template<class QElement>
typename Queue<QElement>::QElementPtr 
        Queue<QElement>::getFrontElement(bool bBlock, scoped_lock& lock) const
{
    if (m_pElements.empty()) {
        if (bBlock) {
            while (m_pElements.empty()) {
                m_Cond.wait(lock);
            }
        } else {
            return QElementPtr();
        }
    }
    return m_pElements.front();
}

}
#endif
