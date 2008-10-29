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

#ifndef _Timeout_H_
#define _Timeout_H_

// Python docs say python.h should be included before any standard headers (!)
#include "../api.h"
#include "WrapPython.h" 

namespace avg {

class AVG_API Timeout
{
    public:
        Timeout (int time, PyObject * pyfunc, bool isInterval, long long StartTime);
        virtual ~Timeout ();

        bool IsReady(long long Time) const;
        bool IsInterval() const;
        void Fire(long long CurTime);
        int GetID() const;
        bool operator <(const Timeout& other) const;

    private:
        long long m_Interval;
        long long m_NextTimeout;
        PyObject * m_PyFunc;
        bool m_IsInterval;
        int m_ID;
        static int s_LastID;
};

}

#endif //_Timeout_H_
