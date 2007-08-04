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

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../imaging/FWCamera.h"
#ifdef AVG_ENABLE_V4L2
#include "../imaging/V4LCamera.h"
#endif

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

CameraNode::CameraNode(const xmlNodePtr xmlNode, Player * pPlayer)
    : VideoBase(xmlNode, pPlayer),
      m_FrameNum(0)
{
    string sDevice = getDefaultedStringAttr (xmlNode, "device", "");
    double FrameRate = getDefaultedDoubleAttr (xmlNode, "framerate", 15);
    string sMode = getDefaultedStringAttr (xmlNode, "mode", "640x480_RGB");
    string sSource = getDefaultedStringAttr (xmlNode, "source", "firewire");

    if (sSource == "firewire") {
#if defined(AVG_ENABLE_1394)\
    || defined(AVG_ENABLE_1394_2)
    m_pCamera = CameraPtr(new FWCamera(sDevice, FrameRate, sMode, true));
#else
        AVG_TRACE(Logger::ERROR, "Firewire camera specified, but firewire \
            support not compiled in.");
#endif
    } else if (sSource == "v4l") {
#if defined(AVG_ENABLE_V4L2)
        int Channel = getDefaultedIntAttr (xmlNode, "channel", 0);
        int Width = getDefaultedIntAttr (xmlNode, "capturewidth", 640);
        int Height = getDefaultedIntAttr (xmlNode, "captureheight", 480);
        string sPF = getDefaultedStringAttr (xmlNode, "pixelformat", "RGB");
        
        m_pCamera = CameraPtr(new V4LCamera(sDevice, Channel,
            IntPoint(Width, Height), sPF, true));
#else
        AVG_TRACE(Logger::ERROR, "Video4Linux camera specified, but \
            Video4Linux support not compiled in.");
#endif
    } else {
        AVG_TRACE(Logger::ERROR,
            "Unable to set up camera. Camera source '"+sSource+"' unknown.");
    }

    if (m_pCamera) {
        m_pCamera->setFeature ("brightness",
            getDefaultedIntAttr(xmlNode, "brightness", -1));
        m_pCamera->setFeature ("exposure",
            getDefaultedIntAttr(xmlNode, "exposure", -1));
        m_pCamera->setFeature ("sharpness",
            getDefaultedIntAttr(xmlNode, "sharpness", -1));
        m_pCamera->setFeature ("saturation",
            getDefaultedIntAttr(xmlNode, "saturation", -1));
        m_pCamera->setFeature ("gamma",
            getDefaultedIntAttr(xmlNode, "gamma", -1));
        m_pCamera->setFeature ("shutter",
            getDefaultedIntAttr(xmlNode, "shutter", -1));
        m_pCamera->setFeature ("gain",
            getDefaultedIntAttr(xmlNode, "gain", -1));
        m_pCamera->setFeature ("whitebalance",
            getDefaultedIntAttr(xmlNode, "whitebalance", -1));
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

IntPoint CameraNode::getSize() 
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
        m_pCamera = CameraPtr();
    }
}

unsigned int CameraNode::getFeature (const std::string& sFeature) const
{
    if (m_pCamera) {
        return m_pCamera->getFeature(sFeature);
    } else {
        return 0;
    }
}

void CameraNode::setFeature (const std::string& sFeature, int Value)
{
    if (m_pCamera) {
        m_pCamera->setFeature(sFeature, Value);
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
