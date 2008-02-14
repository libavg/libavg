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

#include "CameraNode.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "ISurface.h"
#include "NodeDefinition.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../imaging/FWCamera.h"
#ifdef AVG_ENABLE_V4L2
#include "../imaging/V4LCamera.h"
#endif
#ifdef _WIN32
#include "../imaging/DSCamera.h"
#endif

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

NodeDefinition CameraNode::getNodeDefinition()
{
    return NodeDefinition("camera", Node::buildNode<CameraNode>)
        .extendDefinition(VideoBase::getNodeDefinition())
        .addArg("device", "", true)
        .addArg("framerate", "15")
        .addArg("source", "firewire")
        .addArg("capturewidth", "640")
        .addArg("captureheight", "480")
        .addArg("pixelformat", "RGB")
        .addArg("channel", "0")
        .addArg("brightness", "-1")
        .addArg("exposure", "-1")
        .addArg("sharpness", "-1")
        .addArg("saturation", "-1")
        .addArg("gamma", "-1")
        .addArg("shutter", "-1")
        .addArg("gain", "-1")
        .addArg("whitebalance", "-1");
}

CameraNode::CameraNode(const ArgList& Args, Player * pPlayer)
    : VideoBase(Args, pPlayer),
      m_FrameNum(0)
{
    string sDevice = Args.getStringArg ("device");
    double FrameRate = Args.getDoubleArg ("framerate");
    string sSource = Args.getStringArg ("source");
    int Width = Args.getIntArg ("capturewidth");
    int Height = Args.getIntArg ("captureheight");
    string sPF = Args.getStringArg ("pixelformat");

    if (sSource == "firewire") {
#if defined(AVG_ENABLE_1394)\
    || defined(AVG_ENABLE_1394_2)
    m_pCamera = CameraPtr(new FWCamera(sDevice, IntPoint(Width, Height), sPF, 
            FrameRate, true));
#else
        AVG_TRACE(Logger::ERROR, "Firewire camera specified, but firewire "
                "support not compiled in.");
#endif
    } else if (sSource == "v4l") {
#if defined(AVG_ENABLE_V4L2)
        int Channel = Args.getIntArg ("channel");
        
        m_pCamera = CameraPtr(new V4LCamera(sDevice, Channel,
            IntPoint(Width, Height), sPF, true));
#else
        AVG_TRACE(Logger::ERROR, "Video4Linux camera specified, but "
                "Video4Linux support not compiled in.");
#endif
    } else if (sSource == "directshow") {
#if defined(_WIN32)
        m_pCamera = CameraPtr(new DSCamera(sDevice, IntPoint(Width, Height), sPF, 
            FrameRate, true));
#else
        AVG_TRACE(Logger::ERROR, "DirectShow camera specified, but "
                "DirectShow is only available under windows.");
#endif
    } else {
        AVG_TRACE(Logger::ERROR,
            "Unable to set up camera. Camera source '"+sSource+"' unknown.");
    }

    if (m_pCamera) {
        m_pCamera->setFeature (CAM_FEATURE_BRIGHTNESS,
            Args.getIntArg("brightness"));
        m_pCamera->setFeature (CAM_FEATURE_EXPOSURE,
            Args.getIntArg("exposure"));
        m_pCamera->setFeature (CAM_FEATURE_SHARPNESS,
            Args.getIntArg("sharpness"));
        m_pCamera->setFeature (CAM_FEATURE_SATURATION,
            Args.getIntArg("saturation"));
        m_pCamera->setFeature (CAM_FEATURE_GAMMA,
            Args.getIntArg("gamma"));
        m_pCamera->setFeature (CAM_FEATURE_SHUTTER,
            Args.getIntArg("shutter"));
        m_pCamera->setFeature (CAM_FEATURE_GAIN,
            Args.getIntArg("gain"));
        m_pCamera->setFeature (CAM_FEATURE_WHITE_BALANCE,
            Args.getIntArg("whitebalance"));
    }
}

CameraNode::~CameraNode()
{
    close();
}

void CameraNode::setDisplayEngine(DisplayEngine * pEngine)
{
    VideoBase::setDisplayEngine(pEngine);
}

