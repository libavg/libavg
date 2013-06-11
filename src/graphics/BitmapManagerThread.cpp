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

#include "BitmapManagerThread.h"

#include "Bitmap.h"
#include "BitmapLoader.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"

#include <stdio.h>
#include <stdlib.h>

namespace avg {

BitmapManagerThread::BitmapManagerThread(CQueue& cmdQ, BitmapManagerMsgQueue& MsgQueue)
    : WorkerThread<BitmapManagerThread>("BitmapManager", cmdQ),
      m_MsgQueue(MsgQueue),
      m_TotalLatency(0),
      m_NumBmpsLoaded(0)
{
}

bool BitmapManagerThread::work()
{
    waitForCommand();
    return true;
}

void BitmapManagerThread::deinit()
{
    if (m_NumBmpsLoaded > 0) {
        AVG_TRACE(Logger::category::PROFILE, Logger::severity::INFO,
                "Average latency for async bitmap loads: " << m_TotalLatency/m_NumBmpsLoaded
                << " ms");
    }
}

static ProfilingZoneID LoaderProfilingZone("loadBitmap", true);

void BitmapManagerThread::loadBitmap(BitmapManagerMsgPtr pRequest)
{
    BitmapPtr pBmp;
    ScopeTimer timer(LoaderProfilingZone);
    float startTime = pRequest->getStartTime();
    try {
        pBmp = avg::loadBitmap(pRequest->getFilename());
        pRequest->setBitmap(pBmp);
    } catch (const Exception& ex) {
        pRequest->setError(ex);
    }
    m_MsgQueue.push(pRequest);
    m_NumBmpsLoaded++;
    float curLatency = TimeSource::get()->getCurrentMicrosecs()/1000 - startTime;
    m_TotalLatency += curLatency;
    ThreadProfiler::get()->reset();
}

}
