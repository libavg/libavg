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

#ifndef _TrackerEventStream_H_
#define _TrackerEventStream_H_

#include "CursorEvent.h"

#include "../base/Point.h"

#include "../imaging/Blob.h"
#include "../imaging/DeDistort.h"

#include <string>

namespace avg {

    class  EventStream
    //internal class to keep track of blob/event states
    {
        public:
            enum StreamState {
                DOWN_PENDING, //fresh stream. not polled yet
                DOWN_DELIVERED, //initial finger down delivered
                MOTION_PENDING, //recent position change
                MOTION_DELIVERED, //finger resting
                VANISHED, // oops, no followup found -- wait a little while
                UP_PENDING, //finger disappeared, but fingerup yet to be delivered
                UP_DELIVERED // waiting to be cleared.
            };

            // State transitions:
            // Current state       Destination state
            // DOWN_PENDING     -> DOWN_DELIVERED (CURSORDOWN event), UP_DELIVERED (spurious blob)
            // DOWN_DELIVERED   -> VANISHED, MOTION_PENDING, MOTION_DELIVERED
            // MOTION_PENDING   -> VANISHED, MOTION_DELIVERED (CURSORMOTION event)
            // MOTION_DELIVERED -> VANISHED, MOTION_PENDING
            // VANISHED         -> MOTION_PENDING, UP_PENDING
            // UP_PENDING       -> UP_DELIVERED (CURSORUP event)

            EventStream(BlobPtr first_blob, long long time);
            virtual ~EventStream();
            void blobChanged(BlobPtr new_blob, long long time, bool bEventOnMove);
            void blobGone();
            EventPtr pollevent(DeDistortPtr trafo, const IntPoint& DisplayExtents, 
                    CursorEvent::Source Source, bool bEventOnMove);
            bool isGone();
            void setStale();
            bool isStale();
            void dump();
            static std::string stateToString(StreamState State);

        private:
            void calcSpeed(DPoint pos, long long newTime);

            bool m_Stale;
            int m_Id;
            StreamState m_State;
            int m_VanishCounter;
            DPoint m_Pos;
            DPoint m_Speed;
            BlobPtr m_pBlob;
            long long m_Time;
            static int s_LastLabel;
    };

}

#endif
