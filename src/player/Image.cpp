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

#include "Image.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfliprgb.h"

#include "SDLDisplayEngine.h"
#include "OGLSurface.h"

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Image::Image(OGLSurface * pSurface, const string& sFilename)
    : m_sFilename(sFilename),
      m_pBmp(new Bitmap(IntPoint(1,1), B8G8R8X8)),
      m_pSurface(pSurface),
      m_pEngine(0),
      m_State(NOT_AVAILABLE)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    load();
}

Image::~Image()
{
    if (m_State == GPU) {
        m_pSurface->destroy();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}
        
void Image::setBitmap(const Bitmap * pBmp)
{
    if(!pBmp) {
        throw Exception(AVG_ERR_UNSUPPORTED, "setBitmap(): bitmap must not be None!");
    }
    PixelFormat pf = calcSurfacePF(*pBmp);
    if (m_pEngine) {
        if (m_State == NOT_AVAILABLE || m_pSurface->getSize() != pBmp->getSize() || 
                m_pSurface->getPixelFormat() != pf)
        {
            m_pSurface->create(pBmp->getSize(), pf);
        }            
        BitmapPtr pSurfaceBmp = m_pSurface->lockBmp();
        pSurfaceBmp->copyPixels(*pBmp);
        m_pSurface->unlockBmps();
        m_pBmp=BitmapPtr();
        m_State = GPU;
    } else {
        m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), pf, ""));
        m_pBmp->copyPixels(*pBmp);
        m_State = CPU;
    }
    m_sFilename = "";

}

void Image::moveToGPU(SDLDisplayEngine* pEngine)
{
    m_pEngine = pEngine;
    if (m_pScene) {
        m_pSurface->create(m_pScene->getSize(), B8G8R8X8);
        m_pSurface->setTexID(m_pScene->getTexID());
        m_State = GPU;
    } else {
        if (m_State == CPU) {
            m_State = GPU;
            setupSurface();
        }
    }
}

void Image::moveToCPU()
{
    if (m_State != GPU) {
        return;
    }
    if (!m_pScene) {
        m_pBmp = m_pSurface->readbackBmp();
        m_State = CPU;
    }
    m_pEngine = 0;
    m_pSurface->destroy();
}

void Image::setFilename(const std::string& sFilename)
{
    if (m_State == GPU) {
        m_pSurface->destroy();
    }
    m_pScene = OffscreenScenePtr();
    m_State = NOT_AVAILABLE;
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), B8G8R8X8));
    m_sFilename = sFilename;
    load();
    if (m_pEngine) {
        moveToGPU(m_pEngine);
    }
}

void Image::setScene(OffscreenScenePtr pScene)
{
    if (pScene == m_pScene) {
        return;
    }
    if (!pScene) {
        m_sFilename = "";
        switch(m_State) {
            case CPU:
                m_pBmp = BitmapPtr();
                break;
            case GPU:
                m_pSurface->destroy();
                break;
            default:
                break;
        }
    }
    m_pScene = pScene;
    m_State = CPU;
    if (m_pEngine) {
        m_pSurface->create(m_pScene->getSize(), B8G8R8X8);
        m_pSurface->setTexID(m_pScene->getTexID());
        moveToGPU(m_pEngine);
    }
}

const string& Image::getFilename() const
{
    return m_sFilename;
}

BitmapPtr Image::getBitmap()
{
    if (m_State == GPU) {
        return m_pSurface->readbackBmp();
    } else {
        if (m_pScene) {
            return BitmapPtr();
        } else {
            return BitmapPtr(new Bitmap(*m_pBmp));
        }
    }
}

IntPoint Image::getSize()
{
    if (m_State == GPU) {
        return m_pSurface->getSize();
    } else {
        if (m_pScene) {
            return m_pScene->getSize();
        } else {
            return m_pBmp->getSize();
        }
    }
}

PixelFormat Image::getPixelFormat()
{
    if (m_State == GPU) {
        return m_pSurface->getPixelFormat();
    } else {
        if (m_pScene) {
            return B8G8R8X8;
        } else {
            return m_pBmp->getPixelFormat();
        }
    }
}

OGLSurface* Image::getSurface()
{
    AVG_ASSERT(m_State != CPU);
    return m_pSurface;
}

Image::State Image::getState()
{
    return m_State;
}

SDLDisplayEngine* Image::getEngine()
{
    return m_pEngine;
}

void Image::load()
{
    if (m_sFilename == "") {
        return;
    }
    AVG_TRACE(Logger::MEMORY, "Loading " << m_sFilename);
    m_pBmp = BitmapPtr(new Bitmap(m_sFilename));
    m_State = CPU;
}

void Image::setupSurface()
{
    PixelFormat pf = calcSurfacePF(*m_pBmp);
    m_pSurface->create(m_pBmp->getSize(), pf);
    BitmapPtr pSurfaceBmp = m_pSurface->lockBmp();
    pSurfaceBmp->copyPixels(*m_pBmp);
    m_pSurface->unlockBmps();
    m_pBmp=BitmapPtr();
}

PixelFormat Image::calcSurfacePF(const Bitmap& bmp)
{
    PixelFormat pf;
    pf = B8G8R8X8;
    if (bmp.hasAlpha()) {
        pf = B8G8R8A8;
    }
    if (bmp.getPixelFormat() == I8) {
        pf = I8;
    }
    return pf;
}

}
