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
#include "CameraUtils.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 1
#define NUM_BUFFERS 3

Camera::Camera (std::string sDevice, double FrameRate, std::string sMode)
    : m_sDevice(sDevice),
      m_FrameRate(FrameRate),
      m_sMode(sMode)
{
    m_pThread = 0;
}

Camera::~Camera()
{
}

void Camera::open()
{
    m_pThread = new boost::thread(CameraThread(m_BitmapQ, m_CmdQ, m_sDevice, 
            getCamMode(m_sMode), m_FrameRate));
}

void Camera::close() 
{
    if (m_pThread) {
        m_CmdQ.push(STOP);
        m_pThread->join();
        delete m_pThread;
        m_pThread = 0;
    }
}

IntPoint Camera::getImgSize() 
{
    return getCamImgSize(getCamMode(m_sMode));
}

BitmapPtr Camera::getImage() 
{
    if (m_pThread && !m_BitmapQ.empty()) {
        return m_BitmapQ.pop();
    } else {
        return BitmapPtr();
    }
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
    /*
#ifdef AVG_ENABLE_1394
    int FeatureID = getFeatureID(sFeature);
    unsigned int Value;
    int err;
    if (FeatureID == FEATURE_WHITE_BALANCE) {
        unsigned int u_b_value = 0;
        unsigned int v_r_value = 0;
        err = dc1394_get_white_balance(m_FWHandle, m_Camera.node, &u_b_value, &v_r_value);
        Value = ((u_b_value & 0xff) << 8) | (v_r_value & 0xff);
    } else {
        err = dc1394_get_feature_value(m_FWHandle, m_Camera.node, FeatureID, &Value);
    }
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to get " << sFeature << 
                ". Error was " << err);
    }
    return Value;
#else
*/    
    return 0;
}

void Camera::setFeature (const std::string& sFeature, int Value)
{
/*
#ifdef AVG_ENABLE_1394
    int FeatureID = getFeatureID(sFeature);
    m_Features[FeatureID] = Value;
    if (m_bCameraAvailable) {
        setFeature(FeatureID);
    }
#endif
*/
}

void Camera::setFeature(int FeatureID)
{
/*
#ifdef AVG_ENABLE_1394
    if (m_bCameraAvailable && m_FWHandle != 0) {
        int Value = m_Features[FeatureID];
        if (Value == -1) {
            dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 1);
        } else {
            dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 0);
            int err;
            if (FeatureID == FEATURE_WHITE_BALANCE) {
                unsigned int u_b_value = (Value >> 8) & 0xff;
                unsigned int v_r_value = Value & 0xff;
                err = dc1394_set_white_balance(m_FWHandle, m_Camera.node, 
                        u_b_value, v_r_value);
            } else {
                err = dc1394_set_feature_value(m_FWHandle, m_Camera.node, FeatureID, 
                        (unsigned int)Value);
            } 
            if (err != DC1394_SUCCESS) {
                AVG_TRACE(Logger::WARNING, "Camera: Unable to set " << FeatureID << 
                        ". Error was " << err);
            }
        }
    }
#endif
*/    
}

}
