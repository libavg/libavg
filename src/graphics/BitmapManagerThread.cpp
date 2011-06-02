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

#include <stdio.h>
#include <stdlib.h>

namespace avg {

static ProfilingZoneID ProfilingZoneLoadBitmap("BitmapManager loadBitmap");

BitmapManagerThread::BitmapManagerThread(CQueue& cmdQ,
        BitmapManagerMsgQueue& MsgQueue)
        : WorkerThread<BitmapManagerThread>("BitmapManager", cmdQ),
        m_MsgQueue(MsgQueue)
{
}

bool BitmapManagerThread::work()
{
    waitForCommand();
    return true;
}

void BitmapManagerThread::loadBitmap(BitmapManagerMsgPtr pRequest)
{
    BitmapPtr pBmp;

    try {
        pBmp = BitmapPtr(new Bitmap(pRequest->getFilename()));
        pRequest->setBitmap(pBmp);
    } catch (const std::exception& e) {
        pRequest->setError(Exception(AVG_ERR_FILEIO, e.what()));
    }

    m_MsgQueue.push(pRequest);
}

}
