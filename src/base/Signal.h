//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _Signal_H_
#define _Signal_H_

#include "../api.h"

#include <list>

namespace avg {


// Simple implementation of a signal/slot mechanism.
// Might need to be replaced by boost::signal when things get more complicated.
template <class LISTENEROBJ>
class AVG_TEMPLATE_API Signal {
public:
    typedef void (LISTENEROBJ::*ListenerFunc)() ;
    Signal(ListenerFunc pFunc);
    virtual ~Signal();

    void connect(LISTENEROBJ* pListener);
    void disconnect(LISTENEROBJ* pListener);
    
    void emit();
    int getNumListeners() const;

private:
    ListenerFunc m_pFunc;
    std::list<LISTENEROBJ*> m_Listeners;
    typedef typename std::list<LISTENEROBJ*>::iterator ListenerIterator;
    LISTENEROBJ* m_pCurrentListener;
    bool m_bKillCurrentListener;
};

template<class LISTENEROBJ>
Signal<LISTENEROBJ>::Signal(ListenerFunc pFunc)
    : m_pFunc(pFunc),
      m_pCurrentListener(0),
      m_bKillCurrentListener(false)
{
}

template<class LISTENEROBJ>
Signal<LISTENEROBJ>::~Signal()
{
}

template<class LISTENEROBJ>
void Signal<LISTENEROBJ>::connect(LISTENEROBJ* pListener)
{
    ListenerIterator it;
    it = find(m_Listeners.begin(), m_Listeners.end(), pListener);
    assert(it == m_Listeners.end());
    m_Listeners.push_back(pListener);
}

template<class LISTENEROBJ>
void Signal<LISTENEROBJ>::disconnect(LISTENEROBJ* pListener)
{
    if (m_pCurrentListener == pListener) {
        m_bKillCurrentListener = true;
    } else {
        ListenerIterator it;
        it = find(m_Listeners.begin(), m_Listeners.end(), pListener);
        assert (it != m_Listeners.end());
        m_Listeners.erase(it);
    }
}

template<class LISTENEROBJ>
void Signal<LISTENEROBJ>::emit()
{
    ListenerIterator it;
    for (it=m_Listeners.begin(); it != m_Listeners.end();) {
        m_pCurrentListener = *it;
        ((*it)->*m_pFunc)();   // This is the actual call to the listener.
        if (m_bKillCurrentListener) {
            it = m_Listeners.erase(it);
            m_bKillCurrentListener = false;
        } else {
            ++it;
        }
    }
    m_pCurrentListener = 0;
}

template<class LISTENEROBJ>
int Signal<LISTENEROBJ>::getNumListeners() const
{
    return m_Listeners.size();
}

}
#endif
