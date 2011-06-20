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

#include "TrackerTouchStatus.h"
#include "TouchEvent.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <math.h>

using namespace std;

namespace avg {
    
int TrackerTouchStatus::s_LastID = 0;

TrackerTouchStatus::TrackerTouchStatus(BlobPtr pFirstBlob, long long time, 
        DeDistortPtr pDeDistort, const DRect& displayROI, Event::Source source)
    : TouchStatus(createEvent(source, Event::CURSORDOWN, ++s_LastID, pFirstBlob, time,
              pDeDistort, displayROI)),
      m_Source(source),
      m_pDeDistort(pDeDistort),
      m_DisplayROI(displayROI),
      m_Stale(false),
      m_bGone(false),
      m_ID(s_LastID),
      m_pBlob(pFirstBlob),
      m_LastTime(time),
      m_LastCenter(pFirstBlob->getCenter())
{
    AVG_ASSERT(m_Source == Event::TOUCH || m_Source == Event::TRACK);
    ObjectCounter::get()->incRef(&typeid(*this));
}

TrackerTouchStatus::~TrackerTouchStatus()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void TrackerTouchStatus::blobChanged(BlobPtr pNewBlob, long long time, bool bKeepEvent)
{
    AVG_ASSERT(m_pBlob);
    AVG_ASSERT(pNewBlob);
    if (!m_bGone) {
        DPoint c = pNewBlob->getCenter();
        bool bPosChanged;
        if (bKeepEvent) {
            bPosChanged = true;
        } else {
            bPosChanged = (calcDist(c, m_LastCenter) > 1);
        }
        if (bPosChanged) {
            m_LastCenter = pNewBlob->getCenter();
            TouchEventPtr pEvent = createEvent(Event::CURSORMOTION, pNewBlob, time);
            pushEvent(pEvent, false);
        }
        m_pBlob = pNewBlob;
        m_Stale = false;
        m_LastTime = time;
    }
};
    
void TrackerTouchStatus::blobGone()
{
    if (!m_bGone) {
        TouchEventPtr pEvent = createEvent(Event::CURSORUP, m_pBlob, m_LastTime+1);
        pushEvent(pEvent, false);
        m_bGone = true;
    }
}

void TrackerTouchStatus::setStale()
{
    m_Stale = true;
}

bool TrackerTouchStatus::isStale() 
{
    return m_Stale;
}

TouchEventPtr TrackerTouchStatus::createEvent(Event::Source source, Event::Type type, 
        int id, BlobPtr pBlob, long long time, DeDistortPtr pDeDistort, 
        const DRect& displayROI)
{
    DPoint blobOffset = pDeDistort->getActiveBlobArea(displayROI).tl;
    DPoint pt = pBlob->getCenter() + blobOffset;
    DPoint screenpos = pDeDistort->transformBlobToScreen(pt);
    IntPoint pos(int(screenpos.x+0.5), int(screenpos.y+0.5)); 

    return TouchEventPtr(new TouchEvent(id, type, pBlob, pos, source));
}

TouchEventPtr TrackerTouchStatus::createEvent(Event::Type type, BlobPtr pBlob, 
        long long time)
{
    return createEvent(m_Source, type, m_ID, pBlob, time, m_pDeDistort, m_DisplayROI);
}

}
