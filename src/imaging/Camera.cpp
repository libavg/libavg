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

#include "Camera.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

Camera::Camera (std::string sDevice, double FrameRate, std::string sMode)
    : m_sDevice(sDevice),
      m_FrameRate(FrameRate),
      m_sMode(sMode)
{
    m_pThread = 0;
}

Camera::~Camera()
{
    close();
}

void Camera::open()
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    m_pThread = new boost::thread(CameraThread(m_BitmapQ, m_CmdQ, m_sDevice, 
            getCamMode(m_sMode), m_FrameRate));
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        m_CmdQ.push(CameraCmd(CameraCmd::FEATURE, it->first, it->second));
    }
#endif
}

void Camera::close() 
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    if (m_pThread) {
        m_CmdQ.push(CameraCmd(CameraCmd::STOP));
        m_pThread->join();
        delete m_pThread;
        m_pThread = 0;
    }
#endif
}

IntPoint Camera::getImgSize() 
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    return getCamImgSize(getCamMode(m_sMode));
#else
    return IntPoint(640, 480);
#endif
}

BitmapPtr Camera::getImage() 
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    if (m_pThread) {
        try {
            return m_BitmapQ.pop(false);
        } catch (Exception& ex) {
        }
    } 
    return BitmapPtr();
#else
    return BitmapPtr();
#endif
}


bool Camera::isCameraAvailabe()
{
    return m_pThread;
}

const string& Camera::getDevice() const 
{
    return m_sDevice;
}

double Camera::getFrameRate() const
{
    return m_FrameRate;
}

const string& Camera::getMode() const
{
    return m_sMode;
}
        
unsigned int Camera::getFeature (const std::string& sFeature) const
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

void Camera::setFeature (const std::string& sFeature, int Value)
{
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
    dc1394feature_t FeatureID = getFeatureID(sFeature);
    m_Features[FeatureID] = Value;
    if (m_pThread) {
        m_CmdQ.push(CameraCmd(CameraCmd::FEATURE, FeatureID, Value));
    }
#endif
}


}
