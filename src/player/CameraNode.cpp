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
#include "../imaging/FakeCamera.h"
#ifdef AVG_ENABLE_V4L2
#include "../imaging/V4LCamera.h"
#endif
#ifdef AVG_ENABLE_CMU1394
#include "../imaging/CMUCamera.h"
#endif
#ifdef AVG_ENABLE_DSHOW
#include "../imaging/DSCamera.h"
#endif

#include <iostream>
#include <sstream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;

namespace avg {

NodeDefinition CameraNode::createDefinition()
{
    return NodeDefinition("camera", Node::buildNode<CameraNode>)
        .extendDefinition(VideoBase::createDefinition())
        .addArg(Arg<string>("device", ""))
        .addArg(Arg<double>("framerate", 15))
        .addArg(Arg<string>("source", "firewire"))
        .addArg(Arg<int>("capturewidth", 640))
        .addArg(Arg<int>("captureheight", 480))
        .addArg(Arg<string>("pixelformat", "RGB"))
        .addArg(Arg<int>("channel", 0))
        .addArg(Arg<int>("brightness", -1))
        .addArg(Arg<int>("exposure", -1))
        .addArg(Arg<int>("sharpness", -1))
        .addArg(Arg<int>("saturation", -1))
        .addArg(Arg<int>("gamma", -1))
        .addArg(Arg<int>("shutter", -1))
        .addArg(Arg<int>("gain", -1))
        .addArg(Arg<int>("strobeduration", -1));
}

CameraNode::CameraNode(const ArgList& Args, bool bFromXML)
    : m_FrameNum(0)
{
    Args.setMembers(this);
    string sDevice = Args.getArgVal<string>("device");
    double FrameRate = Args.getArgVal<double>("framerate");
    string sSource = Args.getArgVal<string>("source");
    int Width = Args.getArgVal<int>("capturewidth");
    int Height = Args.getArgVal<int>("captureheight");
    string sPF = Args.getArgVal<string>("pixelformat");

    if (sSource == "firewire") {
#if defined(AVG_ENABLE_1394)\
    || defined(AVG_ENABLE_1394_2)
    m_pCamera = CameraPtr(new FWCamera(sDevice, IntPoint(Width, Height), sPF, 
            FrameRate, true));
#elif defined(AVG_ENABLE_CMU1394)
    m_pCamera = CameraPtr(new CMUCamera(sDevice, IntPoint(Width, Height), sPF, 
            FrameRate, true));
#else
        AVG_TRACE(Logger::WARNING, "Firewire camera specified, but firewire "
                "support not compiled in.");
#endif
    } else if (sSource == "v4l") {
#if defined(AVG_ENABLE_V4L2)
        int Channel = Args.getArgVal<int>("channel");
        
        m_pCamera = CameraPtr(new V4LCamera(sDevice, Channel,
            IntPoint(Width, Height), sPF, true));
#else
        AVG_TRACE(Logger::WARNING, "Video4Linux camera specified, but "
                "Video4Linux support not compiled in.");
#endif
    } else if (sSource == "directshow") {
#if defined(AVG_ENABLE_DSHOW)
        m_pCamera = CameraPtr(new DSCamera(sDevice, IntPoint(Width, Height), sPF, 
            FrameRate, true));
#else
        AVG_TRACE(Logger::WARNING, "DirectShow camera specified, but "
                "DirectShow is only available under windows.");
#endif
    } else {
        throw Exception(AVG_ERR_INVALID_ARGS,
                "Unable to set up camera. Camera source '"+sSource+"' unknown.");
    }

    if (!m_pCamera) {
        m_pCamera = CameraPtr(new FakeCamera());
    }

    m_pCamera->setFeature(CAM_FEATURE_BRIGHTNESS,
            Args.getArgVal<int>("brightness"));
    m_pCamera->setFeature(CAM_FEATURE_EXPOSURE,
            Args.getArgVal<int>("exposure"));
    m_pCamera->setFeature(CAM_FEATURE_SHARPNESS,
            Args.getArgVal<int>("sharpness"));
    m_pCamera->setFeature(CAM_FEATURE_SATURATION,
            Args.getArgVal<int>("saturation"));
    m_pCamera->setFeature(CAM_FEATURE_GAMMA,
            Args.getArgVal<int>("gamma"));
    m_pCamera->setFeature(CAM_FEATURE_SHUTTER,
            Args.getArgVal<int>("shutter"));
    m_pCamera->setFeature(CAM_FEATURE_GAIN,
            Args.getArgVal<int>("gain"));
    m_pCamera->setFeature(CAM_FEATURE_STROBE_DURATION,
            Args.getArgVal<int>("strobeduration"));
}

CameraNode::~CameraNode()
{
    close();
}

void CameraNode::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    VideoBase::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

int CameraNode::getBrightness() const
{
    return getFeature(CAM_FEATURE_BRIGHTNESS);
}

void CameraNode::setBrightness(int Value)
{
    setFeature(CAM_FEATURE_BRIGHTNESS, Value);
}

int CameraNode::getExposure() const
{
    return getFeature(CAM_FEATURE_EXPOSURE);
}

void CameraNode::setExposure(int Value)
{
    setFeature(CAM_FEATURE_EXPOSURE, Value);
}

int CameraNode::getSharpness() const
{
    return getFeature(CAM_FEATURE_SHARPNESS);
}

void CameraNode::setSharpness(int Value)
{
    setFeature(CAM_FEATURE_SHARPNESS, Value);
}

int CameraNode::getSaturation() const
{
    return getFeature(CAM_FEATURE_SATURATION);
}

void CameraNode::setSaturation(int Value)
{
    setFeature(CAM_FEATURE_SATURATION, Value);
}

int CameraNode::getGamma() const
{
    return getFeature(CAM_FEATURE_GAMMA);
}

void CameraNode::setGamma(int Value)
{
    setFeature(CAM_FEATURE_GAMMA, Value);
}

int CameraNode::getShutter() const
{
    return getFeature(CAM_FEATURE_SHUTTER);
}

void CameraNode::setShutter(int Value)
{
    setFeature(CAM_FEATURE_SHUTTER, Value);
}

int CameraNode::getGain() const
{
    return getFeature(CAM_FEATURE_GAIN);
}

void CameraNode::setGain(int Value)
{
    setFeature(CAM_FEATURE_GAIN, Value);
}

int CameraNode::getWhitebalanceU() const
{
    return m_pCamera->getWhitebalanceU();
}

int CameraNode::getWhitebalanceV() const
{
    return m_pCamera->getWhitebalanceV();
}

void CameraNode::setWhitebalance(int u, int v)
{
    m_pCamera->setWhitebalance(u, v);
}

int CameraNode::getStrobeDuration() const
{
    return getFeature(CAM_FEATURE_STROBE_DURATION);
}

void CameraNode::setStrobeDuration(int Value)
{
    setFeature(CAM_FEATURE_STROBE_DURATION, Value);
}
            

IntPoint CameraNode::getMediaSize()
{
    return m_pCamera->getImgSize();
}

double CameraNode::getFPS()
{
    return m_pCamera->getFrameRate();
}

void CameraNode::open(YCbCrMode ycbcrMode)
{
    m_pCamera->open();
}

void CameraNode::close()
{
    m_pCamera->close();
}

int CameraNode::getFeature(CameraFeature Feature) const
{
    return m_pCamera->getFeature(Feature);
}

void CameraNode::setFeature(CameraFeature Feature, int Value)
{
    m_pCamera->setFeature(Feature, Value);
}

int CameraNode::getFrameNum() const
{
    return m_FrameNum;
}

static ProfilingZone CameraFetchImage("Camera fetch image");

void CameraNode::preRender()
{
    ScopeTimer Timer(CameraFetchImage);
    m_pCurBmp = m_pCamera->getImage(false);
    if (m_pCurBmp) {
        BitmapPtr pTempBmp;
        while (pTempBmp = m_pCamera->getImage(false)) {
            m_pCurBmp = pTempBmp;
        }
        m_FrameNum++;
    }
}

static ProfilingZone CameraProfilingZone("Camera::render");
static ProfilingZone CameraDownloadProfilingZone("Camera tex download");

bool CameraNode::renderToSurface(ISurface * pSurface)
{
    ScopeTimer Timer(CameraProfilingZone);
    if (m_pCurBmp) {
        BitmapPtr pBmp = pSurface->lockBmp();
        assert(pBmp->getPixelFormat() == m_pCurBmp->getPixelFormat());
        pBmp->copyPixels(*m_pCurBmp);
        pSurface->unlockBmps();
        {
            ScopeTimer Timer(CameraDownloadProfilingZone);
            getDisplayEngine()->surfaceChanged(pSurface);
        }
    }
    return true;
}

PixelFormat CameraNode::getPixelFormat() 
{
    return B8G8R8X8;
}


}
