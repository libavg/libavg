//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
#include "OGLSurface.h"
#include "TypeDefinition.h"
#include "Canvas.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../graphics/Filterfill.h"
#include "../graphics/TextureMover.h"
#include "../graphics/MCTexture.h"
#include "../graphics/GLContextManager.h"
#include "../graphics/BitmapLoader.h"

#include "../imaging/Camera.h"
#include "../imaging/FWCamera.h"
#include "../imaging/FakeCamera.h"

#include <iostream>
#include <sstream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;
using namespace boost;

namespace avg {

void CameraNode::registerType()
{
    TypeDefinition def = TypeDefinition("camera", "rasternode", 
            ExportedObject::buildObject<CameraNode>)
        .addArg(Arg<string>("driver", "firewire"))
        .addArg(Arg<string>("device", ""))
        .addArg(Arg<int>("unit", -1))
        .addArg(Arg<bool>("fw800", false))
        .addArg(Arg<float>("framerate", 15))
        .addArg(Arg<int>("capturewidth", 640))
        .addArg(Arg<int>("captureheight", 480))
        .addArg(Arg<string>("pixelformat", "RGB"))
        .addArg(Arg<int>("brightness", -1))
        .addArg(Arg<int>("exposure", -1))
        .addArg(Arg<int>("sharpness", -1))
        .addArg(Arg<int>("saturation", -1))
        .addArg(Arg<int>("camgamma", -1))
        .addArg(Arg<int>("shutter", -1))
        .addArg(Arg<int>("gain", -1))
        .addArg(Arg<int>("strobeduration", -1));
    TypeRegistry::get()->registerType(def);
}

CameraNode::CameraNode(const ArgList& args)
    : m_bIsPlaying(false),
      m_FrameNum(0),
      m_bAutoUpdateCameraImage(true),
      m_bNewBmp(false),
      m_bNewSurface(false)
{
    args.setMembers(this);
    string sDriver = args.getArgVal<string>("driver");
    string sDevice = args.getArgVal<string>("device");
    int unit = args.getArgVal<int>("unit");
    bool bFW800 = args.getArgVal<bool>("fw800");
    float frameRate = args.getArgVal<float>("framerate");
    int width = args.getArgVal<int>("capturewidth");
    int height = args.getArgVal<int>("captureheight");
    string sPF = args.getArgVal<string>("pixelformat");

    PixelFormat camPF = stringToPixelFormat(sPF);
    if (camPF == NO_PIXELFORMAT) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Unknown camera pixel format "+sPF+".");
    }
    PixelFormat destPF;
    if (pixelFormatIsColored(camPF)) {
        if (BitmapLoader::get()->isBlueFirst()) {
            destPF = B8G8R8X8;
        } else {
            destPF = R8G8B8X8;
        }
    } else {
        destPF = I8;
    }
//    cerr << "CameraNode ctor: " << camPF << "-->" << destPF << endl;

    m_pCamera = createCamera(sDriver, sDevice, unit, bFW800, IntPoint(width, height), 
            camPF, destPF, frameRate);
    AVG_TRACE(Logger::category::CONFIG,Logger::severity::INFO, "Got Camera " <<
            m_pCamera->getDevice() << " from driver: " << m_pCamera->getDriverName());
    
    m_pCamera->setFeature(CAM_FEATURE_BRIGHTNESS, args.getArgVal<int>("brightness"));
    m_pCamera->setFeature(CAM_FEATURE_EXPOSURE, args.getArgVal<int>("exposure"));
    m_pCamera->setFeature(CAM_FEATURE_SHARPNESS, args.getArgVal<int>("sharpness"));
    m_pCamera->setFeature(CAM_FEATURE_SATURATION, args.getArgVal<int>("saturation"));
    m_pCamera->setFeature(CAM_FEATURE_GAMMA, args.getArgVal<int>("camgamma"));
    m_pCamera->setFeature(CAM_FEATURE_SHUTTER, args.getArgVal<int>("shutter"));
    m_pCamera->setFeature(CAM_FEATURE_GAIN, args.getArgVal<int>("gain"));
    m_pCamera->setFeature(CAM_FEATURE_STROBE_DURATION,
            args.getArgVal<int>("strobeduration"));
}

CameraNode::~CameraNode()
{
    m_pCamera = CameraPtr();
}

void CameraNode::connectDisplay()
{
    RasterNode::connectDisplay();
    if (m_bIsPlaying) {
        open();
    }
}

void CameraNode::connect(CanvasPtr pCanvas)
{
    if (!m_pCamera) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Can't use camera node after disconnect(True).");
    }
    RasterNode::connect(pCanvas);
}

void CameraNode::disconnect(bool bKill)
{
    if (bKill) {
        m_pCamera = CameraPtr();
    }
    RasterNode::disconnect(bKill);
}

void CameraNode::play()
{
    if (getState() == NS_CANRENDER) {
        open();
    }
    m_bIsPlaying = true;
}

void CameraNode::stop()
{
    m_bIsPlaying = false;
}

bool CameraNode::isAvailable()
{
    if (!m_pCamera || boost::dynamic_pointer_cast<FakeCamera>(m_pCamera)) {
        return false;
    } else {
        return true;
    }
}

int CameraNode::getBrightness() const
{
    return getFeature(CAM_FEATURE_BRIGHTNESS);
}

void CameraNode::setBrightness(int value)
{
    setFeature(CAM_FEATURE_BRIGHTNESS, value);
}

int CameraNode::getSharpness() const
{
    return getFeature(CAM_FEATURE_SHARPNESS);
}

