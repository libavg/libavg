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

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

CameraNode::CameraNode()
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    m_pCamera = CameraPtr(new Camera("", 15, "640x480_RGB"));
#else
    AVG_TRACE(Logger::ERROR,
            "Unable to set up camera. Camera support not compiled.");
#endif
}

CameraNode::CameraNode(const xmlNodePtr xmlNode, Player * pPlayer)
    : VideoBase(xmlNode, pPlayer)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    string sDevice = getDefaultedStringAttr (xmlNode, "device", "");
    double FrameRate = getDefaultedDoubleAttr (xmlNode, "framerate", 15);
    string sMode = getDefaultedStringAttr (xmlNode, "mode", "640x480_RGB");

    m_pCamera = CameraPtr(new Camera(sDevice, FrameRate, sMode));
    
/*
    m_pCamera->setFeature ("brightness", getDefaultedIntAttr(xmlNode, "brightness", -1));
    m_pCamera->setFeature ("exposure", getDefaultedIntAttr(xmlNode, "exposure", -1));
    m_pCamera->setFeature ("sharpness", getDefaultedIntAttr(xmlNode, "sharpness", -1));
    m_pCamera->setFeature ("saturation", getDefaultedIntAttr(xmlNode, "saturation", -1));
    m_pCamera->setFeature ("gamma", getDefaultedIntAttr(xmlNode, "gamma", -1));
    m_pCamera->setFeature ("shutter", getDefaultedIntAttr(xmlNode, "shutter", -1));
    m_pCamera->setFeature ("gain", getDefaultedIntAttr(xmlNode, "gain", -1));
    m_pCamera->setFeature ("whitebalance", getDefaultedIntAttr(xmlNode, "whitebalance", -1));
*/
#else
    AVG_TRACE(Logger::ERROR,
            "Unable to set up camera. Camera support not compiled.");
#endif
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

IntPoint CameraNode::getNativeSize() 
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    return m_pCamera->getImgSize();
#else
    return IntPoint(640,480);
#endif
}

double CameraNode::getFPS()
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    return m_pCamera->getFrameRate();
#else
    return 0;
#endif

}

void CameraNode::open(int* pWidth, int* pHeight)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    m_pCamera->open();
    *pWidth = m_pCamera->getImgSize().x;
    *pHeight = m_pCamera->getImgSize().y;
#else
    *pWidth = 640;
    *pHeight = 480;
#endif    
}

void CameraNode::close()
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    m_pCamera->close();
#endif
}

unsigned int CameraNode::getFeature (const std::string& sFeature) const
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    return m_pCamera->getFeature(sFeature);
#else
    return 0;
#endif
}

void CameraNode::setFeature (const std::string& sFeature, int Value)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    m_pCamera->setFeature(sFeature, Value);
#endif
}



static ProfilingZone CameraProfilingZone("    Camera::render");
static ProfilingZone CameraUploadProfilingZone("      Camera tex download");

bool CameraNode::renderToSurface(ISurface * pSurface)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    ScopeTimer Timer(CameraProfilingZone);
    BitmapPtr pCurBmp = m_pCamera->getImage();
    if (pCurBmp) {
        BitmapPtr pTempBmp;
        while (pTempBmp = m_pCamera->getImage()) {
            pCurBmp = pTempBmp;
        }
        BitmapPtr pBmp = pSurface->lockBmp();
        assert(pBmp->getPixelFormat() == pCurBmp->getPixelFormat());
        pBmp->copyPixels(*pCurBmp);
        pSurface->unlockBmps();
        {
            ScopeTimer Timer(CameraUploadProfilingZone);
            getEngine()->surfaceChanged(pSurface);
        }
    }
#endif    
    return true;
}

PixelFormat CameraNode::getDesiredPixelFormat() 
{
    return B8G8R8X8;
}

bool CameraNode::canRenderToBackbuffer(int BitsPerPixel)
{
    return false;
}


}
