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

#include "CameraThread.h"
#include "CameraUtils.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"

namespace avg {

#ifdef AVG_ENABLE_1394
#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 1
#define NUM_BUFFERS 3
#endif

using namespace std;

CameraThread::CameraThread(BitmapQueue& BitmapQ, CmdQueue& CmdQ, 
        string sDevice, double FrameRate, string sMode, bool bColor)
    : WorkerThread<CameraThread>(CmdQ),
      m_Camera(sDevice, FrameRate, sMode, bColor),
      m_BitmapQ(BitmapQ)
{
}

bool CameraThread::init()
{
    m_Camera.open();
    return true;
}

bool CameraThread::work()
{
    if (m_Camera.isCameraAvailable()) {
        captureImage();
    } else {
        TimeSource::get()->msleep(100);
        init();
    }
    return true;
}

void CameraThread::deinit()
{
    m_Camera.close();
}

void CameraThread::captureImage()
{
    BitmapPtr pCurBitmap;
    pCurBitmap = m_Camera.getImage(true);
    if (pCurBitmap) {
        m_BitmapQ.push(pCurBitmap);
    } else {
        AVG_TRACE(Logger::WARNING,
                "Camera: Frame capture failed.");
        deinit();
    }
}

void CameraThread::setFeature(dc1394feature_t Feature, int Value) {
    m_Camera.setFeature(Feature, Value);
}
    
}