string CameraNode::getTypeStr()
{
    return "Camera";
}

int CameraNode::getBrightness() const
{
    return getFeature (CAM_FEATURE_BRIGHTNESS);
}

void CameraNode::setBrightness(int Value)
{
    setFeature (CAM_FEATURE_BRIGHTNESS, Value);
}

int CameraNode::getExposure() const
{
    return getFeature (CAM_FEATURE_EXPOSURE);
}

void CameraNode::setExposure(int Value)
{
    setFeature (CAM_FEATURE_EXPOSURE, Value);
}

int CameraNode::getSharpness() const
{
    return getFeature (CAM_FEATURE_SHARPNESS);
}

void CameraNode::setSharpness(int Value)
{
    setFeature (CAM_FEATURE_SHARPNESS, Value);
}

int CameraNode::getSaturation() const
{
    return getFeature (CAM_FEATURE_SATURATION);
}

void CameraNode::setSaturation(int Value)
{
    setFeature (CAM_FEATURE_SATURATION, Value);
}

int CameraNode::getGamma() const
{
    return getFeature (CAM_FEATURE_GAMMA);
}

void CameraNode::setGamma(int Value)
{
    setFeature (CAM_FEATURE_GAMMA, Value);
}

int CameraNode::getShutter() const
{
    return getFeature (CAM_FEATURE_SHUTTER);
}

void CameraNode::setShutter(int Value)
{
    setFeature (CAM_FEATURE_SHUTTER, Value);
}

int CameraNode::getGain() const
{
    return getFeature (CAM_FEATURE_GAIN);
}

void CameraNode::setGain(int Value)
{
    setFeature (CAM_FEATURE_GAIN, Value);
}

unsigned int CameraNode::getWhiteBalance() const
{
    return getFeature (CAM_FEATURE_WHITE_BALANCE);
}

void CameraNode::setWhiteBalance(int Value)
{
    setFeature (CAM_FEATURE_WHITE_BALANCE, Value);
}
            

IntPoint CameraNode::getMediaSize() 
{
    if (m_pCamera) {
        return m_pCamera->getImgSize();
    } else {
        return IntPoint(640,480);
    }
}

double CameraNode::getFPS()
{
    if (m_pCamera) {
        return m_pCamera->getFrameRate();
    } else {
        return 0;
    }
}

void CameraNode::open(YCbCrMode ycbcrMode)
{
    if (m_pCamera) {
        m_pCamera->open();
    }
}

void CameraNode::close()
{
    if (m_pCamera) {
        m_pCamera->close();
    }
}

unsigned int CameraNode::getFeature(CameraFeature Feature) const
{
    if (m_pCamera) {
        return m_pCamera->getFeature(Feature);
    } else {
        return 0;
    }
}

void CameraNode::setFeature(CameraFeature Feature, int Value)
{
    if (m_pCamera) {
        m_pCamera->setFeature(Feature, Value);
    }  
}

int CameraNode::getFrameNum() const
{
    return m_FrameNum;
}

static ProfilingZone CameraProfilingZone("Camera::render");
static ProfilingZone CameraUploadProfilingZone("Camera tex download");

bool CameraNode::renderToSurface(ISurface * pSurface)
{
    if (m_pCamera) {
        ScopeTimer Timer(CameraProfilingZone);
        BitmapPtr pCurBmp = m_pCamera->getImage(false);
        if (pCurBmp) {
            BitmapPtr pTempBmp;
            while (pTempBmp = m_pCamera->getImage(false)) {
                pCurBmp = pTempBmp;
            }
            m_FrameNum++;
            BitmapPtr pBmp = pSurface->lockBmp();
            assert(pBmp->getPixelFormat() == pCurBmp->getPixelFormat());
            pBmp->copyPixels(*pCurBmp);
            pSurface->unlockBmps();
            {
                ScopeTimer Timer(CameraUploadProfilingZone);
                getEngine()->surfaceChanged(pSurface);
            }
        }
    }
    return true;
}

PixelFormat CameraNode::getPixelFormat() 
{
    return B8G8R8X8;
}


}
