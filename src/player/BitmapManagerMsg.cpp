//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "BitmapManagerMsg.h"
#include "IBitmapLoadedListener.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/TimeSource.h"


namespace avg {

BitmapManagerMsg::BitmapManagerMsg(const UTF8String& sFilename,
        const boost::python::object& onLoadedCb, PixelFormat pf) 
{
    ObjectCounter::get()->incRef(&typeid(*this));
    init(sFilename, pf);
    m_OnLoadedCb = onLoadedCb;
    m_pLoadedListener = 0;
}

BitmapManagerMsg::BitmapManagerMsg(const UTF8String& sFilename,
        IBitmapLoadedListener* pLoadedListener, PixelFormat pf)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    init(sFilename, pf);
    m_OnLoadedCb = boost::python::object();
    m_pLoadedListener = pLoadedListener;
}

BitmapManagerMsg::~BitmapManagerMsg()
{
    if (m_pEx) {
        delete m_pEx;
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void BitmapManagerMsg::init(const UTF8String& sFilename, PixelFormat pf)
{
    m_sFilename = sFilename;
    m_StartTime = TimeSource::get()->getCurrentMicrosecs()/1000.0f;
    m_PF = pf;
    m_MsgType = REQUEST;
    m_pEx = 0;
}

void BitmapManagerMsg::executeCallback()
{
    switch (m_MsgType) {
        case BITMAP:
            if (m_pLoadedListener) {
                m_pLoadedListener->onBitmapLoaded(m_pBmp);
            } else {
                boost::python::call<void>(m_OnLoadedCb.ptr(), m_pBmp);
            }
            break;
        case ERROR:
            if (m_pLoadedListener) {
                m_pLoadedListener->onBitmapLoadError(m_pEx);
            } else {
                boost::python::call<void>(m_OnLoadedCb.ptr(), m_pEx);
            }
            break;
        
        default:
            AVG_ASSERT(false);
    }
}
    
const UTF8String BitmapManagerMsg::getFilename()
{
    return m_sFilename;
}

float BitmapManagerMsg::getStartTime()
{
    AVG_ASSERT(m_MsgType == REQUEST);
    return m_StartTime;
}
    
PixelFormat BitmapManagerMsg::getPixelFormat()
{
    AVG_ASSERT(m_MsgType == REQUEST);
    return m_PF;
}

void BitmapManagerMsg::setBitmap(BitmapPtr pBmp)
{
    AVG_ASSERT(m_MsgType == REQUEST);
    m_pBmp = pBmp;
    m_MsgType = BITMAP;
}

void BitmapManagerMsg::setError(const Exception& ex)
{
    AVG_ASSERT(m_MsgType == REQUEST);
    m_MsgType = ERROR;
    m_pEx = new Exception(ex);
}

}
