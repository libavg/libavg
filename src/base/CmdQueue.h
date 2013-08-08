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

#ifndef _CmdQueue_H_
#define _CmdQueue_H_

#include "Command.h"
#include "Queue.h"

#include "../api.h"

namespace avg {

template<class RECEIVER>
class AVG_TEMPLATE_API CmdQueue: public Queue<Command<RECEIVER> >
{
public:
    CmdQueue(int maxSize=-1);
    typedef typename Queue<Command<RECEIVER> >::QElementPtr CmdPtr;
    void pushCmd(typename Command<RECEIVER>::CmdFunc func);
    
};

template<class RECEIVER>
CmdQueue<RECEIVER>::CmdQueue(int maxSize)
    : Queue<Command<RECEIVER> >(maxSize)
{
}

template<class RECEIVER>
void CmdQueue<RECEIVER>::pushCmd(typename Command<RECEIVER>::CmdFunc func)
{
    this->push(CmdPtr(new Command<RECEIVER>(func)));
}

}

#endif