void CameraNode::setSharpness(int value)
{
    setFeature(CAM_FEATURE_SHARPNESS, value);
}

int CameraNode::getSaturation() const
{
    return getFeature(CAM_FEATURE_SATURATION);
}

void CameraNode::setSaturation(int value)
{
    setFeature(CAM_FEATURE_SATURATION, value);
}

int CameraNode::getCamGamma() const
{
    return getFeature(CAM_FEATURE_GAMMA);
}

void CameraNode::setCamGamma(int value)
{
    setFeature(CAM_FEATURE_GAMMA, value);
}

int CameraNode::getShutter() const
{
    return getFeature(CAM_FEATURE_SHUTTER);
}

void CameraNode::setShutter(int value)
{
    setFeature(CAM_FEATURE_SHUTTER, value);
}

int CameraNode::getGain() const
{
    return getFeature(CAM_FEATURE_GAIN);
}

void CameraNode::setGain(int value)
{
    setFeature(CAM_FEATURE_GAIN, value);
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

void CameraNode::doOneShotWhitebalance()
{
    // The first line turns off auto white balance.
    m_pCamera->setWhitebalance(m_pCamera->getWhitebalanceU(), 
            m_pCamera->getWhitebalanceV());
    m_pCamera->setFeatureOneShot(CAM_FEATURE_WHITE_BALANCE);
}

int CameraNode::getStrobeDuration() const
{
    return getFeature(CAM_FEATURE_STROBE_DURATION);
}

void CameraNode::setStrobeDuration(int value)
{
    setFeature(CAM_FEATURE_STROBE_DURATION, value);
}
            

IntPoint CameraNode::getMediaSize()
{
    return m_pCamera->getImgSize();
}

BitmapPtr CameraNode::getBitmap()
{
    if (m_pCurBmp) {
        return m_pCurBmp;
    } else {
        throw Exception(AVG_ERR_CAMERA_NONFATAL, 
                "CameraNode.getBitmap: No camera image available.");
    }
}

CamerasInfosVector CameraNode::getCamerasInfos()
{
    CamerasInfosVector camInfos = avg::getCamerasInfos();
    return camInfos;
}

void CameraNode::resetFirewireBus()
{
    FWCamera::resetBus();
}

float CameraNode::getFPS() const
{
    return m_pCamera->getFrameRate();
}

void CameraNode::open()
{
    m_pCamera->startCapture();
    setViewport(-32767, -32767, -32767, -32767);
    PixelFormat pf = getPixelFormat();
    IntPoint size = getMediaSize();
    bool bMipmap = getMaterial().getUseMipmaps();
    
    m_pTex = GLContextManager::get()->createTexture(size, pf, bMipmap);
    getSurface()->create(pf, m_pTex);
    m_bNewSurface = true;
    newSurface();

    setupFX();
}

int CameraNode::getFeature(CameraFeature feature) const
{
    return m_pCamera->getFeature(feature);
}

void CameraNode::setFeature(CameraFeature feature, int value)
{
    m_pCamera->setFeature(feature, value);
}

int CameraNode::getFrameNum() const
{
    return m_FrameNum;
}

static ProfilingZoneID CameraFetchImage("Camera fetch image");
static ProfilingZoneID CameraDownloadProfilingZone("Camera tex download");

void CameraNode::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    if (m_bAutoUpdateCameraImage) {
        ScopeTimer Timer(CameraFetchImage);
        updateToLatestCameraImage();
    }
    if (isVisible()) {
        if (m_bNewBmp) {
            ScopeTimer Timer(CameraDownloadProfilingZone);
            m_FrameNum++;
            GLContextManager::get()->scheduleTexUpload(m_pTex, m_pCurBmp);
            scheduleFXRender();
            m_bNewBmp = false;
        } else if (m_bNewSurface) {
            BitmapPtr pBmp;
            PixelFormat pf = getPixelFormat();
            pBmp = BitmapPtr(new Bitmap(getMediaSize(), pf));
            if (pf == B8G8R8X8 || pf == B8G8R8A8) {
                FilterFill<Pixel32>(Pixel32(0,0,0,255)).applyInPlace(pBmp);
            } else if (pf == I8) {
                FilterFill<Pixel8>(0).applyInPlace(pBmp);
            } 
            GLContextManager::get()->scheduleTexUpload(m_pTex, pBmp);
            scheduleFXRender();
        }
        m_bNewSurface = false;
    }

    calcVertexArray(pVA);
}

static ProfilingZoneID CameraProfilingZone("Camera::render");

void CameraNode::render()
{
    if (m_bIsPlaying) {
        ScopeTimer Timer(CameraProfilingZone);
        blt32();
    }
}

PixelFormat CameraNode::getPixelFormat() 
{
    return m_pCamera->getDestPF();
}

void CameraNode::updateToLatestCameraImage()
{
    BitmapPtr pTmpBmp = m_pCamera->getImage(false);
    while (pTmpBmp) {
        m_bNewBmp = true;
        m_pCurBmp = pTmpBmp;
        pTmpBmp = m_pCamera->getImage(false);
    }
}

void CameraNode::updateCameraImage()
{
    if (!m_bAutoUpdateCameraImage) {
        m_pCurBmp = m_pCamera->getImage(false);
    }
}

void CameraNode::setAutoUpdateCameraImage(bool bVal)
{
    m_bAutoUpdateCameraImage = bVal;
}

bool CameraNode::isImageAvailable() const
{
    return m_pCurBmp.get() != NULL;
}


}
