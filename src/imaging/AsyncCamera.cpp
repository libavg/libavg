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

#include "AsyncCamera.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

AsyncCamera::AsyncCamera (std::string sDevice, double FrameRate, std::string sMode, bool bColor)
    : m_sDevice(sDevice),
      m_FrameRate(FrameRate),
      m_sMode(sMode),
      m_bColor(bColor)
{
    m_pThread = 0;
}

AsyncCamera::~AsyncCamera()
{
    close();
}

void AsyncCamera::open()
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    m_pThread = new boost::thread(CameraThread(m_BitmapQ, m_CmdQ, m_sDevice, 
            m_FrameRate, m_sMode, m_bColor));
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        m_CmdQ.push(Command<CameraThread>(boost::bind(&CameraThread::setFeature, _1,
            it->first, it->second)));
    }
#endif
}

void AsyncCamera::close() 
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    if (m_pThread) {
        m_CmdQ.push(Command<CameraThread>(boost::bind(&CameraThread::stop, _1)));
        m_pThread->join();
        delete m_pThread;
        m_pThread = 0;
    }
#endif
}

IntPoint AsyncCamera::getImgSize() 
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    return getCamImgSize(getCamMode(m_sMode));
#else
    return IntPoint(640, 480);
#endif
}

BitmapPtr AsyncCamera::getImage(bool bWait) 
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    if (m_pThread) {
        try {
            return m_BitmapQ.pop(bWait);
        } catch (Exception& ex) {
        }
    } 
    return BitmapPtr();
#else
    return BitmapPtr();
#endif
}


bool AsyncCamera::isCameraAvailable()
{
    return m_pThread;
}

const string& AsyncCamera::getDevice() const 
{
    return m_sDevice;
}

double AsyncCamera::getFrameRate() const
{
    return m_FrameRate;
}

const string& AsyncCamera::getMode() const
{
    return m_sMode;
}
        
unsigned int AsyncCamera::getFeature (const std::string& sFeature) const
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    dc1394feature_t FeatureID = getFeatureID(sFeature);
    FeatureMap::const_iterator it = m_Features.find(FeatureID);
    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
#else
    return 0;
#endif
}

void AsyncCamera::setFeature (const std::string& sFeature, int Value)
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    dc1394feature_t FeatureID = getFeatureID(sFeature);
    m_Features[FeatureID] = Value;
    if (m_pThread) {
        m_CmdQ.push(Command<CameraThread>(boost::bind(&CameraThread::setFeature, _1,
            FeatureID, Value)));
    }
#endif
}


}
